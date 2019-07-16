/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerGlslPreProc.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderCompilerGlslPreProc.h"
#include "shaderModuleGlsl.h"
#include "config_shaderpipeline.h"

#include "dcast.h"

TypeHandle ShaderCompilerGlslPreProc::_type_handle;

/**
 *
 */
ShaderCompilerGlslPreProc::
ShaderCompilerGlslPreProc() {
}

/**
 *
 */
std::string ShaderCompilerGlslPreProc::
get_name() const {
  return "GLSL Pre-Processing Compiler";
}

/**
 *
 */
ShaderLanguages ShaderCompilerGlslPreProc::
get_languages() const {
  return {
    Shader::SL_GLSL
  };
}

/**
 * Compiles the source code from the given input stream, producing a
 * ShaderModule on success.
 */
PT(ShaderModule) ShaderCompilerGlslPreProc::
compile_now(ShaderModule::Stage stage, std::istream &in,
            const std::string &filename, BamCacheRecord *record) const {
  PT(ShaderModuleGlsl) module = new ShaderModuleGlsl(stage);
  std::string &into = module->_raw_source;

  std::ostringstream sstr;
  std::set<Filename> open_files;
  if (r_preprocess_source(module, sstr, in, filename, Filename(), open_files, record)) {
    into = sstr.str();

    // Strip trailing whitespace.
    while (!into.empty() && isspace(into[into.size() - 1])) {
      into.resize(into.size() - 1);
    }

    // Except add back a newline at the end, which is needed by Intel drivers.
    into += "\n";

    module->_record = record;
    return module;
  } else {
    return nullptr;
  }
}

/**
 * Loads a given GLSL stream line by line, processing any #pragma include and
 * once statements, as well as removing any comments.
 *
 * The set keeps track of which files we have already included, for checking
 * recursive includes.
 */
bool ShaderCompilerGlslPreProc::
r_preprocess_source(ShaderModuleGlsl *module,
                    std::ostream &out, std::istream &in, const std::string &fn,
                    const Filename &full_fn, std::set<Filename> &once_files,
                    BamCacheRecord *record, int fileno, int depth) const {

  // Iterate over the lines for things we may need to preprocess.
  std::string line;
  int ext_google_include = 0; // 1 = warn, 2 = enable
  int ext_google_line = 0;
  bool had_include = false;
  bool had_version = false;
  int lineno = 0;
  bool write_line_directive = (fileno != 0);

  while (std::getline(in, line)) {
    ++lineno;

    if (line.empty()) {
      // We still write a newline to make sure the line numbering remains
      // consistent, unless we are about to write a #line directive anyway.
      if (!write_line_directive) {
        out.put('\n');
      }
      continue;
    }

    // If the line ends with a backslash, concatenate the following line.
    // Preprocessor definitions may be broken up into multiple lines.
    while (line[line.size() - 1] == '\\') {
      line.resize(line.size() - 1);
      std::string line2;

      if (std::getline(in, line2)) {
        line += line2;
        if (!write_line_directive) {
          out.put('\n');
        }
        ++lineno;
      } else {
        break;
      }
    }

    // Look for comments to strip.  This is necessary because comments may
    // appear in the middle of or around a preprocessor definition.
    size_t line_comment = line.find("//");
    size_t block_comment = line.find("/*");
    if (line_comment < block_comment) {
      // A line comment - strip off the rest of the line.
      line.resize(line_comment);

    } else if (block_comment < line_comment) {
      // A block comment.  Search for closing block.
      std::string line2 = line.substr(block_comment + 2);

      // According to the GLSL specification, a block comment is replaced with
      // a single whitespace character.
      line.resize(block_comment);
      line += ' ';

      size_t block_end = line2.find("*/");
      while (block_end == std::string::npos) {
        // Didn't find it - look in the next line.
        if (std::getline(in, line2)) {
          if (!write_line_directive) {
            out.put('\n');
          }
          ++lineno;
          block_end = line2.find("*/");
        } else {
          shader_cat.error()
            << "Expected */ before end of file " << fn << "\n";
          return false;
        }
      }

      line += line2.substr(block_end + 2);
    }

    // Strip trailing whitespace.
    while (!line.empty() && isspace(line[line.size() - 1])) {
      line.resize(line.size() - 1);
    }

    if (line.empty()) {
      if (!write_line_directive) {
        out.put('\n');
      }
      continue;
    }

    // Check if this line contains a #directive.
    char directive[64];
    if (line.size() < 8 || sscanf(line.c_str(), " # %63s", directive) != 1) {
      // Nope.  Just pass the line through unmodified.
      if (write_line_directive) {
        out << "#line " << lineno << " " << fileno << " // " << fn << "\n";
        write_line_directive = false;
      }
      out << line << "\n";
      continue;
    }

    char pragma[64];
    size_t nread = 0;
    // What kind of directive is it?
    if (strcmp(directive, "pragma") == 0 &&
        sscanf(line.c_str(), " # pragma %63s", pragma) == 1) {
      if (strcmp(pragma, "include") == 0) {
        // Allow both double quotes and angle brackets.
        Filename incfn, source_dir;
        {
          char incfile[2048];
          if (sscanf(line.c_str(), " # pragma%*[ \t]include \"%2047[^\"]\" %zn", incfile, &nread) == 1
              && nread == line.size()) {
            // A regular include, with double quotes.  Probably a local file.
            source_dir = full_fn.get_dirname();
            incfn = incfile;

          } else if (sscanf(line.c_str(), " # pragma%*[ \t]include <%2047[^\"]> %zn", incfile, &nread) == 1
              && nread == line.size()) {
            // Angled includes are also OK, but we don't search in the directory
            // of the source file.
            incfn = incfile;

          } else {
            // Couldn't parse it.
            shader_cat.error()
              << "Malformed #pragma include at line " << lineno
              << " of file " << fn << ":\n  " << line << "\n";
            return false;
          }
        }

        // OK, great.  Process the include.
        if (!r_preprocess_include(module, out, incfn, source_dir, once_files, record, depth + 1)) {
          // An error occurred.  Pass on the failure.
          shader_cat.error(false) << "included at line "
            << lineno << " of file " << fn << ":\n  " << line << "\n";
          return false;
        }

        // Restore the line counter.
        write_line_directive = true;
        had_include = true;
        continue;

      } else if (strcmp(pragma, "once") == 0) {
        // Do a stricter syntax check, just to be extra safe.
        if (sscanf(line.c_str(), " # pragma%*[ \t]once %zn", &nread) != 0 ||
            nread != line.size()) {
          shader_cat.error()
            << "Malformed #pragma once at line " << lineno
            << " of file " << fn << ":\n  " << line << "\n";
          return false;
        }

        if (fileno == 0) {
          shader_cat.warning()
            << "#pragma once in main file at line "
            << lineno << " of file " << fn
#ifndef NDEBUG
            << ":\n  " << line
#endif
            << "\n";
        }

        if (!full_fn.empty()) {
          once_files.insert(full_fn);
        }
        continue;
      }
      // Otherwise, just pass it through to the driver.

    } else if (strcmp(directive, "endif") == 0) {
      // Check for an #endif after an include.  We have to restore the line
      // number in case the include happened under an #if block.
      if (had_include) {
        write_line_directive = true;
      }

    } else if (strcmp(directive, "version") == 0) {
      had_version = true;

    } else if (strcmp(directive, "extension") == 0) {
      // Check for special preprocessing extensions.
      char extension[256];
      char behavior[9];
      if (sscanf(line.c_str(), " # extension%*[ \t]%255[^: \t] : %8s", extension, behavior) == 2) {
        // Parse the behavior string.
        int mode;
        if (strcmp(behavior, "require") == 0 || strcmp(behavior, "enable") == 0) {
          mode = 2;
        } else if (strcmp(behavior, "warn") == 0) {
          mode = 1;
        } else if (strcmp(behavior, "disable") == 0) {
          mode = 0;
        } else {
          shader_cat.error()
            << "Extension directive specifies invalid behavior at line "
            << lineno << " of file " << fn << ":\n  " << line << "\n";
          return false;
        }

        if (strcmp(extension, "all") == 0) {
          if (mode == 2) {
            shader_cat.error()
              << "Extension directive for 'all' may only specify 'warn' or "
                 "'disable' at line " << lineno << " of file " << fn
              << ":\n  " << line << "\n";
            return false;
          }
          ext_google_include = mode;
          ext_google_line = mode;
          // Still pass it through to the driver, so it can enable other
          // extensions.

        } else if (strcmp(extension, "GL_GOOGLE_include_directive") == 0) {
          // Enable the Google extension support for #include statements.
          // This also implicitly enables GL_GOOGLE_cpp_style_line_directive.
          // This matches the behavior of Khronos' glslang reference compiler.
          ext_google_include = mode;
          ext_google_line = mode;
          continue;

        } else if (strcmp(extension, "GL_GOOGLE_cpp_style_line_directive") == 0) {
          // Enables strings in #line statements.
          ext_google_line = mode;
          continue;
        }
      } else {
        shader_cat.error()
          << "Failed to parse extension directive at line "
          << lineno << " of file " << fn << ":\n  " << line << "\n";
        return false;
      }

    } else if (ext_google_include > 0 && strcmp(directive, "include") == 0) {
      // Warn about extension use if requested.
      if (ext_google_include == 1) {
        shader_cat.warning()
          << "Extension GL_GOOGLE_include_directive is being used at line "
          << lineno << " of file " << fn
#ifndef NDEBUG
          << ":\n  " << line
#endif
          << "\n";
      }

      // This syntax allows only double quotes, not angle brackets.
      Filename incfn;
      {
        char incfile[2048];
        if (sscanf(line.c_str(), " # include%*[ \t]\"%2047[^\"]\" %zn", incfile, &nread) != 1
            || nread != line.size()) {
          // Couldn't parse it.
          shader_cat.error()
            << "Malformed #include at line " << lineno
            << " of file " << fn << ":\n  " << line << "\n";
          return false;
        }
        incfn = incfile;
      }

      // OK, great.  Process the include.
      Filename source_dir = full_fn.get_dirname();
      if (!r_preprocess_include(module, out, incfn, source_dir, once_files, record, depth + 1)) {
        // An error occurred.  Pass on the failure.
        shader_cat.error(false) << "included at line "
          << lineno << " of file " << fn << ":\n  " << line << "\n";
        return false;
      }

      // Restore the line counter.
      write_line_directive = true;
      had_include = true;
      continue;

    } else if (ext_google_line > 0 && strcmp(directive, "line") == 0) {
      // It's a #line directive.  See if it uses a string instead of number.
      char filestr[2048];
      if (sscanf(line.c_str(), " # line%*[ \t]%d%*[ \t]\"%2047[^\"]\" %zn", &lineno, filestr, &nread) == 2
          && nread == line.size()) {
        // Warn about extension use if requested.
        if (ext_google_line == 1) {
          shader_cat.warning()
            << "Extension GL_GOOGLE_cpp_style_line_directive is being used at line "
            << lineno << " of file " << fn
#ifndef NDEBUG
            << ":\n  " << line
#endif
            << "\n";
        }

        // Replace the string line number with an integer.  This is something
        // we can substitute later when parsing the GLSL log from the driver.
        fileno = module->add_included_file(filestr);
        out << "#line " << lineno << " " << fileno << " // " << filestr << "\n";
        continue;
      }
    }

    if (write_line_directive) {
      out << "#line " << lineno << " " << fileno << " // " << fn << "\n";
      write_line_directive = false;
    }
    out << line << "\n";
  }

  if (fileno == 0 && !had_version) {
    shader_cat.warning()
      << "GLSL shader " << fn << " does not contain a #version line!\n";
  }

  return true;
}


/**
 * Loads a given GLSL file line by line, and processes any #pragma include and
 * once statements, as well as removes any comments.
 *
 * The set keeps track of which files we have already included, for checking
 * recursive includes.
 */
bool ShaderCompilerGlslPreProc::
r_preprocess_include(ShaderModuleGlsl *module,
                     std::ostream &out, const std::string &fn,
                     const Filename &source_dir,
                     std::set<Filename> &once_files,
                     BamCacheRecord *record, int depth) const {

  if (depth > glsl_include_recursion_limit) {
    shader_cat.error()
      << "GLSL includes nested too deeply, raise glsl-include-recursion-limit"
         " if necessary\n";
    return false;
  }

  DSearchPath path(get_model_path());
  if (!source_dir.empty()) {
    path.prepend_directory(source_dir);
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vf = vfs->find_file(fn, path);
  if (vf == nullptr) {
    shader_cat.error()
      << "Could not find shader include: " << fn << "\n";
    return false;
  }

  Filename full_fn = vf->get_filename();
  if (once_files.find(full_fn) != once_files.end()) {
    // If this file had a #pragma once, just move on.
    return true;
  }

  std::istream *source = vf->open_read_file(true);
  if (source == nullptr) {
    shader_cat.error()
      << "Could not open shader include: " << fn << "\n";
    return false;
  }

  if (record != nullptr) {
    record->add_dependent_file(vf);
  }
  //module->_source_modified = std::max(module->_source_modified, vf->get_timestamp());
  //module->_source_files.push_back(full_fn);

  // We give each file an unique index.  This is so that we can identify a
  // particular shader in the error output.  We offset them by 2048 so that
  // they are more recognizable.  GLSL doesn't give us anything more useful
  // than that, unfortunately.  Don't do this for the top-level file, though.
  // We don't want anything to get in before a potential #version directive.
  //
  // Write it into the vector so that we can substitute it later when we are
  // parsing the GLSL error log.  Don't store the full filename because it
  // would just be too long to display.
  int fileno = module->add_included_file(fn);

  if (shader_cat.is_debug()) {
    shader_cat.debug()
      << "Preprocessing shader include " << fileno << ": " << fn << "\n";
  }

  bool result = r_preprocess_source(module, out, *source, fn, full_fn, once_files, record, fileno, depth);
  vf->close_read_file(source);
  return result;
}
