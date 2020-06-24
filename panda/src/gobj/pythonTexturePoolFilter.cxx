/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonTexturePoolFilter.cxx
 * @author Derzsi Daniel
 * @date 2020-06-14
 */

#include "pythonTexturePoolFilter.h"

#ifdef HAVE_PYTHON

#include "pythonThread.h"
#include "py_panda.h"

extern struct Dtool_PyTypedObject Dtool_Filename;
extern struct Dtool_PyTypedObject Dtool_LoaderOptions;
extern struct Dtool_PyTypedObject Dtool_Texture;

TypeHandle PythonTexturePoolFilter::_type_handle;

/**
 * Constructor.
 */
PythonTexturePoolFilter::
PythonTexturePoolFilter() {
  init_type();
}

/**
 * Destructor.
 */
PythonTexturePoolFilter::
~PythonTexturePoolFilter() {
  if (_pre_load_func != nullptr || _post_load_func != nullptr) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    Py_CLEAR(_pre_load_func);
    Py_CLEAR(_post_load_func);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }
}

/**
 * Initializes the filter to use the given Python filter object.
 *
 * This method expects a Python object that implements either
 * the pre_load or the post_load method.
 */
bool PythonTexturePoolFilter::
init(PyObject *tex_filter) {
  nassertr(tex_filter != nullptr, false);
  nassertr(_pre_load_func == nullptr, false);
  nassertr(_post_load_func == nullptr, false);

  _pre_load_func = PyObject_GetAttrString(tex_filter, "pre_load");
  _post_load_func = PyObject_GetAttrString(tex_filter, "post_load");
  _filter_hash = PyObject_Hash(tex_filter);
  PyErr_Clear();

  if (_pre_load_func == nullptr && _post_load_func == nullptr) {
    PyErr_Format(PyExc_TypeError,
                 "texture pool filter plug-in %R does not define pre_load or post_load function",
                 tex_filter);
    return false;
  }

  return true;
}

/**
 * This method is called before each texture is loaded from disk, via the
 * TexturePool, for the first time. We delegate this functionality to
 * our Python module, loaded through the init function.
 */
PT(Texture) PythonTexturePoolFilter::
pre_load(const Filename &orig_filename, const Filename &orig_alpha_filename,
          int primary_file_num_channels, int alpha_file_channel,
          bool read_mipmaps, const LoaderOptions &options) {
  if (_pre_load_func == nullptr) {
    return nullptr;
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  // Wrap the arguments.
  PyObject *args = PyTuple_Pack(6,
    DTool_CreatePyInstance((void *)&orig_filename, Dtool_Filename, false, true),
    DTool_CreatePyInstance((void *)&orig_alpha_filename, Dtool_Filename, false, true),
    Dtool_WrapValue(primary_file_num_channels),
    Dtool_WrapValue(alpha_file_channel),
    Dtool_WrapValue(read_mipmaps),
    DTool_CreatePyInstance((void *)&options, Dtool_LoaderOptions, false, true)
  );

  PT(Texture) tex = nullptr;
  PyObject *result = PythonThread::call_python_func(_pre_load_func, args);

  Py_DECREF(args);

  if (result != nullptr) {
    if (DtoolInstance_Check(result)) {
      tex = (Texture *)DtoolInstance_UPCAST(result, Dtool_Texture);
    }
    Py_DECREF(result);
  }

  PyObject *exc_type = _PyErr_OCCURRED();

  if (exc_type) {
    gobj_cat.error()
      << "Preloading " << orig_filename.get_basename() << " failed with "
      << ((PyTypeObject *)exc_type)->tp_name << " exception.\n";
    PyErr_Clear();
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  return tex;
}

/**
 * This method is called after each texture has been loaded from disk, via the
 * TexturePool, for the first time. We delegate this functionality to
 * our Python module, loaded through the init function.
 */
PT(Texture) PythonTexturePoolFilter::
post_load(Texture *tex) {
  if (_post_load_func == nullptr) {
    return tex;
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  // Wrap the arguments.
  PyObject *args = PyTuple_Pack(1, DTool_CreatePyInstance(tex, Dtool_Texture, true, false));
  PyObject *result = PythonThread::call_python_func(_post_load_func, args);
  Texture *tex_res = nullptr;

  Py_DECREF(args);

  if (result != nullptr) {
    if (DtoolInstance_Check(result)) {
      tex_res = (Texture *)DtoolInstance_UPCAST(result, Dtool_Texture);
    }
    Py_DECREF(result);
  }

  PyObject *exc_type = _PyErr_OCCURRED();

  if (exc_type) {
    gobj_cat.error()
      << "Postloading failed with " << ((PyTypeObject *)exc_type)->tp_name
      << " exception.\n";
    PyErr_Clear();
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  if (tex_res != nullptr) {
    return tex_res;
  } else {
    // The Python function did not return anything.
    // We'll return our original texture instead!
    return tex;
  }
}

#endif  // HAVE_PYTHON
