/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerRegistry.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderCompilerRegistry.h"
#include "shaderCompiler.h"
#include "config_shaderpipeline.h"
#include "config_putil.h"

#include "load_dso.h"
#include "string_utils.h"
#include "indent.h"

#include <algorithm>

using std::string;

ShaderCompilerRegistry *ShaderCompilerRegistry::_global_ptr;

/**
 *
 */
ShaderCompilerRegistry::
ShaderCompilerRegistry() {
}

/**
 *
 */
ShaderCompilerRegistry::
~ShaderCompilerRegistry() {
}

/**
 * Defines a new ShaderCompiler in the universe.
 */
void ShaderCompilerRegistry::
register_compiler(ShaderCompiler *compiler) {
  // Make sure we haven't already registered this compiler.
  if (find(_compilers.begin(), _compilers.end(), compiler) != _compilers.end()) {
    if (shaderpipeline_cat->is_debug()) {
      shaderpipeline_cat->debug()
        << "Attempt to register ShaderCompiler " << compiler->get_name()
        << " (" << compiler->get_type() << ") more than once.\n";
    }
    return;
  }

  _compilers.push_back(compiler);

  ShaderLanguages langs = compiler->get_languages();
  for (auto langit = langs.begin(); langit != langs.end(); ++langit) {
    record_language(*langit, compiler);
  }
}

/**
 * Records a compiler associated with a particular language to be loaded in the
 * future.  The named library will be dynamically loaded the first time files
 * of this language are loaded; presumably this library will call
 * register_compiler() when it initializes, thus making the language loadable.
 */
void ShaderCompilerRegistry::
register_deferred_compiler(Shader::ShaderLanguage language, const string &library) {
  Languages::const_iterator li;
  li = _languages.find(language);
  if (li != _languages.end()) {
    // We already have a loader for this compiler; no need to register another
    // one.
    if (shaderpipeline_cat->is_debug()) {
      shaderpipeline_cat->debug()
        << "Attempt to register loader library " << library
        << " (" << language << ") when language is already known.\n";
    }
    return;
  }

  DeferredCompilers::const_iterator di;
  di = _deferred_compilers.find(language);
  if (di != _deferred_compilers.end()) {
    if ((*di).second == library) {
      if (shaderpipeline_cat->is_debug()) {
        shaderpipeline_cat->debug()
          << "Attempt to register loader library " << library
          << " (" << language << ") more than once.\n";
      }
      return;
    } else {
      if (shaderpipeline_cat->is_debug()) {
        shaderpipeline_cat->debug()
          << "Multiple libraries registered that use the language "
          << language << "\n";
      }
    }
  }

  _deferred_compilers[language] = library;
}

/**
 * Returns the total number of compilers registered.
 */
int ShaderCompilerRegistry::
get_num_compilers() const {
  return _compilers.size();
}

/**
 * Returns the nth compiler registered.
 */
ShaderCompiler *ShaderCompilerRegistry::
get_compiler(int n) const {
  nassertr(n >= 0 && n < (int)_compilers.size(), nullptr);
  return _compilers[n];
}

/**
 * Determines the compiler of the file based on the indicated language (without a
 * leading dot).  Returns NULL if the language matches no known file compilers.
 */
ShaderCompiler *ShaderCompilerRegistry::
get_compiler_from_language(Shader::ShaderLanguage language) {
  Languages::const_iterator li;
  li = _languages.find(language);
  if (li == _languages.end()) {
    // Nothing matches that language.  Do we have a deferred compiler?

    DeferredCompilers::iterator di;
    di = _deferred_compilers.find(language);
    if (di != _deferred_compilers.end()) {
      // We do!  Try to load the deferred library on-the-fly.  Note that this
      // is a race condition if we support threaded loading; this whole
      // function needs to be protected from multiple entry.
      string name = (*di).second;
      Filename dlname = Filename::dso_filename("lib" + name + ".so");
      _deferred_compilers.erase(di);

      shaderpipeline_cat->info()
        << "loading file compiler module: " << name << std::endl;
      void *tmp = load_dso(get_plugin_path().get_value(), dlname);
      if (tmp == nullptr) {
        shaderpipeline_cat->warning()
          << "Unable to load " << dlname.to_os_specific() << ": "
          << load_dso_error() << std::endl;
        return nullptr;
      } else if (shaderpipeline_cat.is_debug()) {
        shaderpipeline_cat.debug()
          << "done loading file compiler module: " << name << std::endl;
      }

      // Now try again to find the ShaderCompiler.
      li = _languages.find(language);
    }
  }

  if (li == _languages.end()) {
    // Nothing matches that language, even after we've checked for a deferred
    // compiler description.
    return nullptr;
  }

  return (*li).second;
}

/**
 * Writes a list of supported file compilers to the indicated output stream, one
 * per line.
 */
void ShaderCompilerRegistry::
write(std::ostream &out, int indent_level) const {
  if (_compilers.empty()) {
    indent(out, indent_level) << "(No file compilers are known).\n";
  } else {
    Compilers::const_iterator ti;
    for (ti = _compilers.begin(); ti != _compilers.end(); ++ti) {
      ShaderCompiler *compiler = (*ti);
      string name = compiler->get_name();
      indent(out, indent_level) << name;
      indent(out, std::max(30 - (int)name.length(), 0)) << " ";

      bool comma = false;
      ShaderLanguages langs = compiler->get_languages();
      for (auto li = langs.begin(); li != langs.end(); ++li) {
        if (comma) {
          out << ",";
        } else {
          comma = true;
        }
        out << " ." << *li;
      }
      out << "\n";
    }
  }

  if (!_deferred_compilers.empty()) {
    indent(out, indent_level) << "Also available:";
    DeferredCompilers::const_iterator di;
    for (di = _deferred_compilers.begin(); di != _deferred_compilers.end(); ++di) {
        Shader::ShaderLanguage language = (*di).first;
      out << " ." << language;
    }
    out << "\n";
  }
}

/**
 * Returns a pointer to the global ShaderCompilerRegistry object.
 */
ShaderCompilerRegistry *ShaderCompilerRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new ShaderCompilerRegistry;
  }
  return _global_ptr;
}

/**
 * Records a Shader::ShaderLanguage recognized by a shader compiler.
 */
void ShaderCompilerRegistry::
record_language(Shader::ShaderLanguage language, ShaderCompiler *compiler) {
  Languages::const_iterator li;
  li = _languages.find(language);
  if (li != _languages.end()) {
    if (shaderpipeline_cat->is_debug()) {
      shaderpipeline_cat->debug()
        << "Multiple ShaderCompilers registered that use the language "
        << language << "\n";
    }
  } else {
    _languages.insert(Languages::value_type(language, compiler));
  }

  _deferred_compilers.erase(language);
}
