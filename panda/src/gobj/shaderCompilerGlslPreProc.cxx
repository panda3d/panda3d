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
#include "config_gobj.h"

#include "dcast.h"

// Maps required extensions to shader caps, so we can do a slightly better job
// at checking whether a given shader module is supported by the GSG.
static struct ExtensionCaps { const char *ext; uint64_t caps; } _extension_caps[] = {
  {"GL_ARB_compute_shader", ShaderEnums::C_compute_shader},
  {"GL_ARB_derivative_control", ShaderEnums::C_derivative_control},
  {"GL_ARB_draw_instanced", ShaderEnums::C_instance_id},
  {"GL_ARB_enhanced_layouts", ShaderEnums::C_enhanced_layouts},
  {"GL_ARB_geometry_shader4", ShaderEnums::C_geometry_shader},
  {"GL_ARB_gpu_shader_fp64", ShaderEnums::C_double},
  {"GL_ARB_shader_bit_encoding", ShaderEnums::C_bit_encoding},
  {"GL_ARB_shader_image_load_store", ShaderEnums::C_image_load_store},
  {"GL_ARB_shader_texture_lod", ShaderEnums::C_texture_lod},
  {"GL_ARB_tessellation_shader", ShaderEnums::C_tessellation_shader},
  {"GL_ARB_texture_buffer_object", ShaderEnums::C_texture_buffer},
  {"GL_ARB_texture_query_levels", ShaderEnums::C_texture_query_levels},
  {"GL_ARB_texture_query_lod", ShaderEnums::C_texture_query_lod},
  {"GL_EXT_geometry_shader", ShaderEnums::C_geometry_shader},
  {"GL_EXT_gpu_shader4", ShaderEnums::C_unified_model},
  {"GL_EXT_shader_texture_lod", ShaderEnums::C_texture_lod},
  {"GL_EXT_shadow_samplers", ShaderEnums::C_shadow_samplers},
  {"GL_EXT_tessellation_shader", ShaderEnums::C_tessellation_shader},
  {"GL_OES_sample_variables", ShaderEnums::C_sample_variables},
  {"GL_OES_standard_derivatives", ShaderEnums::C_standard_derivatives},
  {"GL_OES_texture_buffer", ShaderEnums::C_texture_buffer},
  {nullptr, 0},
};

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
  //return ShaderLanguages();
  return {
    Shader::SL_GLSL
  };
}

/**
 * Compiles the source code from the given input stream, producing a
 * ShaderModule on success.
 */
PT(ShaderModule) ShaderCompilerGlslPreProc::
compile_now(Stage stage, std::istream &in, const Filename &fullpath,
            BamCacheRecord *record) const {
  // Create a name that's easier to read in error messages.
  std::string filename;
  if (fullpath.empty()) {
    filename = "created-shader";
  } else {
    Filename fullpath_rel = fullpath;
    if (fullpath_rel.make_relative_to(ExecutionEnvironment::get_environment_variable("MAIN_DIR")) &&
        fullpath_rel.length() < fullpath.length()) {
      filename = fullpath_rel;
    } else {
      filename = fullpath;
    }
  }

  State state;
  if (!r_preprocess_source(state, in, filename, fullpath, record)) {
    return nullptr;
  }

  if (!state.has_code) {
    shader_cat.error()
      << "GLSL shader " << filename << " does not contain any code!\n";
    return nullptr;
  }
  if (!state.version) {
    shader_cat.warning()
      << "GLSL shader " << filename << " does not contain a #version line!\n";
  }

  PT(ShaderModuleGlsl) module = new ShaderModuleGlsl(stage, state.code.str(), state.version);
  module->_included_files = std::move(state.included_files);
  module->_used_caps |= state.required_caps;
  module->_record = record;
  return module;
}

/**
 * Loads a given GLSL stream line by line, processing any #pragma include and
 * once statements, as well as removing any comments.
 *
 * The set keeps track of which files we have already included, for checking
 * recursive includes.
 */
bool ShaderCompilerGlslPreProc::
r_preprocess_source(State &state, std::istream &in, const std::string &fn,
                    const Filename &full_fn, BamCacheRecord *record, int fileno,
                    int depth) const {

  // Iterate over the lines for things we may need to preprocess.
  std::string line;
  int ext_google_include = 0; // 1 = warn, 2 = enable
  int ext_google_line = 0;
  bool had_nested_include = false;
  int lineno = 0;
  bool write_line_directive = (fileno != 0);

  std::ostream &out = state.code;

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
      }
      else break;
    }

    // Look for comments to strip.  This is necessary because comments may
    // appear in the middle of or around a preprocessor definition.
    size_t line_comment = line.find("//");
    size_t block_comment = line.find("/*");
    if (line_comment < block_comment) {
      // A line comment - strip off the rest of the line.
      line.resize(line_comment);
    }
    else if (block_comment < line_comment) {
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
      state.has_code = true;
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
          }
          else if (sscanf(line.c_str(), " # pragma%*[ \t]include <%2047[^>]> %zn", incfile, &nread) == 1
              && nread == line.size()) {
            // Angled includes are also OK, but we don't search in the directory
            // of the source file.
            incfn = incfile;
          }
          else {
            // Couldn't parse it.
            shader_cat.error()
              << "Malformed #pragma include at line " << lineno
              << " of file " << fn << ":\n  " << line << "\n";
            return false;
          }
        }

        // OK, great.  Process the include.
        if (!r_preprocess_include(state, incfn, source_dir, record, depth + 1)) {
          // An error occurred.  Pass on the failure.
          shader_cat.error(false) << "included at line "
            << lineno << " of file " << fn << ":\n  " << line << "\n";
          return false;
        }

        // Restore the line counter.
        write_line_directive = true;
        if (state.cond_nesting > 0) {
          had_nested_include = true;
        }
        state.has_code = true;
        continue;
      }
      else if (strcmp(pragma, "once") == 0) {
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
          state.once_files.insert(full_fn);
        }
        continue;
      }
      // Otherwise, just pass it through to the driver.
    }
    else if (strncmp(directive, "if", 2) == 0) {
      // Keep track of the level of conditional nesting.
      state.cond_nesting++;
    }
    else if (strcmp(directive, "endif") == 0) {
      // Check for an #endif after an include.  We have to restore the line
      // number in case the include happened under an #if block.
      if (had_nested_include) {
        write_line_directive = true;
      }
      state.cond_nesting--;
    }
    else if (strcmp(directive, "version") == 0) {
      if (sscanf(line.c_str(), " # version %d", &state.version) != 1 || state.version <= 0) {
        shader_cat.error()
          << "Invalid version number at line " << lineno << " of file " << fn
          << ": " << line << "\n";
        return false;
      }
    }
    else if (strcmp(directive, "extension") == 0) {
      // Check for special preprocessing extensions.
      char extension[256];
      char behavior[9];
      if (sscanf(line.c_str(), " # extension%*[ \t]%255[^: \t] : %8s", extension, behavior) == 2) {
        // Parse the behavior string.
        int mode;
        if (strcmp(behavior, "require") == 0) {
          mode = 3;
        }
        else if (strcmp(behavior, "enable") == 0) {
          mode = 2;
        }
        else if (strcmp(behavior, "warn") == 0) {
          mode = 1;
        }
        else if (strcmp(behavior, "disable") == 0) {
          mode = 0;
        }
        else {
          shader_cat.error()
            << "Extension directive specifies invalid behavior at line "
            << lineno << " of file " << fn << ":\n  " << line << "\n";
          return false;
        }

        if (strcmp(extension, "all") == 0) {
          if (mode >= 2) {
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
        }
        else if (strcmp(extension, "GL_GOOGLE_include_directive") == 0) {
          // Enable the Google extension support for #include statements.
          // This also implicitly enables GL_GOOGLE_cpp_style_line_directive.
          // This matches the behavior of Khronos' glslang reference compiler.
          ext_google_include = mode;
          ext_google_line = mode;
          continue;
        }
        else if (strcmp(extension, "GL_GOOGLE_cpp_style_line_directive") == 0) {
          // Enables strings in #line statements.
          ext_google_line = mode;
          continue;
        }
        else if (mode == 3 && state.cond_nesting == 0) {
          // Pick up the capabilities used by this extension.
          ExtensionCaps *caps = _extension_caps;
          while (caps->ext) {
            int result = strcmp(extension, caps->ext);
            if (result < 0) {
              break;
            }
            if (result == 0) {
              state.required_caps |= caps->caps;
              break;
            }
            ++caps;
          }
        }
      }
      else {
        shader_cat.error()
          << "Failed to parse extension directive at line "
          << lineno << " of file " << fn << ":\n  " << line << "\n";
        return false;
      }
    }
    else if (ext_google_include > 0 && strcmp(directive, "include") == 0) {
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
      if (!r_preprocess_include(state, incfn, source_dir, record, depth + 1)) {
        // An error occurred.  Pass on the failure.
        shader_cat.error(false) << "included at line "
          << lineno << " of file " << fn << ":\n  " << line << "\n";
        return false;
      }

      // Restore the line counter.
      write_line_directive = true;
      if (state.cond_nesting > 0) {
        had_nested_include = true;
      }
      state.has_code = true;
      continue;
    }
    else if (ext_google_line > 0 && strcmp(directive, "line") == 0) {
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
        fileno = ShaderModuleGlsl::_fileno_offset + (int)state.included_files.size();
        state.included_files.push_back(filestr);
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
r_preprocess_include(State &state, const std::string &fn,
                     const Filename &source_dir, BamCacheRecord *record,
                     int depth) const {

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
  if (state.once_files.find(full_fn) != state.once_files.end()) {
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
  int fileno = ShaderModuleGlsl::_fileno_offset + (int)state.included_files.size();
  state.included_files.push_back(fn);

  if (shader_cat.is_debug()) {
    shader_cat.debug()
      << "Preprocessing shader include " << fileno << ": " << fn << "\n";
  }

  bool result = r_preprocess_source(state, *source, fn, full_fn, record, fileno, depth);
  vf->close_read_file(source);
  return result;
}
