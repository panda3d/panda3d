/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonLoaderFileType.cxx
 * @author rdb
 * @date 2019-07-29
 */

#include "pythonLoaderFileType.h"

#ifdef HAVE_PYTHON

#include "bamCacheRecord.h"
#include "modelRoot.h"
#include "pythonThread.h"
#include "py_panda.h"
#include "virtualFileSystem.h"

extern struct Dtool_PyTypedObject Dtool_BamCacheRecord;
extern struct Dtool_PyTypedObject Dtool_Filename;
extern struct Dtool_PyTypedObject Dtool_LoaderOptions;
extern struct Dtool_PyTypedObject Dtool_PandaNode;
extern struct Dtool_PyTypedObject Dtool_PythonLoaderFileType;

TypeHandle PythonLoaderFileType::_type_handle;

/**
 * This constructor expects init() to be called manually.
 */
PythonLoaderFileType::
PythonLoaderFileType() {
  init_type();
}

/**
 * This constructor expects a single pkg_resources.EntryPoint argument for a
 * deferred loader.
 */
PythonLoaderFileType::
PythonLoaderFileType(std::string extension, PyObject *entry_point) :
  _extension(std::move(extension)),
  _entry_point(entry_point) {

  init_type();
  Py_INCREF(entry_point);
}

/**
 * Destructor.
 */
PythonLoaderFileType::
~PythonLoaderFileType() {
  if (_entry_point != nullptr || _load_func != nullptr || _save_func != nullptr) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    Py_CLEAR(_entry_point);
    Py_CLEAR(_load_func);
    Py_CLEAR(_save_func);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }
}

/**
 * Initializes the fields from the given Python loader object.
 */
bool PythonLoaderFileType::
init(PyObject *loader) {
  nassertr(loader != nullptr, false);
  nassertr(_load_func == nullptr, false);
  nassertr(_save_func == nullptr, false);

  // Check the extensions member.  If we already have a registered extension,
  // it must occur in the list.
  PyObject *extensions = PyObject_GetAttrString(loader, "extensions");
  if (extensions != nullptr) {
    if (PyUnicode_Check(extensions)
#if PY_MAJOR_VERSION < 3
      || PyString_Check(extensions)
#endif
      ) {
      Dtool_Raise_TypeError("extensions list should be a list or tuple");
      Py_DECREF(extensions);
      return false;
    }

    PyObject *sequence = PySequence_Fast(extensions, "extensions must be a sequence");
    PyObject **items = PySequence_Fast_ITEMS(sequence);
    Py_ssize_t num_items = PySequence_Fast_GET_SIZE(sequence);
    Py_DECREF(extensions);

    if (num_items == 0) {
      PyErr_SetString(PyExc_ValueError, "extensions list may not be empty");
      Py_DECREF(sequence);
      return false;
    }

    bool found_extension = false;

    for (Py_ssize_t i = 0; i < num_items; ++i) {
      PyObject *extension = items[i];
      const char *extension_str;
      Py_ssize_t extension_len;
  #if PY_MAJOR_VERSION >= 3
      extension_str = PyUnicode_AsUTF8AndSize(extension, &extension_len);
  #else
      if (PyString_AsStringAndSize(extension, (char **)&extension_str, &extension_len) == -1) {
        extension_str = nullptr;
      }
  #endif

      if (extension_str == nullptr) {
        Py_DECREF(sequence);
        return false;
      }

      if (_extension.empty()) {
        _extension.assign(extension_str, extension_len);
        found_extension = true;
      } else {
        std::string new_extension(extension_str, extension_len);
        if (_extension == new_extension) {
          found_extension = true;
        } else {
          if (!_additional_extensions.empty()) {
            _additional_extensions += ' ';
          }
          _additional_extensions += new_extension;
        }
      }
    }
    Py_DECREF(sequence);

    if (!found_extension) {
      PyObject *repr = PyObject_Repr(loader);
      loader_cat.error()
        << "Registered extension '" << _extension
        << "' does not occur in extensions list of "
#if PY_MAJOR_VERSION >= 3
        << PyUnicode_AsUTF8(repr) << "\n";
#else
        << PyString_AsString(repr) << "\n";
#endif
      Py_DECREF(repr);
      return false;
    }
  } else {
    return false;
  }

  PyObject *supports_compressed = PyObject_GetAttrString(loader, "supports_compressed");
  if (supports_compressed != nullptr) {
    if (supports_compressed == Py_True) {
      _supports_compressed = true;
    }
    else if (supports_compressed == Py_False) {
      _supports_compressed = false;
    }
    else {
      Dtool_Raise_TypeError("supports_compressed must be a bool");
      Py_DECREF(supports_compressed);
      return false;
    }
    Py_DECREF(supports_compressed);
  }

  _load_func = PyObject_GetAttrString(loader, "load_file");
  _save_func = PyObject_GetAttrString(loader, "save_file");
  PyErr_Clear();

  if (_load_func == nullptr && _save_func == nullptr) {
#if PY_MAJOR_VERSION >= 3
    PyErr_Format(PyExc_TypeError,
                 "loader plug-in %R does not define load_file or save_file function",
                 loader);
#else
    PyObject *repr = PyObject_Repr(loader);
    PyErr_Format(PyExc_TypeError,
                 "loader plug-in %s does not define load_file or save_file function",
                 PyString_AsString(repr));
    Py_DECREF(repr);
#endif
    return false;
  }

  // We don't need this any more.
  Py_CLEAR(_entry_point);

  return true;
}

/**
 * Ensures that the referenced Python module is loaded.
 */
bool PythonLoaderFileType::
ensure_loaded() const {
  if (_load_func != nullptr || _save_func != nullptr) {
    return true;
  }
  nassertr_always(_entry_point != nullptr, false);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  if (loader_cat.is_info()) {
    PyObject *repr = PyObject_Repr(_entry_point);

    loader_cat.info()
      << "loading file type module: "
#if PY_MAJOR_VERSION >= 3
      << PyUnicode_AsUTF8(repr) << "\n";
#else
      << PyString_AsString(repr) << "\n";
#endif
    Py_DECREF(repr);
  }

  PyObject *result = PyObject_CallMethod(_entry_point, (char *)"load", nullptr);

  bool success = false;
  if (result != nullptr) {
    success = ((PythonLoaderFileType *)this)->init(result);
  } else {
    PyErr_Clear();
    PyObject *repr = PyObject_Repr(_entry_point);

    loader_cat.error()
      << "unable to load "
#if PY_MAJOR_VERSION >= 3
      << PyUnicode_AsUTF8(repr) << "\n";
#else
      << PyString_AsString(repr) << "\n";
#endif
    Py_DECREF(repr);
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  return success;
}

/**
 *
 */
std::string PythonLoaderFileType::
get_name() const {
  return "Python loader";
}

/**
 *
 */
std::string PythonLoaderFileType::
get_extension() const {
  return _extension;
}

/**
 * Returns a space-separated list of extension, in addition to the one
 * returned by get_extension(), that are recognized by this converter.
 */
std::string PythonLoaderFileType::
get_additional_extensions() const {
  return _additional_extensions;
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool PythonLoaderFileType::
supports_compressed() const {
  return ensure_loaded() && _supports_compressed;
}

/**
 * Returns true if the file type can be used to load files, and load_file() is
 * supported.  Returns false if load_file() is unimplemented and will always
 * fail.
 */
bool PythonLoaderFileType::
supports_load() const {
  return ensure_loaded() && _load_func != nullptr;
}

/**
 * Returns true if the file type can be used to save files, and save_file() is
 * supported.  Returns false if save_file() is unimplemented and will always
 * fail.
 */
bool PythonLoaderFileType::
supports_save() const {
  return ensure_loaded() && _save_func != nullptr;
}

/**
 *
 */
PT(PandaNode) PythonLoaderFileType::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {
  // Let's check whether the file even exists before calling Python.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vfile = vfs->get_file(path);
  if (vfile == nullptr) {
    return nullptr;
  }

  if (!supports_load()) {
    return nullptr;
  }

  if (record != nullptr) {
    record->add_dependent_file(vfile);
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  // Wrap the arguments.
  PyObject *args = PyTuple_New(3);
  PyTuple_SET_ITEM(args, 0, DTool_CreatePyInstance((void *)&path, Dtool_Filename, false, true));
  PyTuple_SET_ITEM(args, 1, DTool_CreatePyInstance((void *)&options, Dtool_LoaderOptions, false, true));
  if (record != nullptr) {
    record->ref();
    PyTuple_SET_ITEM(args, 2, DTool_CreatePyInstanceTyped((void *)record, Dtool_BamCacheRecord, true, false, record->get_type_index()));
  } else {
    PyTuple_SET_ITEM(args, 2, Py_None);
    Py_INCREF(Py_None);
  }

  PT(PandaNode) node;

  PyObject *result = PythonThread::call_python_func(_load_func, args);
  if (result != nullptr) {
    if (DtoolInstance_Check(result)) {
      node = (PandaNode *)DtoolInstance_UPCAST(result, Dtool_PandaNode);
    }
    Py_DECREF(result);
  }

  Py_DECREF(args);

  if (node == nullptr) {
    PyObject *exc_type = _PyErr_OCCURRED();
    if (!exc_type) {
      loader_cat.error()
        << "load_file must return valid PandaNode or raise exception\n";
    } else {
      loader_cat.error()
        << "Loading " << path.get_basename()
        << " failed with " << ((PyTypeObject *)exc_type)->tp_name << " exception.\n";
      PyErr_Clear();
    }
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  if (node != nullptr && node->is_of_type(ModelRoot::get_class_type())) {
    ModelRoot *model_root = DCAST(ModelRoot, node.p());
    model_root->set_fullpath(path);
    model_root->set_timestamp(vfile->get_timestamp());
  }

  return node;
}

/**
 *
 */
bool PythonLoaderFileType::
save_file(const Filename &path, const LoaderOptions &options,
          PandaNode *node) const {
  if (!supports_save()) {
    return false;
  }

  nassertr(node != nullptr, false);
  node->ref();

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  // Wrap the arguments.
  PyObject *args = PyTuple_New(3);
  PyTuple_SET_ITEM(args, 0, DTool_CreatePyInstance((void *)&path, Dtool_Filename, false, true));
  PyTuple_SET_ITEM(args, 1, DTool_CreatePyInstance((void *)&options, Dtool_LoaderOptions, false, true));
  PyTuple_SET_ITEM(args, 2, DTool_CreatePyInstanceTyped((void *)node, Dtool_PandaNode, true, false, node->get_type_index()));

  PyObject *result = PythonThread::call_python_func(_load_func, args);
  Py_DECREF(args);
  if (result != nullptr) {
    Py_DECREF(result);
  } else {
    PyErr_Clear();
    loader_cat.error()
      << "save_file failed with an exception.\n";
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  return (result != nullptr);
}

#endif  // HAVE_PYTHON
