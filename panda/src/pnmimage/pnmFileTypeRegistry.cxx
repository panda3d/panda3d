/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeRegistry.cxx
 * @author drose
 * @date 2000-06-15
 */

#include "pnmFileTypeRegistry.h"
#include "pnmFileType.h"
#include "config_pnmimage.h"

#include "string_utils.h"
#include "indent.h"
#include "pset.h"

#include <algorithm>

using std::string;

PNMFileTypeRegistry *PNMFileTypeRegistry::_global_ptr;

/**
 *
 */
PNMFileTypeRegistry::
PNMFileTypeRegistry() {
  _requires_sort = false;
}

/**
 *
 */
PNMFileTypeRegistry::
~PNMFileTypeRegistry() {
}

/**
 * Defines a new PNMFileType in the universe.
 */
void PNMFileTypeRegistry::
register_type(PNMFileType *type) {
  if (pnmimage_cat->is_debug()) {
    pnmimage_cat->debug()
      << "Registering image type " << type->get_name() << "\n";
  }

  // Make sure we haven't already registered this type.
  TypeHandle handle = type->get_type();
  if (handle != PNMFileType::get_class_type()) {
    Handles::iterator hi = _handles.find(handle);
    if (hi != _handles.end()) {
      pnmimage_cat->warning()
        << "Attempt to register PNMFileType " << type->get_name()
       << " (" << type->get_type() << ") more than once.\n";
      return;
    }
    _handles.insert(Handles::value_type(handle, type));
  }

  _types.push_back(type);

  // Collect the unique extensions associated with the type.
  pset<string> unique_extensions;
  int num_extensions = type->get_num_extensions();
  for (int i = 0; i < num_extensions; i++) {
    string extension = downcase(type->get_extension(i));

    if (!unique_extensions.insert(extension).second) {
      pnmimage_cat->warning()
        << "PNMFileType " << type->get_name()
        << " (" << type->get_type() << ") defined extension "
        << extension << " more than once.\n";
    }
  }

  pset<string>::iterator ui;
  for (ui = unique_extensions.begin(); ui != unique_extensions.end(); ++ui) {
    _extensions[*ui].push_back(type);
  }

  _requires_sort = true;
}

/**
 * Removes a PNMFileType previously passed to register_type.
 */
void PNMFileTypeRegistry::
unregister_type(PNMFileType *type) {
  if (pnmimage_cat->is_debug()) {
    pnmimage_cat->debug()
      << "Unregistering image type " << type->get_name() << "\n";
  }

  TypeHandle handle = type->get_type();
  if (handle != PNMFileType::get_class_type()) {
    Handles::iterator hi = _handles.find(handle);
    if (hi != _handles.end()) {
      _handles.erase(hi);
    }
  }

  _types.erase(std::remove(_types.begin(), _types.end(), type),
               _types.end());

  Extensions::iterator ei;
  for (ei = _extensions.begin(); ei != _extensions.end(); ++ei) {
    Types &types = ei->second;
    types.erase(std::remove(types.begin(), types.end(), type),
                types.end());
  }

  _requires_sort = true;
}

/**
 * Returns the total number of types registered.
 */
int PNMFileTypeRegistry::
get_num_types() const {
  if (_requires_sort) {
    ((PNMFileTypeRegistry *)this)->sort_preferences();
  }
  return _types.size();
}

/**
 * Returns the nth type registered.
 */
PNMFileType *PNMFileTypeRegistry::
get_type(int n) const {
  nassertr(n >= 0 && n < (int)_types.size(), nullptr);
  return _types[n];
}

/**
 * Tries to determine what the PNMFileType is likely to be for a particular
 * image file based on its extension.  Returns a suitable PNMFileType pointer,
 * or NULL if no type can be determined.
 */
PNMFileType *PNMFileTypeRegistry::
get_type_from_extension(const string &filename) const {
  if (_requires_sort) {
    ((PNMFileTypeRegistry *)this)->sort_preferences();
  }

  // Extract the extension from the filename; if there is no dot, use the
  // whole filename as the extension.  This allows us to pass in just a
  // dotless extension name in lieu of a filename.

  string extension;
  size_t dot = filename.rfind('.');

  if (dot == string::npos) {
    extension = filename;
  } else {
    extension = filename.substr(dot + 1);
  }

#ifdef HAVE_ZLIB
  if (extension == "pz" || extension == "gz") {
    // If the extension is .pz, then we've got a Panda-compressed image file.
    // Back up some more and get the extension before that.
    size_t prev_dot = filename.rfind('.', dot - 1);
    if (prev_dot == string::npos) {
      extension = filename.substr(0, dot);
    } else {
      extension = filename.substr(prev_dot + 1, dot - prev_dot - 1);
    }
  }
#endif  // HAVE_ZLIB

  if (extension.find('/') != string::npos) {
    // If we picked the whole filename and it contains slashes, or if the
    // rightmost dot wasn't in the basename of the filename, then it's
    // actually a filename without an extension.
    extension = "";
  }

  Extensions::const_iterator ei;
  ei = _extensions.find(extension);
  if (ei == _extensions.end() || (*ei).second.empty()) {
    // Nothing matches that string.  Try again with a downcased string in case
    // we got an all-uppercase filename (most of our extensions are
    // downcased).
    ei = _extensions.find(downcase(extension));

    if (ei == _extensions.end() || (*ei).second.empty()) {
      // Nothing matches that string.
      return nullptr;
    }
  }

  // Return the first file type associated with the given extension.
  return (*ei).second.front();
}

/**
 * Tries to determine what the PNMFileType is likely to be for a particular
 * image file based on its magic number, the first two bytes read from the
 * file.  Returns a suitable PNMFileType pointer, or NULL if no type can be
 * determined.
 */
PNMFileType *PNMFileTypeRegistry::
get_type_from_magic_number(const string &magic_number) const {
  if (_requires_sort) {
    ((PNMFileTypeRegistry *)this)->sort_preferences();
  }

  Types::const_iterator ti;
  for (ti = _types.begin(); ti != _types.end(); ++ti) {
    PNMFileType *type = (*ti);
    if (type->has_magic_number() &&
        type->matches_magic_number(magic_number)) {
      return type;
    }
  }

  return nullptr;
}

/**
 * Returns the PNMFileType instance stored in the registry for the given
 * TypeHandle, e.g.  as retrieved by a previous call to get_type() on the type
 * instance.
 */
PNMFileType *PNMFileTypeRegistry::
get_type_by_handle(TypeHandle handle) const {
  Handles::const_iterator hi;
  hi = _handles.find(handle);
  if (hi != _handles.end()) {
    return (*hi).second;
  }

  return nullptr;
}

/**
 * Writes a list of supported image file types to the indicated output stream,
 * one per line.
 */
void PNMFileTypeRegistry::
write(std::ostream &out, int indent_level) const {
  if (_types.empty()) {
    indent(out, indent_level) << "(No image types are known).\n";
  } else {
    Types::const_iterator ti;
    for (ti = _types.begin(); ti != _types.end(); ++ti) {
      PNMFileType *type = (*ti);
      string name = type->get_name();
      indent(out, indent_level) << name;
      indent(out, std::max(30 - (int)name.length(), 0)) << "  ";

      int num_extensions = type->get_num_extensions();
      if (num_extensions == 1) {
        out << "." << type->get_extension(0);
      } else if (num_extensions > 1) {
        out << "." << type->get_extension(0);
        for (int i = 1; i < num_extensions; i++) {
          out << ", ." << type->get_extension(i);
        }
      }
      out << "\n";
    }
  }
}

/**
 * Returns a pointer to the global PNMFileTypeRegistry object.
 */
PNMFileTypeRegistry *PNMFileTypeRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new PNMFileTypeRegistry;
  }
  return _global_ptr;
}

/**
 * Sorts the PNMFileType pointers in order according to user-specified
 * preferences in the config file.  This allows us to choose a particular
 * PNMFileType over another for particular extensions when multiple file types
 * map to the same extension, or for file types that have no magic number.
 */
void PNMFileTypeRegistry::
sort_preferences() {
  // So, we don't do anything here yet.  One day we will.

  _requires_sort = false;
}
