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
#include "shaderModuleSpirV.h"
#include "config_shaderpipeline.h"
#include "virtualFile.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "dcast.h"

#ifndef CPPPARSER
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
    Shader::SL_GLSL
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

  static bool is_initialized = false;
  if (!is_initialized) {
    ShInitialize();
    is_initialized = true;
  }

  const char *string = (const char *)code.data();
  const int length = (int)code.size();
  const char *fname = filename.c_str();

  glslang::TShader *shader = new glslang::TShader((EShLanguage)stage);
  shader->setStringsWithLengthsAndNames(&string, &length, &fname, 1);
  //shader->setEntryPoint("main");
  //shader->setSourceEntryPoint("main");
  //shader->setEnvInput(glslang::EShSource::EShSourceGlsl, (EShLanguage)stage, glslang::EShClient::EShClientVulkan, 430);
  //shader->setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetVulkan_1_1);
  shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

  // Have the compilers assign bindings to everything--even if we will end up
  // changing them, it's useful if there are already location assignments to
  // overwrite so that we only have to modify instructions and not insert new
  // ones.

  // This will squelch the warnings about missing bindings and locations, since
  // we can assign those ourselves.
  shader->setAutoMapBindings(true);
  shader->setAutoMapLocations(true);

  Includer includer(record);
  if (!shader->parse(&resource_limits, 110, false, (EShMessages)(EShMsgDefault | EShMsgDebugInfo), includer)) {
    std::cerr << "failed to parse " << filename << ":\n";
    std::cerr << shader->getInfoLog() << "\n";
    std::cerr << shader->getInfoDebugLog() << "\n";
    return nullptr;
  }

  glslang::TIntermediate *ir = shader->getIntermediate();
  if (!ir) {
    std::cerr << "failed to obtain IR " << filename << ":\n";
    return nullptr;
  }

  std::vector<unsigned int> spirv;
  std::string warningsErrors;
  spv::SpvBuildLogger logger;
  glslang::SpvOptions spvOptions;
  spvOptions.generateDebugInfo = true;
  spvOptions.disableOptimizer = false;
  spvOptions.optimizeSize = false;
  spvOptions.disassemble = false;
  spvOptions.validate = true;
  glslang::GlslangToSpv(*ir, spirv, &logger, &spvOptions);

  printf("%s", logger.getAllMessages().c_str());
  /*if (Options & EOptionOutputHexadecimal) {
  } else {
      glslang::OutputSpvBin(spirv, GetBinaryName((EShLanguage)stage));
  }*/

  return new ShaderModuleSpirV(stage, spirv.data(), spirv.size());
}
