/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerCg.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderCompilerCg.h"
#include "config_shaderpipeline.h"

#ifdef HAVE_CG
#include <Cg/cg.h>
#endif

#include "dcast.h"

static const std::string vertex_profiles[] = {
  "gp4vp",
  "gp5vp",
  "glslv",
  "arbvp1",
  "vp40",
  "vp30",
  "vp20",
  "vs_1_1",
  "vs_2_0",
  "vs_2_x",
  "vs_3_0",
  "vs_4_0",
  "vs_5_0"
};

static const std::string fragment_profiles[] = {
  "gp4fp",
  "gp5fp",
  "glslf",
  "arbfp1",
  "fp40",
  "fp30",
  "fp20",
  "ps_1_1",
  "ps_1_2",
  "ps_1_3",
  "ps_2_0",
  "ps_2_x",
  "ps_3_0",
  "ps_4_0",
  "ps_5_0"
};

static const std::string geometry_profiles[] = {
  "gp4gp",
  "gp5gp",
  "glslg",
  "gs_4_0",
  "gs_5_0"
};

TypeHandle ShaderCompilerCg::_type_handle;

/**
 *
 */
ShaderCompilerCg::
ShaderCompilerCg() {
}

/**
 * Determines the appropriate active shader profile settings based on any
 * profile directives stored within the shader header
 */
void ShaderCompilerCg::
get_profile_from_header(const std::string &shader_text, Shader::ShaderCaps &caps) const {
  // Note this forces profile based on what is specified in the shader header
  // string.  Should probably be relying on card caps eventually.
#ifdef HAVE_CG
  size_t last_profile_pos = 0;
  while ((last_profile_pos = shader_text.find("//Cg profile", last_profile_pos)) != std::string::npos) {
    size_t newline_pos = shader_text.find('\n', last_profile_pos);
    std::string search_str = shader_text.substr(last_profile_pos, newline_pos);

    // Scan the line for known cg2 vertex program profiles
    for (const std::string &profile : vertex_profiles) {
      if (search_str.find(profile) != std::string::npos) {
        caps._active_vprofile = cgGetProfile(profile.c_str());
      }
    }

    // Scan the line for known cg2 fragment program profiles
    for (const std::string &profile : fragment_profiles) {
      if (search_str.find(profile) != std::string::npos) {
        caps._active_fprofile = cgGetProfile(profile.c_str());
      }
    }

    // Scan the line for known cg2 geometry program profiles
    for (const std::string &profile : geometry_profiles) {
      if (search_str.find(profile) != std::string::npos) {
        caps._active_gprofile = cgGetProfile(profile.c_str());
      }
    }

    last_profile_pos = newline_pos;
  }
#endif // HAVE_CG
}

/**
 *
 */
std::string ShaderCompilerCg::
get_name() const {
  return "Cg Compiler";
}

/**
 *
 */
ShaderLanguages ShaderCompilerCg::
get_languages() const {
  return {
    Shader::SL_Cg
  };
}

PT(ShaderModule) ShaderCompilerCg::
compile_now(ShaderModule::Stage stage, std::istream &in) const {
  return nullptr;
}
