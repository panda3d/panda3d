// Filename: loaderFileTypeRegistry.cxx
// Created by:  drose (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "loaderFileTypeRegistry.h"
#include "loaderFileType.h"
#include "config_loader.h"

#include <string_utils.h>
#include <indent.h>

#include <algorithm>

LoaderFileTypeRegistry *LoaderFileTypeRegistry::_global_ptr;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
LoaderFileTypeRegistry::
LoaderFileTypeRegistry() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
LoaderFileTypeRegistry::
~LoaderFileTypeRegistry() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::get_ptr
//       Access: Public, Static
//  Description: Returns a pointer to the global LoaderFileTypeRegistry
//               object.
////////////////////////////////////////////////////////////////////
LoaderFileTypeRegistry *LoaderFileTypeRegistry::
get_ptr() {
  if (_global_ptr == (LoaderFileTypeRegistry *)NULL) {
    _global_ptr = new LoaderFileTypeRegistry;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::get_num_types
//       Access: Public
//  Description: Returns the total number of types registered.
////////////////////////////////////////////////////////////////////
int LoaderFileTypeRegistry::
get_num_types() const {
  return _types.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::get_type
//       Access: Public
//  Description: Returns the nth type registered.
////////////////////////////////////////////////////////////////////
LoaderFileType *LoaderFileTypeRegistry::
get_type(int n) const {
  nassertr(n >= 0 && n < (int)_types.size(), NULL);
  return _types[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::get_type_from_extension
//       Access: Public
//  Description: Determines the type of the file based on the indicated
//               extension (without a leading dot).  Returns NULL if
//               the extension matches no known file types.
////////////////////////////////////////////////////////////////////
LoaderFileType *LoaderFileTypeRegistry::
get_type_from_extension(const string &extension) const {
  Extensions::const_iterator ei;
  ei = _extensions.find(downcase(extension));
  if (ei == _extensions.end()) {
    // Nothing matches that extension.
    return NULL;
  }

  return (*ei).second;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::write_types
//       Access: Public
//  Description: Writes a list of supported file types to the
//               indicated output stream, one per line.
////////////////////////////////////////////////////////////////////
void LoaderFileTypeRegistry::
write_types(ostream &out, int indent_level) const {
  if (_types.empty()) {
    indent(out, indent_level) << "(No file types are known).\n";
  } else {
    Types::const_iterator ti;
    for (ti = _types.begin(); ti != _types.end(); ++ti) {
      LoaderFileType *type = (*ti);
      string name = type->get_name();
      indent(out, indent_level) << name;
      indent(out, max(30 - (int)name.length(), 0)) 
        << "  ." << type->get_extension() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeRegistry::register_type
//       Access: Public
//  Description: Defines a new LoaderFileType in the universe.
////////////////////////////////////////////////////////////////////
void LoaderFileTypeRegistry::
register_type(LoaderFileType *type) {
  // Make sure we haven't already registered this type.
  if (find(_types.begin(), _types.end(), type) != _types.end()) {
    loader_cat.warning()
      << "Attempt to register LoaderFileType " << type->get_name() 
      << " (" << type->get_type() << ") more than once.\n";
    return;
  }

  _types.push_back(type);

  string extension = downcase(type->get_extension());
  Extensions::const_iterator ei;
  ei = _extensions.find(extension);
  if (ei != _extensions.end()) {
    loader_cat.warning() 
      << "Multiple LoaderFileTypes registered that use the extension " 
      << extension << "\n";
  } else {
    _extensions.insert(Extensions::value_type(extension, type));
  }
}
