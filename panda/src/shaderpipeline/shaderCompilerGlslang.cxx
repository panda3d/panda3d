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
#include "config_shaderpipeline.h"
#include "virtualFile.h"
#include "shaderCompilerGlslPreProc.h"

#ifndef CPPPARSER
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <spirv-tools/optimizer.hpp>

const TBuiltInResource resource_limits = {
  /* .MaxLights = */ 32,
  /* .MaxClipPlanes = */ 6,
  /* .MaxTextureUnits = */ 32,
  /* .MaxTextureCoords = */ 32,
  /* .MaxVertexAttribs = */ 64,
  /* .MaxVertexUniformComponents = */ 4096,
  /* .MaxVaryingFloats = */ 64,
  /* .MaxVertexTextureImageUnits = */ 32,
  /* .MaxCombinedTextureImageUnits = */ 80,
  /* .MaxTextureImageUnits = */ 32,
  /* .MaxFragmentUniformComponents = */ 4096,
  /* .MaxDrawBuffers = */ 32,
  /* .MaxVertexUniformVectors = */ 128,
  /* .MaxVaryingVectors = */ 8,
  /* .MaxFragmentUniformVectors = */ 16,
  /* .MaxVertexOutputVectors = */ 16,
  /* .MaxFragmentInputVectors = */ 15,
  /* .MinProgramTexelOffset = */ -8,
  /* .MaxProgramTexelOffset = */ 7,
  /* .MaxClipDistances = */ 8,
  /* .MaxComputeWorkGroupCountX = */ 65535,
  /* .MaxComputeWorkGroupCountY = */ 65535,
  /* .MaxComputeWorkGroupCountZ = */ 65535,
  /* .MaxComputeWorkGroupSizeX = */ 1024,
  /* .MaxComputeWorkGroupSizeY = */ 1024,
  /* .MaxComputeWorkGroupSizeZ = */ 64,
  /* .MaxComputeUniformComponents = */ 1024,
  /* .MaxComputeTextureImageUnits = */ 16,
  /* .MaxComputeImageUniforms = */ 8,
  /* .MaxComputeAtomicCounters = */ 8,
  /* .MaxComputeAtomicCounterBuffers = */ 1,
  /* .MaxVaryingComponents = */ 60,
  /* .MaxVertexOutputComponents = */ 64,
  /* .MaxGeometryInputComponents = */ 64,
  /* .MaxGeometryOutputComponents = */ 128,
  /* .MaxFragmentInputComponents = */ 128,
  /* .MaxImageUnits = */ 8,
  /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
  /* .MaxCombinedShaderOutputResources = */ 8,
  /* .MaxImageSamples = */ 0,
  /* .MaxVertexImageUniforms = */ 0,
  /* .MaxTessControlImageUniforms = */ 0,
  /* .MaxTessEvaluationImageUniforms = */ 0,
  /* .MaxGeometryImageUniforms = */ 0,
  /* .MaxFragmentImageUniforms = */ 8,
  /* .MaxCombinedImageUniforms = */ 8,
  /* .MaxGeometryTextureImageUnits = */ 16,
  /* .MaxGeometryOutputVertices = */ 256,
  /* .MaxGeometryTotalOutputComponents = */ 1024,
  /* .MaxGeometryUniformComponents = */ 1024,
  /* .MaxGeometryVaryingComponents = */ 64,
  /* .MaxTessControlInputComponents = */ 128,
  /* .MaxTessControlOutputComponents = */ 128,
  /* .MaxTessControlTextureImageUnits = */ 16,
  /* .MaxTessControlUniformComponents = */ 1024,
  /* .MaxTessControlTotalOutputComponents = */ 4096,
  /* .MaxTessEvaluationInputComponents = */ 128,
  /* .MaxTessEvaluationOutputComponents = */ 128,
  /* .MaxTessEvaluationTextureImageUnits = */ 16,
  /* .MaxTessEvaluationUniformComponents = */ 1024,
  /* .MaxTessPatchComponents = */ 120,
  /* .MaxPatchVertices = */ 32,
  /* .MaxTessGenLevel = */ 64,
  /* .MaxViewports = */ 16,
  /* .MaxVertexAtomicCounters = */ 0,
  /* .MaxTessControlAtomicCounters = */ 0,
  /* .MaxTessEvaluationAtomicCounters = */ 0,
  /* .MaxGeometryAtomicCounters = */ 0,
  /* .MaxFragmentAtomicCounters = */ 8,
  /* .MaxCombinedAtomicCounters = */ 8,
  /* .MaxAtomicCounterBindings = */ 1,
  /* .MaxVertexAtomicCounterBuffers = */ 0,
  /* .MaxTessControlAtomicCounterBuffers = */ 0,
  /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
  /* .MaxGeometryAtomicCounterBuffers = */ 0,
  /* .MaxFragmentAtomicCounterBuffers = */ 1,
  /* .MaxCombinedAtomicCounterBuffers = */ 1,
  /* .MaxAtomicCounterBufferSize = */ 16384,
  /* .MaxTransformFeedbackBuffers = */ 4,
  /* .MaxTransformFeedbackInterleavedComponents = */ 64,
  /* .MaxCullDistances = */ 8,
  /* .MaxCombinedClipAndCullDistances = */ 8,
  /* .MaxSamples = */ 4,
  /* .maxMeshOutputVerticesNV = */ 256,
  /* .maxMeshOutputPrimitivesNV = */ 512,
  /* .maxMeshWorkGroupSizeX_NV = */ 32,
  /* .maxMeshWorkGroupSizeY_NV = */ 1,
  /* .maxMeshWorkGroupSizeZ_NV = */ 1,
  /* .maxTaskWorkGroupSizeX_NV = */ 32,
  /* .maxTaskWorkGroupSizeY_NV = */ 1,
  /* .maxTaskWorkGroupSizeZ_NV = */ 1,
  /* .maxMeshViewCountNV = */ 4,
  /* .maxDualSourceDrawBuffersEXT = */ 1,

  /* .limits = */ {
      /* .nonInductiveForLoops = */ 1,
      /* .whileLoops = */ 1,
      /* .doWhileLoops = */ 1,
      /* .generalUniformIndexing = */ 1,
      /* .generalAttributeMatrixVectorIndexing = */ 1,
      /* .generalVaryingIndexing = */ 1,
      /* .generalSamplerIndexing = */ 1,
      /* .generalVariableIndexing = */ 1,
      /* .generalConstantMatrixVectorIndexing = */ 1,
  }
};

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
      static const std::string error_msg("failed to find file");
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
      static const std::string error_msg("failed to find file");
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
  NotifySeverity severity;
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
            const std::string &filename, BamCacheRecord *record) const {

  vector_uchar code;
  if (!VirtualFile::simple_read_file(&in, code)) {
    return nullptr;
  }

  bool is_cg = false;
  bool add_include_directive = false;
  int glsl_version = 110;

  // Look for a special //Cg header.  Otherwise, it's a GLSL shader.
  if (check_cg_header(code)) {
    is_cg = true;
  }
  else if (!preprocess_glsl(code, glsl_version, add_include_directive)) {
    return nullptr;
  }

  if (!is_cg && glsl_version < 330 && glsl_version != 150) {
    // Fall back to GlslPreProc handler.  Cleaner way to do this?
    static ShaderCompilerGlslPreProc preprocessor;

    std::istringstream stream(std::string((const char *)&code[0], code.size()));
    return preprocessor.compile_now(stage, stream, filename, record);
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
  const char *fname = filename.c_str();
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

    // We map shadow samplers to DX10 syntax, but those use separate samplers/
    // images, so we need to ask glslang to kindly combine these back.
    shader.setTextureSamplerTransformMode(EShTexSampTransUpgradeTextureRemoveSampler);

    shader.setEnvInput(glslang::EShSource::EShSourceHlsl, (EShLanguage)stage, glslang::EShClient::EShClientOpenGL, 120);
  } else {
    shader.setEnvInput(glslang::EShSource::EShSourceGlsl, (EShLanguage)stage, glslang::EShClient::EShClientOpenGL, 450);

    if (add_include_directive) {
      shader.setPreamble(
        "#extension GL_GOOGLE_include_directive : require\n"
      );
    }
  }
  shader.setEnvClient(glslang::EShClient::EShClientOpenGL, glslang::EShTargetOpenGL_450);
  shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

  // This will squelch the warnings about missing bindings and locations, since
  // we can assign those ourselves.
  shader.setAutoMapBindings(true);
  shader.setAutoMapLocations(true);

  Includer includer(record);
  if (!shader.parse(&resource_limits, 110, false, messages, includer)) {
    shader_cat.error()
      << "Failed to parse " << filename << ":\n"
      << shader.getInfoLog();
    return nullptr;
  }

  // I don't know why we need to pretend to link it into a program.  One would
  // think one can just do shader->getIntermediate() and convert that to SPIR-V,
  // but that generates shaders that are ever-so-subtly wrong.
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
    spvOptions.disableOptimizer = false;
    spvOptions.optimizeSize = false;
    spvOptions.disassemble = false;
    spvOptions.validate = true;
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

  // Run it through the optimizer.
  spvtools::Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  opt.SetMessageConsumer(log_message);
  opt.RegisterPerformancePasses();

  if (is_cg) {
    opt.RegisterLegalizationPasses();
  }

  std::vector<uint32_t> optimized;
  if (!opt.Run(stream.get_data(), stream.get_data_size(), &optimized)) {
    return nullptr;
  }

  return new ShaderModuleSpirV(stage, std::move(optimized));
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
preprocess_glsl(vector_uchar &code, int &glsl_version, bool &uses_pragma_include) {
  glsl_version = 110;

  // Make sure it ends with a newline.  This makes parsing easier.
  if (!code.empty() && code.back() != (unsigned char)'\n') {
    code.push_back((unsigned char)'\n');
  }

  char *p = (char *)&code[0];
  char *end = (char *)&code[0] + code.size();
  bool has_code = false;

  while (p < end) {
    if (isspace(*p)) {
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
      }
      else if (*p == '*') {
        // Skip till */
        do { ++p; } while ((p + 1) < end && (p[0] != '*' || p[1] != '/'));
      }
    }
    else if (*p == '#') {
      // Check for a preprocessor directive.
      do { ++p; } while (isspace(*p) && *p != '\n');
      char *directive = p;
      do { ++p; } while (!isspace(*p));
      size_t directive_size = p - directive;
      do { ++p; } while (isspace(*p) && *p != '\n');

      if (directive_size == 7 && strncmp(directive, "version", directive_size) == 0) {
        glsl_version = strtol(p, &p, 10);

        if (!isspace(*p)) {
          shader_cat.error()
            << "Invalid version number in #version directive\n";
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
      }
      else if (directive_size == 6 && glsl_preprocess &&
               strncmp(directive, "pragma", directive_size) == 0) {
        if (strncmp(p, "include", 7) == 0 && !isalnum(p[7]) && p[7] != '_') {
          // Turn this into a normal include directive (by replacing the word
          // "pragma" with spaces) and enable the GL_GOOGLE_include_directive
          // extension using the preamble.
          memset(directive, ' ', directive_size);
          uses_pragma_include = true;
          has_code = true;

          static bool warned = false;
          if (!warned) {
            warned = true;
            shader_cat.warning()
              << "#pragma include is deprecated, use the "
                 "GL_GOOGLE_include_directive extension instead.\n";
          }
        }
      }
      else {
        has_code = true;
      }

      // Skip the rest of the line.
      while (*p != '\n') { ++p; }
    }
    else {
      has_code = true;
    }

    ++p;
  }

  if (!has_code) {
    shader_cat.error()
      << "Shader contains no code\n";
    return false;
  }

  if (glsl_version < 330 && glsl_version != 150) {
    if (glsl_version == 100 || glsl_version == 110 || glsl_version == 120 ||
        glsl_version == 130 || glsl_version == 140 || glsl_version == 300) {
      shader_cat.warning()
        << "Support for GLSL " << glsl_version << " is deprecated.  Some "
           "features may not work.  Minimum supported version is GLSL 330.\n";
      return true;
    }
    else {
      shader_cat.error()
        << "Invalid GLSL version " << glsl_version << ".\n";
      return false;
    }
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
