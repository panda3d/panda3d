/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerGlslang.cxx
 * @author rdb
 * @date 2020-01-02
 */

#include "shaderCompilerGlslang.h"
#include "config_gobj.h"
#include "virtualFile.h"
#include "shaderCompilerGlslPreProc.h"

#ifndef CPPPARSER
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <spirv-tools/optimizer.hpp>

/**
 * Interface for processing includes via the VirtualFileSystem.
 */
class Includer : public glslang::TShader::Includer {
public:
  using glslang::TShader::Includer::IncludeResult;

  Includer(BamCacheRecord *record) : _record(record) {}

  virtual IncludeResult *includeSystem(const char *header_name, const char *includer_name, size_t depth) override {
    if (shader_cat.is_spam()) {
      shader_cat.spam()
        << "Resolving #include <" << header_name << "> from "
        << includer_name << "\n";
    }

    Filename fn = header_name;
    DSearchPath path(get_model_path());

    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    PT(VirtualFile) vf = vfs->find_file(fn, path);
    if (vf == nullptr) {
      static const std::string error_msg("failed to find file");
      return new IncludeResult("", error_msg.data(), error_msg.size(), nullptr);
    }

    vector_uchar *data = new vector_uchar;
    if (!vf->read_file(*data, true)) {
      static const std::string error_msg("failed to read file");
      return new IncludeResult("", error_msg.data(), error_msg.size(), nullptr);
    }

    if (_record != nullptr) {
      _record->add_dependent_file(vf);
    }

    return new IncludeResult(vf->get_filename(), (const char *)data->data(), data->size(), data);
  }

  virtual IncludeResult *includeLocal(const char *header_name, const char *includer_name, size_t depth) override {
    if (shader_cat.is_spam()) {
      shader_cat.spam()
        << "Resolving #include \"" << header_name << "\" from "
        << includer_name << "\n";
    }

    Filename includer = includer_name;
    Filename fn(includer.get_dirname(), header_name);

    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    PT(VirtualFile) vf = vfs->get_file(fn);
    if (vf == nullptr) {
      // Will try includeSystem instead.
      return nullptr;
    }

    vector_uchar *data = new vector_uchar;
    if (!vf->read_file(*data, true)) {
      static const std::string error_msg("failed to read file");
      return new IncludeResult("", error_msg.data(), error_msg.size(), nullptr);
    }

    if (_record != nullptr) {
      _record->add_dependent_file(vf);
    }

    return new IncludeResult(vf->get_filename(), (const char *)data->data(), data->size(), data);
  }

  virtual void releaseInclude(IncludeResult *result) override {
    if (result != nullptr) {
      delete (vector_uchar *)result->userData;
      delete result;
    }
  }

private:
  BamCacheRecord *_record = nullptr;
};

/**
 * Message consumer for SPIRV-Tools.
 */
static void
log_message(spv_message_level_t level, const char *, const spv_position_t &, const char *msg) {
  NotifySeverity severity = NS_info;
  switch (level) {
  case SPV_MSG_FATAL:
  case SPV_MSG_INTERNAL_ERROR:
    severity = NS_fatal;
    break;
  case SPV_MSG_ERROR:
    severity = NS_error;
    break;
  case SPV_MSG_WARNING:
    severity = NS_warning;
    break;
  case SPV_MSG_INFO:
    severity = NS_info;
    break;
  case SPV_MSG_DEBUG:
    severity = NS_debug;
    break;
  }
  shader_cat.out(severity) << msg << std::endl;
}

#endif  // CPPPARSER

TypeHandle ShaderCompilerGlslang::_type_handle;

/**
 *
 */
ShaderCompilerGlslang::
ShaderCompilerGlslang() {
}

/**
 *
 */
std::string ShaderCompilerGlslang::
get_name() const {
  return "GLslang";
}

/**
 *
 */
ShaderLanguages ShaderCompilerGlslang::
get_languages() const {
  return {
    Shader::SL_GLSL,
    Shader::SL_Cg,
  };
}

/**
 * Compiles the source code from the given input stream, producing a
 * ShaderModule on success.
 */
PT(ShaderModule) ShaderCompilerGlslang::
compile_now(ShaderModule::Stage stage, std::istream &in,
            const Filename &fullpath, BamCacheRecord *record) const {

  vector_uchar code;
  if (!VirtualFile::simple_read_file(&in, code)) {
    shader_cat.error()
      << "Failed to read " << stage << " shader from stream.\n";
    return nullptr;
  }

  bool is_cg = false;
  int glsl_version = 110;

  // Look for a special //Cg header.  Otherwise, it's a GLSL shader.
  if (check_cg_header(code)) {
    is_cg = true;
  }
  else {
    pset<Filename> once_files;
    if (!preprocess_glsl(code, glsl_version, fullpath, once_files, record)) {
      return nullptr;
    }
  }

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

  if (!is_cg && glsl_version < 310 && glsl_version != 150) {
    if (glsl_version != 100 && glsl_version != 110 && glsl_version != 120 &&
        glsl_version != 130 && glsl_version != 140 && glsl_version != 300) {
      shader_cat.error()
        << filename << " uses invalid GLSL version " << glsl_version << ".\n";
      return nullptr;
    }

    shader_cat.warning()
      << filename << " uses deprecated GLSL version " << glsl_version
      << ".  Some features may not work.  Minimum supported version is 330 or 310 es.\n";

    // Fall back to GlslPreProc handler.  Cleaner way to do this?
    static ShaderCompilerGlslPreProc preprocessor;

    std::istringstream stream(std::string((const char *)&code[0], code.size()));
    return preprocessor.compile_now(stage, stream, fullpath, record);
  }

  static bool is_initialized = false;
  if (!is_initialized) {
    ShInitialize();
    is_initialized = true;
  }

  EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules);
  EShLanguage language;
  switch (stage) {
  case ShaderModule::Stage::vertex:
    language = EShLangVertex;
    break;
  case ShaderModule::Stage::tess_control:
    language = EShLangTessControl;
    break;
  case ShaderModule::Stage::tess_evaluation:
    language = EShLangTessEvaluation;
    break;
  case ShaderModule::Stage::geometry:
    language = EShLangGeometry;
    break;
  case ShaderModule::Stage::fragment:
    language = EShLangFragment;
    break;
  case ShaderModule::Stage::compute:
    language = EShLangCompute;
    break;
  default:
    shader_cat.error()
      << "glslang compiler does not support " << stage << " shaders.\n";
    return nullptr;
  }

  glslang::TShader shader(language);

  const char *string = (const char *)code.data();
  const int length = (int)code.size();
  const char *fname = fullpath.c_str();
  shader.setStringsWithLengthsAndNames(&string, &length, &fname, 1);
  shader.setEntryPoint("main");

  // If it's marked as a Cg shader, we compile it with the HLSL front-end.
  if (is_cg) {
    messages = (EShMessages)(messages | EShMsgHlslDX9Compatible | EShMsgHlslLegalization);

    const char *source_entry_point;
    switch (stage) {
    case ShaderModule::Stage::vertex:
      source_entry_point = "vshader";
      break;
    case ShaderModule::Stage::geometry:
      source_entry_point = "gshader";
      break;
    case ShaderModule::Stage::fragment:
      source_entry_point = "fshader";
      break;
    default:
      shader_cat.error()
        << "Cg does not support " << stage << " shaders.\n";
      return nullptr;
    }
    shader.setSourceEntryPoint(source_entry_point);

    // Generate a special preamble to define some functions that Cg defines but
    // HLSL doesn't.  This is sourced from cg_preamble.hlsl.
    extern const char cg_preamble[];
    shader.setPreamble(cg_preamble);

    // We map some sampler types to DX10 syntax, but those use separate
    // samplers/images, so we need to ask glslang to combine these back.
    shader.setTextureSamplerTransformMode(EShTexSampTransUpgradeTextureRemoveSampler);

    shader.setEnvInput(glslang::EShSource::EShSourceHlsl, (EShLanguage)stage, glslang::EShClient::EShClientOpenGL, 120);
  } else {
    shader.setEnvInput(glslang::EShSource::EShSourceGlsl, (EShLanguage)stage, glslang::EShClient::EShClientOpenGL, 450);

    shader.setPreamble("#extension GL_GOOGLE_cpp_style_line_directive : require\n");
  }
  shader.setEnvClient(glslang::EShClient::EShClientOpenGL, glslang::EShTargetOpenGL_450);
  shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

  // This will squelch the warnings about missing bindings and locations, since
  // we can assign those ourselves.
  shader.setAutoMapBindings(true);
  shader.setAutoMapLocations(true);

  Includer includer(record);
  if (!shader.parse(GetDefaultResources(), 110, false, messages, includer)) {
    shader_cat.error()
      << "Failed to parse " << filename << ":\n"
      << shader.getInfoLog();
    return nullptr;
  }

  // I don't know why we need to pretend to link it into a program.  One would
  // think one can just do shader->getIntermediate() and convert that to SPIR-V,
  // but that generates shaders that are ever-so-subtly wrong, see: glslang#2418
  ShaderModuleSpirV::InstructionStream stream;
  {
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages)) {
      shader_cat.error()
        << "Failed to link " << filename << ":\n"
        << program.getInfoLog();
      return nullptr;
    }

    glslang::TIntermediate *ir = program.getIntermediate((EShLanguage)stage);
    nassertr(ir != nullptr, nullptr);

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    spvOptions.generateDebugInfo = true;
    spvOptions.disableOptimizer = true;
    spvOptions.optimizeSize = false;
    spvOptions.disassemble = false;
    spvOptions.validate = false;
    glslang::GlslangToSpv(*ir, stream, &logger, &spvOptions);

    std::string messages = logger.getAllMessages();
    if (!messages.empty()) {
      shader_cat.warning()
        << "Compilation to SPIR-V produced the following messages:\n"
        << messages;
    }
  }

  if (!stream.validate_header()) {
    return nullptr;
  }

  // Special validation for features in GLSL 330 that are not in GLSL 150.
  if (glsl_version == 150 && !postprocess_glsl150(stream)) {
    return nullptr;
  }

  if (is_cg && !postprocess_cg(stream)) {
    return nullptr;
  }

  // Run it through the optimizer.
  spvtools::Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  opt.SetMessageConsumer(log_message);
  opt.RegisterPerformancePasses();

  if (is_cg) {
    opt.RegisterLegalizationPasses();
  }

  // We skip validation because of the `uniform bool` bug, see SPIRV-Tools#3387
  std::vector<uint32_t> optimized;
  spvtools::ValidatorOptions validator_options;
  if (!opt.Run(stream.get_data(), stream.get_data_size(), &optimized,
               validator_options, true)) {
    return nullptr;
  }

  return new ShaderModuleSpirV(stage, std::move(optimized), record);
}

/**
 * Returns true if there is a special //Cg header at the beginning of the code.
 */
bool ShaderCompilerGlslang::
check_cg_header(const vector_uchar &code) {
  const char *p = (const char *)&code[0];
  const char *end = p + code.size();
  while (p < end && isspace(*p)) {
    // Skip leading whitespace.
    ++p;
  }
  return (end - p) >= 5
      && strncmp(p, "//Cg", 4) == 0
      && isspace(p[4]);
}

/**
 * Do some very basic preprocessing of the GLSL shader to extract the GLSL
 * version and fix the use of #pragma include (which glslang doesn't support,
 * but we historically did).
 * Returns false if any errors occurred.
 */
bool ShaderCompilerGlslang::
preprocess_glsl(vector_uchar &code, int &glsl_version, const Filename &source_filename,
                pset<Filename> &once_files, BamCacheRecord *record) {
  // Make sure it ends with a newline.  This makes parsing easier.
  if (!code.empty() && code.back() != (unsigned char)'\n') {
    code.push_back((unsigned char)'\n');
  }

  char *p = (char *)&code[0];
  char *end = (char *)&code[0] + code.size();
  int lineno = 1;
  bool had_include = false;

  while (p < end) {
    if (isspace(*p)) {
      if (*p == '\n') ++lineno;
      ++p;
      continue;
    }
    if (*p == '/') {
      // Check for comment, so that we don't pick up commented-out preprocessor
      // directives.
      ++p;
      if (*p == '/') {
        // Skip till end of line.
        do { ++p; } while (*p != '\n');
        ++lineno;
      }
      else if (*p == '*') {
        // Skip till */
        do {
          ++p;
          if (*p == '\n') ++lineno;
        } while ((p + 1) < end && (p[0] != '*' || p[1] != '/'));
        ++p;
      }
      else if (*p == '\n') {
        ++lineno;
      }
    }
    else if (*p == '#') {
      // Skip whitespace after # to find start of preprocessor directive.
      char *line_start = p;
      do { ++p; } while (isspace(*p) && *p != '\n');

      // Read directive keyword
      char *directive = p;
      do { ++p; } while (!isspace(*p));
      size_t directive_size = p - directive;

      // Skip whitespace until we reach EOL or beginning of argument
      while (isspace(*p) && *p != '\n') {
        ++p;
      }

      if (directive_size == 7 && strncmp(directive, "version", directive_size) == 0) {
        glsl_version = strtol(p, &p, 10);

        if (!isspace(*p) || glsl_version <= 0) {
          shader_cat.error(false)
            << "ERROR: " << source_filename << ":" << lineno
            << ": invalid version number in #version directive\n";
          return false;
        }

        if (glsl_version == 150) {
          // glslang doesn't support 150, but it's almost identical to 330,
          // and we have many shaders written in 150, so sneakily change
          // the version number.  We'll do some extra validation later.
          p[-3] = '3';
          p[-2] = '3';
          p[-1] = '0';
        }
        else if (glsl_version < 310) {
          // We're done here, the rest is handled by the GLSL preprocessor.
          return true;
        }
      }
      else if (directive_size == 6 && glsl_preprocess &&
               strncmp(directive, "pragma", directive_size) == 0) {
        if (strncmp(p, "include", 7) == 0 && !isalnum(p[7]) && p[7] != '_') {
          // We insert the included file ourselves, so that we can still handle
          // any `#pragma include` in included files.  This implementation is
          // probably fairly inefficient, but that's okay, because this is just
          // here to support a deprecated behavior.

          static bool warned = false;
          if (!warned) {
            warned = true;
            shader_cat.warning(false)
              << "WARNING: " << source_filename << ":" << lineno
              << ": #pragma include is deprecated, use the "
                 "GL_GOOGLE_include_directive extension instead.\n";
          }

          p += 7;
          while (isspace(*p) && *p != '\n') { ++p; }

          char quote = *p;
          if (quote != '"' && quote != '<') {
            shader_cat.error(false)
              << "ERROR: " << source_filename << ":" << lineno
              << ": expected < or \" after #pragma include\n";
            return false;
          }

          *p = '"';
          char *fn_start = ++p;
          if (quote == '<') {
            quote = '>';
          }
          while (*p != quote && *p != '\n') {
            ++p;
          }
          if (*p != quote) {
            shader_cat.error(false)
              << "ERROR: " << source_filename << ":" << lineno
              << ": malformed #pragma include, expected " << quote << " before EOL\n";
            return false;
          }

          Filename fn = std::string(fn_start, p);
          DSearchPath path(get_model_path());
          if (quote == '"') {
            // A regular include, with double quotes.  Probably a local file.
            if (!source_filename.empty()) {
              path.prepend_directory(source_filename.get_dirname());
            }
          } else {
            *p = '"';
          }

          // Expect EOL.
          do { ++p; } while (isspace(*p) && *p != '\n');
          if (*p != '\n') {
            shader_cat.error(false)
              << "ERROR: " << source_filename << ":" << lineno
              << ": unexpected '" << *p << "' before EOL\n";
            return false;
          }
          ++lineno;
          ++p;

          VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
          PT(VirtualFile) vf = vfs->find_file(fn, path);
          if (vf == nullptr) {
            shader_cat.error(false)
              << "ERROR: " << source_filename << ":" << lineno
              << ": failed to find included file: " << fn << "\n";
            return false;
          }

          Filename full_fn = vf->get_filename();
          if (once_files.count(full_fn)) {
            continue;
          }

          vector_uchar inc_code;
          if (!vf->read_file(inc_code, true)) {
            shader_cat.error(false)
              << "ERROR: " << source_filename << ":" << lineno
              << ": failed to read included file: " << fn << "\n";
            return false;
          }

          int nested_glsl_version = 0;
          if (!preprocess_glsl(inc_code, nested_glsl_version, full_fn, once_files, record)) {
            return false;
          }
          if (nested_glsl_version != 0) {
            shader_cat.error(false)
              << "ERROR: " << fn
              << ": included file should not specify #version\n";
            return false;
          }

          // Replace this line with the next:
          // #pragma include "test.glsl"
          // #line 1         "test.glsl"
          // We know that the second one must be less long than the first one,
          // so we overwrite everything up to the quote with spaces.
          memcpy(line_start, "#line 1", 7);
          memset(line_start + 7, ' ', fn_start - line_start - 8);

          // Restore the #line number by appending it to the included code.
          std::ostringstream line_str;
          line_str << "#line " << lineno << " \"" << source_filename << "\"\n";
          std::string line = line_str.str();
          std::copy((unsigned char *)line.data(), (unsigned char *)line.data() + line.size(),
                    std::back_inserter(inc_code));

          // Insert the code bytes and reposition the pointer after it.
          size_t offset = p - (char *)&code[0];
          code.insert(code.begin() + offset, inc_code.begin(), inc_code.end());
          p = (char *)&code[0] + offset + inc_code.size();
          end = (char *)&code[0] + code.size();
          had_include = true;
          continue;
        }
        else if (strncmp(p, "once", 4) == 0 && isspace(p[4])) {
          // Expect EOL.
          p += 4;
          while (isspace(*p) && *p != '\n') {
            ++p;
          }
          if (*p != '\n') {
            shader_cat.error(false)
              << "ERROR: " << source_filename << ":" << lineno
              << ": unexpected '" << *p << "' before EOL\n";
            return false;
          }

          if (source_filename.empty()) {
            shader_cat.warning(false)
              << "WARNING: " << source_filename << ":" << lineno
              << ": ignoring #pragma once in main file\n";
          } else {
            once_files.insert(source_filename);
          }

          // Blank out the whole line to avoid glslang warning.
          memset(line_start, ' ', p - line_start);
          ++lineno;
          ++p;
          continue;
        }
        else if (strncmp(p, "optionNV", 8) == 0 && isspace(p[8])) {
          // glslang struggles to parse this pragma, so blank it out.
          static bool warned = false;
          if (!warned) {
            warned = true;
            shader_cat.warning(false)
              << "WARNING: " << source_filename << ":" << lineno
              << ": #pragma optionNV is ignored.\n";
          }

          p += 8;
          while (*p != '\n') {
            ++p;
          }
          memset(line_start, ' ', p - line_start);
          ++lineno;
          ++p;
          continue;
        }
      }
      else if (directive_size == 5 && strncmp(directive, "endif", directive_size) == 0) {
        // Check for an #endif after an include.  We have to restore the line
        // number in case the include happened under an #if block.  Again, this
        // is inefficient, but it doesn't matter.
        if (had_include) {
          while (*p != '\n') { ++p; }
          ++lineno;
          ++p;

          std::ostringstream line_str;
          line_str << "#line " << lineno << " \"" << source_filename << "\"\n";
          std::string line = line_str.str();

          size_t offset = p - (char *)&code[0];
          code.insert(code.begin() + offset, (unsigned char *)line.data(),
                      (unsigned char *)line.data() + line.size());

          p = (char *)&code[0] + offset + line.size();
          end = (char *)&code[0] + code.size();
          continue;
        }
      }

      // Skip the rest of the line.
      while (*p != '\n') { ++p; }
      ++lineno;
    }

    ++p;
  }

  return true;
}

/**
 * Validates that the given SPIR-V shader does not use features that are
 * available in GLSL 330 but not in GLSL 150.
 */
bool ShaderCompilerGlslang::
postprocess_glsl150(ShaderModuleSpirV::InstructionStream &stream) {
  bool has_bit_encoding = false;
  bool has_explicit_location = false;

  for (ShaderModuleSpirV::Instruction op : stream) {
    if (op.opcode == spv::OpSource) {
      // Set this back to 150.
      if (op.nargs >= 2) {
        op.args[1] = 150;
      }
    }
    else if (op.opcode == spv::OpSourceExtension) {
      if (strcmp((const char *)op.args, "GL_ARB_shader_bit_encoding") == 0 ||
          strcmp((const char *)op.args, "GL_ARB_gpu_shader5") == 0) {
        has_bit_encoding = true;
      }
      else if (strcmp((const char *)op.args, "GL_ARB_explicit_attrib_location") == 0) {
        has_explicit_location = true;
      }
    }
    else if (op.opcode == spv::OpDecorate && op.nargs >= 2 &&
             (spv::Decoration)op.args[1] == spv::DecorationLocation &&
             !has_explicit_location) {
      shader_cat.error()
        << "Explicit location assignments require #version 330 or "
           "#extension GL_ARB_explicit_attrib_location\n";
      return false;
    }
    else if (op.opcode == spv::OpBitcast && !has_bit_encoding) {
      shader_cat.error()
        << "floatBitsToInt, floatBitsToUint, intBitsToFloat, uintBitsToFloat"
           " require #version 330 or #extension GL_ARB_shader_bit_encoding.\n";
      return false;
    }
  }

  return true;
}

/**
 * Does any postprocessing needed for Cg.
 */
bool ShaderCompilerGlslang::
postprocess_cg(ShaderModuleSpirV::InstructionStream &stream) {
  pset<uint32_t> glsl_imports;

  for (ShaderModuleSpirV::Instruction op : stream) {
    switch (op.opcode) {
    case spv::OpExtInstImport:
      if (strcmp((const char*)&op.args[1], "GLSL.std.450") == 0) {
        glsl_imports.insert(op.args[0]);
      }
      break;

    case spv::OpExtInst:
      // glslang maps round() to roundEven(), which is correct for SM 4.0+ but
      // not supported on pre-DX10 hardware, and Cg made no guarantee of
      // round-to-even behavior to begin with, so we switch it back to round().
      if (glsl_imports.count(op.args[2]) && op.args[3] == 2) {
        op.args[3] = 1;
      }
      break;

    case spv::OpTypeImage:
      // glslang marks buffer textures as having a specific format, but we want
      // to allow the use of any format, so wipe this field.
      if (op.args[6] == 1) {
        op.args[7] = spv::ImageFormatUnknown;
      }
      break;

    default:
      break;
    }
  }

  return true;
}
