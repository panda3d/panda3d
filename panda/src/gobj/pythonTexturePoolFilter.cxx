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

    Py_CLEAR(_entry_point);
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
  nassertr(_entry_point == nullptr, false);
  nassertr(_pre_load_func == nullptr, false);
  nassertr(_post_load_func == nullptr, false);

  _pre_load_func = PyObject_GetAttrString(tex_filter, "pre_load");
  PyErr_Clear();
  _post_load_func = PyObject_GetAttrString(tex_filter, "post_load");
  PyErr_Clear();

  if (_pre_load_func == nullptr && _post_load_func == nullptr) {
    PyErr_Format(PyExc_TypeError,
                 "texture pool filter plug-in %R does not define pre_load or post_load function",
                 tex_filter);
    return false;
  }

  _entry_point = Py_NewRef(tex_filter);
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
  PyObject *args = Py_BuildValue("(OOiiNO)",
    DTool_CreatePyInstance((void *)&orig_filename, Dtool_Filename, false, true),
    DTool_CreatePyInstance((void *)&orig_alpha_filename, Dtool_Filename, false, true),
    primary_file_num_channels,
    alpha_file_channel,
    PyBool_FromLong(read_mipmaps),
    DTool_CreatePyInstance((void *)&options, Dtool_LoaderOptions, false, true)
  );

  PyObject *result = PythonThread::call_python_func(_pre_load_func, args);
  Py_DECREF(args);

  PT(Texture) tex;
  if (result != nullptr) {
    if (result != Py_None) {
      if (DtoolInstance_Check(result)) {
        tex = (Texture *)DtoolInstance_UPCAST(result, Dtool_Texture);
      }

      if (tex == nullptr) {
        gobj_cat.error()
          << "Preloading " << orig_filename.get_basename() << " failed as "
          << "preloaded texture is not of Texture type.\n";
      }
    }

    Py_DECREF(result);
  } else {
    PyObject *exc_type = PyErr_Occurred();
    nassertr(exc_type != nullptr, nullptr);

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
  PyObject *args = PyTuple_Pack(1,
    DTool_CreatePyInstance(tex, Dtool_Texture, true, false)
  );
  PyObject *result = PythonThread::call_python_func(_post_load_func, args);
  Py_DECREF(args);

  PT(Texture) result_tex;
  if (result != nullptr) {
    if (result != Py_None) {
      // Check if the call returned a texture pointer.
      if (DtoolInstance_Check(result)) {
        result_tex = (Texture *)DtoolInstance_UPCAST(result, Dtool_Texture);
      }

      if (result_tex == nullptr) {
        // No valid texture was returned, use the original pointer.
        gobj_cat.error()
          << "Postloading failed as the returned texture is not of the Texture type.\n";
        result_tex = tex;
      }
    }
  } else {
    PyObject *exc_type = PyErr_Occurred();
    nassertr(exc_type != nullptr, result_tex);

    gobj_cat.error()
      << "Postloading texture failed with "
      << ((PyTypeObject *)exc_type)->tp_name << " exception.\n";
    PyErr_Clear();

    result_tex = tex;
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  return result_tex;
}

#endif  // HAVE_PYTHON
