/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeRegistry.cxx
 * @author drose
 * @date 2000-06-20
 */

#include "loaderFileTypeRegistry.h"
#include "loaderFileType.h"
#include "config_pgraph.h"

#include "load_dso.h"
#include "string_utils.h"
#include "indent.h"

#include <algorithm>

using std::string;

LoaderFileTypeRegistry *LoaderFileTypeRegistry::_global_ptr;

/**
 *
 */
LoaderFileTypeRegistry::
LoaderFileTypeRegistry() {
}

/**
 *
 */
LoaderFileTypeRegistry::
~LoaderFileTypeRegistry() {
}

/**
 * Defines a new LoaderFileType in the universe.
 */
void LoaderFileTypeRegistry::
register_type(LoaderFileType *type) {
  // Make sure we haven't already registered this type.
  if (find(_types.begin(), _types.end(), type) != _types.end()) {
    if (loader_cat->is_debug()) {
      loader_cat->debug()
        << "Attempt to register LoaderFileType " << type->get_name()
        << " (" << type->get_type() << ") more than once.\n";
    }
    return;
  }

  _types.push_back(type);

  if (!type->get_extension().empty()) {
    record_extension(type->get_extension(), type);
  }

  vector_string words;
  extract_words(type->get_additional_extensions(), words);
  vector_string::const_iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    record_extension(*wi, type);
  }
}

/**
 * Records a type associated with a particular extension to be loaded in the
 * future.  The named library will be dynamically loaded the first time files
 * of this extension are loaded; presumably this library will call
 * register_type() when it initializes, thus making the extension loadable.
 */
void LoaderFileTypeRegistry::
register_deferred_type(const string &extension, const string &library) {
  string dcextension = downcase(extension);

  Extensions::const_iterator ei;
  ei = _extensions.find(dcextension);
  if (ei != _extensions.end()) {
    // We already have a loader for this type; no need to register another
    // one.
    if (loader_cat->is_debug()) {
      loader_cat->debug()
        << "Attempt to register loader library " << library
        << " (" << dcextension << ") when extension is already known.\n";
    }
    return;
  }

  DeferredTypes::const_iterator di;
  di = _deferred_types.find(dcextension);
  if (di != _deferred_types.end()) {
    if ((*di).second == library) {
      if (loader_cat->is_debug()) {
        loader_cat->debug()
          << "Attempt to register loader library " << library
          << " (" << dcextension << ") more than once.\n";
      }
      return;
    } else {
      if (loader_cat->is_debug()) {
        loader_cat->debug()
          << "Multiple libraries registered that use the extension "
          << dcextension << "\n";
      }
    }
  }

  _deferred_types[dcextension] = library;
}

/**
 * Removes a type previously registered using register_type.
 */
void LoaderFileTypeRegistry::
unregister_type(LoaderFileType *type) {
  Types::iterator it = find(_types.begin(), _types.end(), type);
  if (it == _types.end()) {
    if (loader_cat.is_debug()) {
      loader_cat.debug()
        << "Attempt to unregister LoaderFileType " << type->get_name()
        << " (" << type->get_type() << "), which was not registered.\n";
    }
    return;
  }

  _types.erase(it);

  {
    std::string dcextension = downcase(type->get_extension());
    Extensions::iterator ei = _extensions.find(dcextension);
    if (ei != _extensions.end() && ei->second == type) {
      _extensions.erase(ei);
    }
  }

  vector_string words;
  extract_words(type->get_additional_extensions(), words);
  for (const std::string &word : words) {
    Extensions::iterator ei = _extensions.find(downcase(word));
    if (ei != _extensions.end() && ei->second == type) {
      _extensions.erase(ei);
    }
  }
}

/**
 * Returns the total number of types registered.
 */
int LoaderFileTypeRegistry::
get_num_types() const {
  return _types.size();
}

/**
 * Returns the nth type registered.
 */
LoaderFileType *LoaderFileTypeRegistry::
get_type(int n) const {
  nassertr(n >= 0 && n < (int)_types.size(), nullptr);
  return _types[n];
}

/**
 * Determines the type of the file based on the indicated extension (without a
 * leading dot).  Returns NULL if the extension matches no known file types.
 */
LoaderFileType *LoaderFileTypeRegistry::
get_type_from_extension(const string &extension) {
  string dcextension = downcase(extension);

  Extensions::const_iterator ei;
  ei = _extensions.find(dcextension);
  if (ei == _extensions.end()) {
    // Nothing matches that extension.  Do we have a deferred type?

    DeferredTypes::iterator di;
    di = _deferred_types.find(dcextension);
    if (di != _deferred_types.end()) {
      // We do!  Try to load the deferred library on-the-fly.  Note that this
      // is a race condition if we support threaded loading; this whole
      // function needs to be protected from multiple entry.
      string name = (*di).second;
      Filename dlname = Filename::dso_filename("lib" + name + ".so");
      _deferred_types.erase(di);

      loader_cat->info()
        << "loading file type module: " << name << std::endl;
      void *tmp = load_dso(get_plugin_path().get_value(), dlname);
      if (tmp == nullptr) {
        loader_cat->warning()
          << "Unable to load " << dlname.to_os_specific() << ": "
          << load_dso_error() << std::endl;
        return nullptr;
      } else if (loader_cat.is_debug()) {
        loader_cat.debug()
          << "done loading file type module: " << name << std::endl;
      }

      // Now try again to find the LoaderFileType.
      ei = _extensions.find(dcextension);
    }
  }

  if (ei == _extensions.end()) {
    // Nothing matches that extension, even after we've checked for a deferred
    // type description.
    return nullptr;
  }

  return (*ei).second;
}

/**
 * Writes a list of supported file types to the indicated output stream, one
 * per line.
 */
void LoaderFileTypeRegistry::
write(std::ostream &out, int indent_level) const {
  if (_types.empty()) {
    indent(out, indent_level) << "(No file types are known).\n";
  } else {
    Types::const_iterator ti;
    for (ti = _types.begin(); ti != _types.end(); ++ti) {
      LoaderFileType *type = (*ti);
      string name = type->get_name();
      indent(out, indent_level) << name;
      indent(out, std::max(30 - (int)name.length(), 0)) << " ";

      bool comma = false;
      if (!type->get_extension().empty()) {
        out << " ." << type->get_extension();
        comma = true;
      }

      vector_string words;
      extract_words(type->get_additional_extensions(), words);
      vector_string::const_iterator wi;
      for (wi = words.begin(); wi != words.end(); ++wi) {
        if (comma) {
          out << ",";
        } else {
          comma = true;
        }
        out << " ." << *wi;
      }
      out << "\n";
    }
  }

  if (!_deferred_types.empty()) {
    indent(out, indent_level) << "Also available:";
    DeferredTypes::const_iterator di;
    for (di = _deferred_types.begin(); di != _deferred_types.end(); ++di) {
      const string &extension = (*di).first;
      out << " ." << extension;
    }
    out << "\n";
  }
}

/**
 * Returns a pointer to the global LoaderFileTypeRegistry object.
 */
LoaderFileTypeRegistry *LoaderFileTypeRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new LoaderFileTypeRegistry;
  }
  return _global_ptr;
}

/**
 * Records a filename extension recognized by a loader file type.
 */
void LoaderFileTypeRegistry::
record_extension(const string &extension, LoaderFileType *type) {
  string dcextension = downcase(extension);
  Extensions::const_iterator ei;
  ei = _extensions.find(dcextension);
  if (ei != _extensions.end()) {
    if (loader_cat->is_debug()) {
      loader_cat->debug()
        << "Multiple LoaderFileTypes registered that use the extension "
        << dcextension << "\n";
    }
  } else {
    _extensions.insert(Extensions::value_type(dcextension, type));
  }

  _deferred_types.erase(dcextension);
}
