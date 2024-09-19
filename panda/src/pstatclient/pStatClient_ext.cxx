/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClient_ext.cxx
 * @author rdb
 * @date 2022-11-23
 */

#include "pStatClient_ext.h"

#if defined(HAVE_PYTHON) && defined(DO_PSTATS)

#include "pStatCollector.h"
#include "config_pstatclient.h"

#ifndef CPPPARSER
#include "frameobject.h"
#endif

static bool _python_profiler_enabled = false;

// Used to cache stuff onto PyCodeObjects.
static Py_ssize_t _extra_index = -1;

// Stores a mapping between C method definitions and collector indices.
static pmap<PyMethodDef *, int> _c_method_collectors;

// Parent collector for all Python profiling collectors.
static PStatCollector code_collector("App:Python");

/**
 * Walks up the type hierarchy to find the class where the method originates.
 */
static bool
find_method(PyTypeObject *&cls, PyObject *name, PyCodeObject *code) {
  PyObject *meth = _PyType_Lookup(cls, name);
  if (meth == nullptr || !PyFunction_Check(meth) ||
      PyFunction_GET_CODE(meth) != (PyObject *)code) {
    return false;
  }

  if (cls->tp_bases != nullptr) {
    Py_ssize_t size = PyTuple_GET_SIZE(cls->tp_bases);
    for (Py_ssize_t i = 0; i < size; ++i) {
      PyTypeObject *base = (PyTypeObject *)PyTuple_GET_ITEM(cls->tp_bases, i);

      if (find_method(base, name, code)) {
        cls = base;
        return true;
      }
    }
  }

  // Didn't find it in any of the bases, it must be defined here.
  return true;
}

/**
 * Returns the collector for a Python frame.
 */
static int
#ifdef __GNUC__
__attribute__ ((noinline))
#elif defined(_MSC_VER)
__declspec(noinline)
#endif
make_python_frame_collector(PyFrameObject *frame, PyCodeObject *code) {
#if PY_VERSION_HEX >= 0x030B0000 // 3.11
  // Fetch the module name out of the frame's global scope.
  const char *mod_name = "<unknown>";
  PyObject *py_mod_name = nullptr;
  PyObject *globals = PyFrame_GetGlobals(frame);
#if PY_VERSION_HEX >= 0x030D00A1 // 3.13
  if (PyDict_GetItemStringRef(globals, "__name__", &py_mod_name) > 0) {
    mod_name = PyUnicode_AsUTF8(py_mod_name);
  }
#else
  py_mod_name = PyDict_GetItemString(globals, "__name__");
  if (py_mod_name != nullptr) {
    mod_name = PyUnicode_AsUTF8(py_mod_name);
  }
#endif
  Py_DECREF(globals);

  const char *meth_name = PyUnicode_AsUTF8(code->co_qualname);
  char buffer[1024];
  size_t len = snprintf(buffer, sizeof(buffer), "%s:%s", mod_name, meth_name);
  for (size_t i = 0; i < len - 1; ++i) {
    if (buffer[i] == '.') {
      buffer[i] = ':';
    }
  }

#if PY_VERSION_HEX >= 0x030D00A1 // 3.13
  Py_XDECREF(py_mod_name);
#endif

#else
  // Try to figure out the type name.  There's no obvious way to do this.
  // It's possible that the first argument passed to this function is the
  // self instance or the current type (for a classmethod), but we have to
  // double-check that to make sure.
  PyTypeObject *cls = nullptr;
  if (code->co_argcount >= 1) {
    PyFrame_FastToLocals(frame);
    PyObject *first_arg = PyDict_GetItem(frame->f_locals, PyTuple_GET_ITEM(code->co_varnames, 0));
    cls = PyType_Check(first_arg) ? (PyTypeObject *)first_arg : Py_TYPE(first_arg);
    if ((cls->tp_flags & Py_TPFLAGS_HEAPTYPE) != 0) {
      // Mangling scheme for methods starting (but not ending) with "__"
      PyObject *meth_name = code->co_name;
      Py_ssize_t len = PyUnicode_GET_LENGTH(meth_name);
      if (len >= 2 && PyUnicode_READ_CHAR(meth_name, 0) == '_' && PyUnicode_READ_CHAR(meth_name, 1) == '_' &&
          (len < 4 || PyUnicode_READ_CHAR(meth_name, len - 1) != '_' || PyUnicode_READ_CHAR(meth_name, len - 2) != '_')) {
        const char *cls_name = cls->tp_name;
        while (cls_name[0] == '_') {
          ++cls_name;
        }
        meth_name = PyUnicode_FromFormat("_%s%S", cls_name, meth_name);
      } else {
        meth_name = Py_NewRef(meth_name);
      }
      if (!find_method(cls, meth_name, code)) {
        // Not a matching method object, it's something else.  Forget it.
        cls = nullptr;
      }
      Py_DECREF(meth_name);
    } else {
      cls = nullptr;
    }
  }

  // Fetch the module name out of the frame's global scope.
  PyObject *py_mod_name = PyDict_GetItemString(frame->f_globals, "__name__");
  if (py_mod_name == nullptr && cls != nullptr) {
    py_mod_name = PyDict_GetItemString(cls->tp_dict, "__module__");
  }

  const char *mod_name = py_mod_name ? PyUnicode_AsUTF8(py_mod_name) : "<unknown>";
  char buffer[1024];
  size_t len = snprintf(buffer, sizeof(buffer), "%s:", mod_name);
  for (size_t i = 0; i < len - 1; ++i) {
    if (buffer[i] == '.') {
      buffer[i] = ':';
    }
  }

  const char *meth_name = PyUnicode_AsUTF8(code->co_name);
  if (cls != nullptr) {
    len += snprintf(buffer + len, sizeof(buffer) - len, "%s:%s", cls->tp_name, meth_name);
  } else {
    len += snprintf(buffer + len, sizeof(buffer) - len, "%s", meth_name);
  }
#endif

  // Add parentheses, unless it's something special like <listcomp>
  if (len < sizeof(buffer) - 2 && buffer[len - 1] != '>') {
    buffer[len++] = '(';
    buffer[len++] = ')';
    buffer[len] = '\0';
  }

  PStatCollector collector(code_collector, buffer);
  intptr_t collector_index = collector.get_index();
  if (_extra_index != -1) {
    _PyCode_SetExtra((PyObject *)code, _extra_index, (void *)collector_index);
  }
  return collector_index;
}

/**
 * Creates a collector for a C function.
 */
static int
#ifdef __GNUC__
__attribute__ ((noinline))
#elif defined(_MSC_VER)
__declspec(noinline)
#endif
make_c_function_collector(PyCFunctionObject *meth) {
  char buffer[1024];
  size_t len;
  if (meth->m_self != nullptr && !PyModule_Check(meth->m_self)) {
    PyTypeObject *cls = PyType_Check(meth->m_self) ? (PyTypeObject *)meth->m_self : Py_TYPE(meth->m_self);

    const char *dot = strrchr(cls->tp_name, '.');
    if (dot != nullptr) {
      // The module name is included in the type name.
      snprintf(buffer, sizeof(buffer), "%s:%s()", cls->tp_name, meth->m_ml->ml_name);
      len = (dot - cls->tp_name) + 1;
    } else {
      // If there's no module name, we need to get it from __module__.
      PyObject *py_mod_name = nullptr;
      const char *mod_name = nullptr;
      if (cls->tp_dict != nullptr &&
          PyDict_GetItemStringRef(cls->tp_dict, "__module__", &py_mod_name) > 0) {
        if (PyUnicode_Check(py_mod_name)) {
          mod_name = PyUnicode_AsUTF8(py_mod_name);
        } else {
          // Might be a descriptor.
          Py_DECREF(py_mod_name);
          py_mod_name = PyObject_GetAttrString(meth->m_self, "__module__");
          if (py_mod_name != nullptr) {
            if (PyUnicode_Check(py_mod_name)) {
              mod_name = PyUnicode_AsUTF8(py_mod_name);
            }
          }
          else PyErr_Clear();
        }
      }
      else PyErr_Clear();

      if (mod_name == nullptr) {
        // Is it a built-in, like int or dict?
        PyObject *builtins = PyEval_GetBuiltins();
        if (PyDict_GetItemString(builtins, cls->tp_name) == (PyObject *)cls) {
          mod_name = "builtins";
        } else {
          mod_name = "<unknown>";
        }
      }
      len = snprintf(buffer, sizeof(buffer), "%s:%s:%s()", mod_name, cls->tp_name, meth->m_ml->ml_name) - 2;
      Py_XDECREF(py_mod_name);
    }
  }
  else if (meth->m_self != nullptr) {
    const char *mod_name = PyModule_GetName(meth->m_self);
    len = snprintf(buffer, sizeof(buffer), "%s:%s()", mod_name, meth->m_ml->ml_name) - 2;
  }
  else {
    snprintf(buffer, sizeof(buffer), "%s()", meth->m_ml->ml_name);
    len = 0;
  }
  for (size_t i = 0; i < len; ++i) {
    if (buffer[i] == '.') {
      buffer[i] = ':';
    }
  }
  PStatCollector collector(code_collector, buffer);
  int collector_index = collector.get_index();
  _c_method_collectors[meth->m_ml] = collector.get_index();
  return collector_index;
}

/**
 * Attempts to establish a connection to the indicated PStatServer.  Returns
 * true if successful, false on failure.
 */
bool Extension<PStatClient>::
client_connect(std::string hostname, int port) {
  extern struct Dtool_PyTypedObject Dtool_PStatThread;

  if (_this->client_connect(std::move(hostname), port)) {
    // Pass a PStatThread as argument.
    if (!_python_profiler_enabled && pstats_python_profiler) {
      PStatThread *thread = new PStatThread(_this->get_current_thread());
      PyObject *arg = DTool_CreatePyInstance((void *)thread, Dtool_PStatThread, true, false);
      if (_extra_index == -1) {
        _extra_index = _PyEval_RequestCodeExtraIndex(nullptr);
      }
      PyEval_SetProfile(&trace_callback, arg);
      _python_profiler_enabled = false;
    }
    return true;
  }
  else if (_python_profiler_enabled) {
    PyEval_SetProfile(nullptr, nullptr);
    _python_profiler_enabled = false;
  }
  return false;
}

/**
 * Closes the connection previously established.
 */
void Extension<PStatClient>::
client_disconnect() {
  _this->client_disconnect();
  if (_python_profiler_enabled) {
    PyEval_SetProfile(nullptr, nullptr);
    _python_profiler_enabled = false;
  }
}

/**
 * Callback passed to PyEval_SetProfile.
 */
int Extension<PStatClient>::
trace_callback(PyObject *py_thread, PyFrameObject *frame, int what, PyObject *arg) {
  intptr_t collector_index;

  if (what == PyTrace_CALL || what == PyTrace_RETURN || what == PyTrace_EXCEPTION) {
    // Normal Python frame entry/exit.
#if PY_VERSION_HEX >= 0x030B0000 // 3.11
    PyCodeObject *code = PyFrame_GetCode(frame);
#else
    PyCodeObject *code = frame->f_code;
#endif

    // The index for this collector is cached on the code object.
    if (_PyCode_GetExtra((PyObject *)code, _extra_index, (void **)&collector_index) != 0 || collector_index == 0) {
      collector_index = make_python_frame_collector(frame, code);
    }

#if PY_VERSION_HEX >= 0x030B0000 // 3.11
    Py_DECREF(code);
#endif
  } else if (what == PyTrace_C_CALL || what == PyTrace_C_RETURN || what == PyTrace_C_EXCEPTION) {
    // Call to a C function or method, which has no frame of its own.
    if (PyCFunction_CheckExact(arg)) {
      PyCFunctionObject *meth = (PyCFunctionObject *)arg;
      auto it = _c_method_collectors.find(meth->m_ml);
      if (it != _c_method_collectors.end()) {
        collector_index = it->second;
      } else {
        collector_index = make_c_function_collector(meth);
      }
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  if (collector_index <= 0) {
    return 0;
  }

  PStatThread &pthread = *(PStatThread *)DtoolInstance_VOID_PTR(py_thread);
  PStatClient *client = pthread.get_client();
  if (!client->client_is_connected()) {
    // Client was disconnected, disable Python profiling.
    PyEval_SetProfile(nullptr, nullptr);
    _python_profiler_enabled = false;
    return 0;
  }

  int thread_index = pthread.get_index();

#ifdef _DEBUG
  nassertr(collector_index >= 0 && collector_index < client->get_num_collectors(), -1);
  nassertr(thread_index >= 0 && thread_index < client->get_num_threads(), -1);
#endif

  PStatClient::Collector *collector = client->get_collector_ptr(collector_index);
  PStatClient::InternalThread *thread = client->get_thread_ptr(thread_index);

  if (collector->is_active() && thread->_is_active) {
    double as_of = client->get_real_time();

    LightMutexHolder holder(thread->_thread_lock);
    if (thread->_thread_active) {
      if (what == PyTrace_CALL || what == PyTrace_C_CALL) {
        thread->_frame_data.add_start(collector_index, as_of);
      } else {
        thread->_frame_data.add_stop(collector_index, as_of);
      }
    }
  }

  return 0;
}

#endif  // HAVE_PYTHON && DO_PSTATS
