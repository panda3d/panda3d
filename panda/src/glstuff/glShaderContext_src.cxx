// Filename: glShaderContext_src.cxx
// Created by: jyelon (01Sep05)
// Updated by: fperazzi, PandaSE (29Apr10) (updated CLP with note that some
//   parameter types only supported under Cg)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef OPENGLES_1

#include "pStatTimer.h"

TypeHandle CLP(ShaderContext)::_type_handle;

#ifndef GL_GEOMETRY_SHADER_EXT
#define GL_GEOMETRY_SHADER_EXT 0x8DD9
#endif
#ifndef GL_GEOMETRY_VERTICES_OUT_EXT
#define GL_GEOMETRY_VERTICES_OUT_EXT 0x8DDA
#endif
#ifndef GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT 0x8DE0
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::ParseAndSetShaderUniformVars
//       Access: Public
//  Description: The Panda CG shader syntax defines a useful set of shorthand notations for setting nodepath
//               properties as shaderinputs. For example, float4 mspos_XXX refers to nodepath XXX's position
//               in model space. This function is a rough attempt to reimplement some of the shorthand 
//               notations for GLSL. The code is ~99% composed of excerpts dealing with matrix shaderinputs
//               from Shader::compile_parameter.  
//               
//               Given a uniform variable name queried from the compiled shader passed in via arg_id, 
//                  1) parse the name
//                  2a) if the name refers to a Panda shorthand notation
//                        push the appropriate matrix into shader._mat_spec
//                        returns True
//                  2b) If the name doesn't refer to a Panda shorthand notation
//                        returns False
//               
//               The boolean return is used to notify down-river processing whether the shader var/parm was 
//               actually picked up and the appropriate ShaderMatSpec pushed onto _mat_spec.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
parse_and_set_short_hand_shader_vars(Shader::ShaderArgId &arg_id, Shader *objShader) {
  Shader::ShaderArgInfo p;
  p._id = arg_id;
  p._cat = GLCAT;

  string basename(arg_id._name);
  // Split it at the underscores.
  vector_string pieces;
  tokenize(basename, pieces, "_");

  if (pieces[0] == "mstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("model");
  }
  if (pieces[0] == "wstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("world");
  }
  if (pieces[0] == "vstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("view");
  }
  if (pieces[0] == "cstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("clip");
  }
  if (pieces[0] == "mspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("model");
  }
  if (pieces[0] == "wspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("world");
  }
  if (pieces[0] == "vspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("view");
  }
  if (pieces[0] == "cspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("clip");
  }

  if ((pieces[0] == "mat") || (pieces[0] == "inv") ||
      (pieces[0] == "tps") || (pieces[0] == "itp")) {
    if (!objShader->cp_errchk_parameter_words(p, 2)) {
      return false;
    }
    string trans = pieces[0];
    string matrix = pieces[1];
    pieces.clear();
    if (matrix == "modelview") {
      tokenize("trans_model_to_apiview", pieces, "_");
    } else if (matrix == "projection") {
      tokenize("trans_apiview_to_apiclip", pieces, "_");
    } else if (matrix == "modelproj") {
      tokenize("trans_model_to_apiclip", pieces, "_");
    } else {
      objShader->cp_report_error(p,"unrecognized matrix name");
      return false;
    }
    if (trans == "mat") {
      pieces[0] = "trans";
    } else if (trans == "inv") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
    } else if (trans == "tps") {
      pieces[0] = "tpose";
    } else if (trans == "itp") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
      pieces[0] = "tpose";
    }
  }

  // Implement the transform-matrix generator.
  if ((pieces[0] == "trans") ||
      (pieces[0] == "tpose") ||
      (pieces[0] == "row0") ||
      (pieces[0] == "row1") ||
      (pieces[0] == "row2") ||
      (pieces[0] == "row3") ||
      (pieces[0] == "col0") ||
      (pieces[0] == "col1") ||
      (pieces[0] == "col2") ||
      (pieces[0] == "col3")) {

    Shader::ShaderMatSpec bind;
    bind._id = arg_id;
    bind._func = Shader::SMF_compose;

    int next = 1;
    pieces.push_back("");

    // Decide whether this is a matrix or vector.
    if      (pieces[0] == "trans") bind._piece = Shader::SMP_whole;
    else if (pieces[0] == "tpose") bind._piece = Shader::SMP_transpose;
    else if (pieces[0] == "row0")  bind._piece = Shader::SMP_row0;
    else if (pieces[0] == "row1")  bind._piece = Shader::SMP_row1;
    else if (pieces[0] == "row2")  bind._piece = Shader::SMP_row2;
    else if (pieces[0] == "row3")  bind._piece = Shader::SMP_row3;
    else if (pieces[0] == "col0")  bind._piece = Shader::SMP_col0;
    else if (pieces[0] == "col1")  bind._piece = Shader::SMP_col1;
    else if (pieces[0] == "col2")  bind._piece = Shader::SMP_col2;
    else if (pieces[0] == "col3")  bind._piece = Shader::SMP_col3;

    if (!objShader->cp_parse_coord_sys(p, pieces, next, bind, true)) {
      return false;
    }
    if (!objShader->cp_parse_delimiter(p, pieces, next)) {
      return false;
    }
    if (!objShader->cp_parse_coord_sys(p, pieces, next, bind, false)) {
      return false;
    }
    if (!objShader->cp_parse_eol(p, pieces, next)) {
      return false;
    }
    objShader->cp_optimize_mat_spec(bind);
    objShader->_mat_spec.push_back(bind);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s) : ShaderContext(s) {
  _glgsg = glgsg;
  _glsl_program = 0;
  _glsl_vshader = 0;
  _glsl_fshader = 0;
  _glsl_gshader = 0;
  _glsl_tcshader = 0;
  _glsl_teshader = 0;
  _glsl_cshader = 0;
  _uses_standard_vertex_arrays = false;

  nassertv(s->get_language() == Shader::SL_GLSL);

  // We compile and analyze the shader here, instead of in shader.cxx, to avoid gobj getting a dependency on GL stuff.
  if (!glsl_compile_shader()) {
    release_resources();
    s->_error_flag = true;
    return;
  }
  _glgsg->_glUseProgram(_glsl_program);

  // Analyze the uniforms and put them in _glsl_parameter_map
  if (_glsl_parameter_map.size() == 0) {
    int seqno = 0, texunitno = 0, imgunitno = 0;
    string noprefix;
    GLint param_count, param_maxlength, param_size;
    GLenum param_type;
    _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORMS, &param_count);
    _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &param_maxlength);
    char* param_name_cstr = (char *)alloca(param_maxlength);

    for (int i = 0; i < param_count; ++i) {
      param_name_cstr[0] = 0;
      _glgsg->_glGetActiveUniform(_glsl_program, i, param_maxlength, NULL, &param_size, &param_type, param_name_cstr);
      string param_name(param_name_cstr);
      GLint p = _glgsg->_glGetUniformLocation(_glsl_program, param_name_cstr);

      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Active uniform " << param_name << " with size " << param_size
          << " and type " << param_type << " is bound to location " << p << "\n";
      }

      if (p > -1) {
        // Strip off [0] suffix that some drivers append to arrays.
        size_t size = param_name.size();
        if (size > 3 && param_name.substr(size - 3, 3) == "[0]") {
          param_name = param_name.substr(0, size - 3);
        }

        Shader::ShaderArgId arg_id;
        arg_id._name  = param_name;
        arg_id._seqno = seqno++;
        _glsl_parameter_map.push_back(p);

        // Check for inputs with p3d_ prefix.
        if (param_name.substr(0, 4) == "p3d_") {
          noprefix = param_name.substr(4);

          // Check for matrix inputs.
          bool transpose = false;
          bool inverse = false;
          string matrix_name (noprefix);

          // Check for and chop off any "Transpose" or "Inverse" suffix.
          if (size > 15 && matrix_name.compare(size - 9, 9, "Transpose") == 0) {
            transpose = true;
            matrix_name = matrix_name.substr(0, size - 9);
          }
          size = matrix_name.size();
          if (size > 13 && matrix_name.compare(size - 7, 7, "Inverse") == 0) {
            inverse = true;
            matrix_name = matrix_name.substr(0, size - 7);
          }
          size = matrix_name.size();

          // Now if the suffix that is left over is "Matrix",
          // we know that it is supposed to be a matrix input.
          if (size > 6 && matrix_name.compare(size - 6, 6, "Matrix") == 0) {
            Shader::ShaderMatSpec bind;
            bind._id = arg_id;
            if (transpose) {
              bind._piece = Shader::SMP_transpose;
            } else {
              bind._piece = Shader::SMP_whole;
            }
            bind._arg[0] = NULL;
            bind._arg[1] = NULL;

            if (matrix_name == "ModelViewProjectionMatrix") {
              bind._func = Shader::SMF_compose;
              if (inverse) {
                bind._part[0] = Shader::SMO_apiclip_to_view;
                bind._part[1] = Shader::SMO_view_to_model;
              } else {
                bind._part[0] = Shader::SMO_model_to_view;
                bind._part[1] = Shader::SMO_view_to_apiclip;
              }
              bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
              bind._dep[1] = Shader::SSD_general | Shader::SSD_transform;

            } else if (matrix_name == "ModelViewMatrix") {
              bind._func = Shader::SMF_compose;
              if (inverse) {
                bind._part[0] = Shader::SMO_apiview_to_view;
                bind._part[1] = Shader::SMO_view_to_model;
              } else {
                bind._part[0] = Shader::SMO_model_to_view;
                bind._part[1] = Shader::SMO_view_to_apiview;
              }
              bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
              bind._dep[1] = Shader::SSD_general | Shader::SSD_transform;

            } else if (matrix_name == "ProjectionMatrix") {
              bind._func = Shader::SMF_compose;
              if (inverse) {
                bind._part[0] = Shader::SMO_apiclip_to_view;
                bind._part[1] = Shader::SMO_view_to_apiview;
              } else {
                bind._part[0] = Shader::SMO_apiview_to_view;
                bind._part[1] = Shader::SMO_view_to_apiclip;
              }
              bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
              bind._dep[1] = Shader::SSD_general | Shader::SSD_transform;

            } else if (matrix_name == "NormalMatrix") {
              // This is really the upper 3x3 of the ModelViewMatrixInverseTranspose.
              bind._func = Shader::SMF_compose;
              if (inverse) {
                bind._part[0] = Shader::SMO_model_to_view;
                bind._part[1] = Shader::SMO_view_to_apiview;
              } else {
                bind._part[0] = Shader::SMO_apiview_to_view;
                bind._part[1] = Shader::SMO_view_to_model;
              }
              bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
              bind._dep[1] = Shader::SSD_general | Shader::SSD_transform;

              if (transpose) {
                bind._piece = Shader::SMP_upper3x3;
              } else {
                bind._piece = Shader::SMP_transpose3x3;
              }

            } else {
              GLCAT.error() << "Unrecognized uniform matrix name '" << matrix_name << "'!\n";
              continue;
            }
            s->_mat_spec.push_back(bind);
            continue;
          }
          if (size > 7 && noprefix.substr(0, 7) == "Texture") {
            Shader::ShaderTexSpec bind;
            bind._id = arg_id;
            bind._name = 0;
            bind._desired_type = Texture::TT_2d_texture;
            bind._stage = atoi(noprefix.substr(7).c_str());
            s->_tex_spec.push_back(bind);
            continue;
          }
          if (size > 9 && noprefix.substr(0, 9) == "Material.") {
            Shader::ShaderMatSpec bind;
            bind._id = arg_id;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_attr_material;
            bind._arg[0] = NULL;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_material;
            bind._part[1] = Shader::SMO_identity;
            bind._arg[1] = NULL;
            bind._dep[1] = Shader::SSD_NONE;

            if (noprefix == "Material.ambient") {
              bind._piece = Shader::SMP_row0;
              s->_mat_spec.push_back(bind);
              continue;
            } else if (noprefix == "Material.diffuse") {
              bind._piece = Shader::SMP_row1;
              s->_mat_spec.push_back(bind);
              continue;
            } else if (noprefix == "Material.emission") {
              bind._piece = Shader::SMP_row2;
              s->_mat_spec.push_back(bind);
              continue;
            } else if (noprefix == "Material.specular") {
              bind._piece = Shader::SMP_row3;
              s->_mat_spec.push_back(bind);
              continue;
            }
          }
          if (noprefix == "ColorScale") {
            Shader::ShaderMatSpec bind;
            bind._id = arg_id;
            bind._piece = Shader::SMP_row3;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_attr_colorscale;
            bind._arg[0] = NULL;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_colorscale;
            bind._part[1] = Shader::SMO_identity;
            bind._arg[1] = NULL;
            bind._dep[1] = Shader::SSD_NONE;
            s->_mat_spec.push_back(bind);
            continue;
          }
          GLCAT.error() << "Unrecognized uniform name '" << param_name_cstr << "'!\n";
          continue;

        } else if (param_name.substr(0, 4) == "osg_") {
          // These inputs are supported by OpenSceneGraph.  We can support
          // them as well, to increase compatibility.
          // Other inputs we may support in the future:
          // int osg_FrameNumber

          Shader::ShaderMatSpec bind;
          bind._id = arg_id;
          bind._arg[0] = NULL;
          bind._arg[1] = NULL;

          if (param_name == "osg_ViewMatrix") {
            bind._piece = Shader::SMP_whole;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_world_to_view;
            bind._part[1] = Shader::SMO_identity;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
            bind._dep[1] = Shader::SSD_NONE;
            s->_mat_spec.push_back(bind);
            continue;

          } else if (param_name == "osg_InverseViewMatrix") {
            bind._piece = Shader::SMP_whole;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_view_to_world;
            bind._part[1] = Shader::SMO_identity;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
            bind._dep[1] = Shader::SSD_NONE;
            s->_mat_spec.push_back(bind);
            continue;

          } else if (param_name == "osg_FrameTime") {
            bind._piece = Shader::SMP_row3x1;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_frame_time;
            bind._part[1] = Shader::SMO_identity;
            bind._dep[0] = Shader::SSD_general;
            bind._dep[1] = Shader::SSD_NONE;
            s->_mat_spec.push_back(bind);
            continue;

          } else if (param_name == "osg_DeltaFrameTime") {
            bind._piece = Shader::SMP_row3x1;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_frame_delta;
            bind._part[1] = Shader::SMO_identity;
            bind._dep[0] = Shader::SSD_general;
            bind._dep[1] = Shader::SSD_NONE;
            s->_mat_spec.push_back(bind);
            continue;
          }
        }

        //Tries to parse shorthand notations like mspos_XXX and trans_model_to_clip_of_XXX
        if (parse_and_set_short_hand_shader_vars(arg_id, s)) {
          continue;
        }

        if (param_size == 1) {
          switch (param_type) {
#ifndef OPENGLES
            case GL_SAMPLER_1D_SHADOW:
            case GL_SAMPLER_1D: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = InternalName::make(param_name);
              bind._desired_type = Texture::TT_1d_texture;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
            case GL_SAMPLER_2D_SHADOW:
#endif
            case GL_SAMPLER_2D: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = InternalName::make(param_name);
              bind._desired_type = Texture::TT_2d_texture;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
            case GL_SAMPLER_3D: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = InternalName::make(param_name);
              bind._desired_type = Texture::TT_3d_texture;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
            case GL_SAMPLER_CUBE: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = InternalName::make(param_name);
              bind._desired_type = Texture::TT_cube_map;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
#ifndef OPENGLES
            case GL_SAMPLER_2D_ARRAY: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = InternalName::make(param_name);
              bind._desired_type = Texture::TT_2d_texture_array;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
#endif
            case GL_FLOAT_MAT2:
#ifndef OPENGLES
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
#endif
              GLCAT.warning() << "GLSL shader requested an unrecognized matrix type\n";
              continue;
            case GL_FLOAT_MAT3: {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              bind._piece = Shader::SMP_upper3x3;
              bind._func = Shader::SMF_first;
              bind._part[0] = Shader::SMO_mat_constant_x;
              bind._arg[0] = InternalName::make(param_name);
              bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs;
              bind._part[1] = Shader::SMO_identity;
              bind._arg[1] = NULL;
              bind._dep[1] = Shader::SSD_NONE;
              s->_mat_spec.push_back(bind);
              continue; }
            case GL_FLOAT_MAT4: {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              bind._piece = Shader::SMP_whole;
              bind._func = Shader::SMF_first;
              bind._part[0] = Shader::SMO_mat_constant_x;
              bind._arg[0] = InternalName::make(param_name);
              bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs;
              bind._part[1] = Shader::SMO_identity;
              bind._arg[1] = NULL;
              bind._dep[1] = Shader::SSD_NONE;
              s->_mat_spec.push_back(bind);
              continue; }
            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4: {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              switch (param_type) {
              case GL_BOOL:
              case GL_FLOAT:      bind._piece = Shader::SMP_row3x1; break;
              case GL_BOOL_VEC2:
              case GL_FLOAT_VEC2: bind._piece = Shader::SMP_row3x2; break;
              case GL_BOOL_VEC3:
              case GL_FLOAT_VEC3: bind._piece = Shader::SMP_row3x3; break;
              case GL_BOOL_VEC4:
              case GL_FLOAT_VEC4: bind._piece = Shader::SMP_row3  ; break;
              }
              bind._func = Shader::SMF_first;
              bind._part[0] = Shader::SMO_vec_constant_x;
              bind._arg[0] = InternalName::make(param_name);
              bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs;
              bind._part[1] = Shader::SMO_identity;
              bind._arg[1] = NULL;
              bind._dep[1] = Shader::SSD_NONE;
              s->_mat_spec.push_back(bind);
              continue; }
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
              GLCAT.warning() << "Panda does not support passing integers to shaders (yet)!\n";
              continue;
#ifndef OPENGLES
            case GL_IMAGE_1D_EXT:
            case GL_IMAGE_2D_EXT:
            case GL_IMAGE_3D_EXT:
            case GL_IMAGE_CUBE_EXT:
            case GL_IMAGE_2D_ARRAY_EXT:
            case GL_INT_IMAGE_1D_EXT:
            case GL_INT_IMAGE_2D_EXT:
            case GL_INT_IMAGE_3D_EXT:
            case GL_INT_IMAGE_CUBE_EXT:
            case GL_INT_IMAGE_2D_ARRAY_EXT:
            case GL_UNSIGNED_INT_IMAGE_1D_EXT:
            case GL_UNSIGNED_INT_IMAGE_2D_EXT:
            case GL_UNSIGNED_INT_IMAGE_3D_EXT:
            case GL_UNSIGNED_INT_IMAGE_CUBE_EXT:
            case GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT:
              // This won't really change at runtime, so we might as well
              // bind once and then forget about it.
              _glgsg->_glUniform1i(p, imgunitno++);
              _glsl_img_inputs.push_back(InternalName::make(param_name));
              continue;
#endif
            default:
              GLCAT.warning() << "Ignoring unrecognized GLSL parameter type!\n";
          }
        } else {
          switch (param_type) {
          case GL_FLOAT_MAT2:
#ifndef OPENGLES
          case GL_FLOAT_MAT2x3:
          case GL_FLOAT_MAT2x4:
          case GL_FLOAT_MAT3x2:
          case GL_FLOAT_MAT3x4:
          case GL_FLOAT_MAT4x2:
          case GL_FLOAT_MAT4x3:
#endif
            GLCAT.warning() << "GLSL shader requested an unrecognized matrix array type\n";
            continue;
          case GL_BOOL:
          case GL_BOOL_VEC2:
          case GL_BOOL_VEC3:
          case GL_BOOL_VEC4:
          case GL_FLOAT:
          case GL_FLOAT_VEC2:
          case GL_FLOAT_VEC3:
          case GL_FLOAT_VEC4:
          case GL_FLOAT_MAT3:
          case GL_FLOAT_MAT4: {
            Shader::ShaderPtrSpec bind;
            bind._id = arg_id;
            switch (param_type) {
              case GL_BOOL:
              case GL_FLOAT:      bind._dim[1] = 1; break;
              case GL_BOOL_VEC2:
              case GL_FLOAT_VEC2: bind._dim[1] = 2; break;
              case GL_BOOL_VEC3:
              case GL_FLOAT_VEC3: bind._dim[1] = 3; break;
              case GL_BOOL_VEC4:
              case GL_FLOAT_VEC4: bind._dim[1] = 4; break;
              case GL_FLOAT_MAT3: bind._dim[1] = 9; break;
              case GL_FLOAT_MAT4: bind._dim[1] = 16; break;
            }
            bind._arg = InternalName::make(param_name);
            bind._dim[0] = param_size;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs;
            bind._dep[1] = Shader::SSD_NONE;
            s->_ptr_spec.push_back(bind);
            continue; }
          case GL_INT:
          case GL_INT_VEC2:
          case GL_INT_VEC3:
          case GL_INT_VEC4:
            GLCAT.warning() << "Panda does not support passing integer arrays to shaders (yet)!\n";
            continue;
          default:
            GLCAT.warning() << "Ignoring unrecognized GLSL parameter array type!\n";
          }
        }
      }
    }
    
    // Now we've processed the uniforms, we'll process the attribs.
    _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTES, &param_count);
    _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &param_maxlength);
    param_name_cstr = (char *)alloca(param_maxlength);

    for (int i = 0; i < param_count; ++i) {
      param_name_cstr[0] = 0;
      _glgsg->_glGetActiveAttrib(_glsl_program, i, param_maxlength, NULL, &param_size, &param_type, param_name_cstr);
      string param_name(param_name_cstr);

      // Get the attrib location.
      GLint p = _glgsg->_glGetAttribLocation(_glsl_program, param_name_cstr);

      GLCAT.debug() <<
        "Active attribute " << param_name << " is bound to location " << p << "\n";

      if (p == -1) {
        // A gl_ attribute such as gl_Vertex requires us to pass the
        // standard vertex arrays as we would do without shader.
        // This is not always the case, though -- see below.
        _uses_standard_vertex_arrays = true;
        continue;
      }

      Shader::ShaderArgId arg_id;
      arg_id._name  = param_name;
      arg_id._seqno = seqno++;
      _glsl_parameter_map.push_back(p);

      Shader::ShaderVarSpec bind;
      bind._id = arg_id;
      bind._name = NULL;
      bind._append_uv = -1;

      if (param_name.substr(0, 3) == "gl_") {
        // Not all drivers return -1 in glGetAttribLocation
        // for gl_ prefixed attributes.
        _uses_standard_vertex_arrays = true;
        continue;
      } else if (param_name.substr(0, 4) == "p3d_") {
        noprefix = param_name.substr(4);
      } else {
        noprefix = "";
      }

      if (noprefix.empty()) {
        // Arbitrarily named attribute.
        bind._name = InternalName::make(param_name);

      } else if (noprefix == "Vertex") {
        bind._name = InternalName::get_vertex();

      } else if (noprefix == "Normal") {
        bind._name = InternalName::get_normal();

      } else if (noprefix == "Color") {
        bind._name = InternalName::get_color();

      } else if (noprefix.substr(0, 7) == "Tangent") {
        bind._name = InternalName::get_tangent();
        if (noprefix.size() > 7) {
          bind._append_uv = atoi(noprefix.substr(7).c_str());
        }

      } else if (noprefix.substr(0, 8) == "Binormal") {
        bind._name = InternalName::get_binormal();
        if (noprefix.size() > 8) {
          bind._append_uv = atoi(noprefix.substr(8).c_str());
        }

      } else if (noprefix.substr(0, 13) == "MultiTexCoord") {
        bind._name = InternalName::get_texcoord();
        bind._append_uv = atoi(noprefix.substr(13).c_str());

      } else {
        GLCAT.error() << "Unrecognized vertex attrib '" << param_name << "'!\n";
        continue;
      }
      s->_var_spec.push_back(bind);
    }
  }
  _glgsg->_glUseProgram(0);

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  release_resources();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::release_resources
//       Access: Public
//  Description: Should deallocate all system resources (such as
//               vertex program handles or Cg contexts).
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
release_resources() {
  if (!_glgsg) {
    return;
  }
  if (_glsl_program != 0) {
    if (_glsl_vshader != 0) {
      _glgsg->_glDetachShader(_glsl_program, _glsl_vshader);
    }
    if (_glsl_fshader != 0) {
      _glgsg->_glDetachShader(_glsl_program, _glsl_fshader);
    }
    if (_glsl_gshader != 0) {
      _glgsg->_glDetachShader(_glsl_program, _glsl_gshader);
    }
    if (_glsl_tcshader != 0) {
      _glgsg->_glDetachShader(_glsl_program, _glsl_tcshader);
    }
    if (_glsl_teshader != 0) {
      _glgsg->_glDetachShader(_glsl_program, _glsl_teshader);
    }
    if (_glsl_cshader != 0) {
      _glgsg->_glDetachShader(_glsl_program, _glsl_cshader);
    }
    _glgsg->_glDeleteProgram(_glsl_program);
    _glsl_program = 0;
  }
  if (_glsl_vshader != 0) {
    _glgsg->_glDeleteShader(_glsl_vshader);
    _glsl_vshader = 0;
  }
  if (_glsl_fshader != 0) {
    _glgsg->_glDeleteShader(_glsl_fshader);
    _glsl_fshader = 0;
  }
  if (_glsl_gshader != 0) {
    _glgsg->_glDeleteShader(_glsl_gshader);
    _glsl_gshader = 0;
  }
  if (_glsl_tcshader != 0) {
    _glgsg->_glDeleteShader(_glsl_tcshader);
    _glsl_tcshader = 0;
  }
  if (_glsl_teshader != 0) {
    _glgsg->_glDeleteShader(_glsl_teshader);
    _glsl_teshader = 0;
  }
  if (_glsl_cshader != 0) {
    _glgsg->_glDeleteShader(_glsl_cshader);
    _glsl_cshader = 0;
  }
  
  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::bind
//       Access: Public
//  Description: This function is to be called to enable a new
//               shader.  It also initializes all of the shader's
//               input parameters.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind(bool reissue_parameters) {
  // GLSL shaders need to be bound before passing parameters.
  if (!_shader->get_error_flag()) {
    _glgsg->_glUseProgram(_glsl_program);
  }

  if (reissue_parameters) {
    // Pass in k-parameters and transform-parameters
    issue_parameters(Shader::SSD_general);
  }

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind() {
  _glgsg->_glUseProgram(0);
  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::issue_parameters
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               or TransformState has changed, but the Shader
//               itself has not changed.  It loads new values into the
//               shader's parameters.
//
//               If "altered" is false, that means you promise that
//               the parameters for this shader context have already
//               been issued once, and that since the last time the
//               parameters were issued, no part of the render
//               state has changed except the external and internal
//               transforms.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
issue_parameters(int altered) {
  PStatTimer timer(_glgsg->_draw_set_state_shader_parameters_pcollector);

  if (!valid()) {
    return;
  }

  // Iterate through _ptr parameters
  for (int i=0; i<(int)_shader->_ptr_spec.size(); i++) {
    if(altered & (_shader->_ptr_spec[i]._dep[0] | _shader->_ptr_spec[i]._dep[1])) {
      const Shader::ShaderPtrSpec& _ptr = _shader->_ptr_spec[i];
      Shader::ShaderPtrData* _ptr_data =
        const_cast< Shader::ShaderPtrData*>(_glgsg->fetch_ptr_parameter(_ptr));
      if (_ptr_data == NULL) { //the input is not contained in ShaderPtrData
        release_resources();
        return;
      }
      GLint p = _glsl_parameter_map[_shader->_ptr_spec[i]._id._seqno];
      switch (_ptr._dim[1]) {
        case 1: _glgsg->_glUniform1fv(p, _ptr._dim[0], (float*)_ptr_data->_ptr); continue;
        case 2: _glgsg->_glUniform2fv(p, _ptr._dim[0], (float*)_ptr_data->_ptr); continue;
        case 3: _glgsg->_glUniform3fv(p, _ptr._dim[0], (float*)_ptr_data->_ptr); continue;
        case 4: _glgsg->_glUniform4fv(p, _ptr._dim[0], (float*)_ptr_data->_ptr); continue;
        case 9: _glgsg->_glUniformMatrix3fv(p, _ptr._dim[0], GL_FALSE, (float*)_ptr_data->_ptr); continue;
        case 16: _glgsg->_glUniformMatrix4fv(p, _ptr._dim[0], GL_FALSE, (float*)_ptr_data->_ptr); continue;
      }
    }
  }

  for (int i=0; i<(int)_shader->_mat_spec.size(); i++) {
    if (altered & (_shader->_mat_spec[i]._dep[0] | _shader->_mat_spec[i]._dep[1])) {
      const LMatrix4 *val = _glgsg->fetch_specified_value(_shader->_mat_spec[i], altered);
      if (!val) continue;
#ifndef STDFLOAT_DOUBLE
      // In this case, the data is already single-precision.
      const PN_float32 *data = val->get_data();
#else
      // In this case, we have to convert it.
      LMatrix4f valf = LCAST(PN_float32, *val);
      const PN_float32 *data = valf.get_data();
#endif

      GLint p = _glsl_parameter_map[_shader->_mat_spec[i]._id._seqno];
      switch (_shader->_mat_spec[i]._piece) {
      case Shader::SMP_whole: _glgsg->_glUniformMatrix4fv(p, 1, GL_FALSE, data); continue;
      case Shader::SMP_transpose: _glgsg->_glUniformMatrix4fv(p, 1, GL_TRUE, data); continue;
      case Shader::SMP_col0: _glgsg->_glUniform4f(p, data[0], data[4], data[ 8], data[12]); continue;
      case Shader::SMP_col1: _glgsg->_glUniform4f(p, data[1], data[5], data[ 9], data[13]); continue;
      case Shader::SMP_col2: _glgsg->_glUniform4f(p, data[2], data[6], data[10], data[14]); continue;
      case Shader::SMP_col3: _glgsg->_glUniform4f(p, data[3], data[7], data[11], data[15]); continue;
      case Shader::SMP_row0: _glgsg->_glUniform4fv(p, 1, data+ 0); continue;
      case Shader::SMP_row1: _glgsg->_glUniform4fv(p, 1, data+ 4); continue;
      case Shader::SMP_row2: _glgsg->_glUniform4fv(p, 1, data+ 8); continue;
      case Shader::SMP_row3: _glgsg->_glUniform4fv(p, 1, data+12); continue;
      case Shader::SMP_row3x1: _glgsg->_glUniform1fv(p, 1, data+12); continue;
      case Shader::SMP_row3x2: _glgsg->_glUniform2fv(p, 1, data+12); continue;
      case Shader::SMP_row3x3: _glgsg->_glUniform3fv(p, 1, data+12); continue;
      case Shader::SMP_upper3x3:
        {
          LMatrix3f upper3 = val->get_upper_3();
          _glgsg->_glUniformMatrix3fv(p, 1, false, upper3.get_data());
          continue;
        }
      case Shader::SMP_transpose3x3:
        {
          LMatrix3f upper3 = val->get_upper_3();
          _glgsg->_glUniformMatrix3fv(p, 1, true, upper3.get_data());
          continue;
        }
      }
    }
  }

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays() {
  if (!valid()) {
    return;
  }

  for (int i=0; i<(int)_shader->_var_spec.size(); i++) {
    _glgsg->_glDisableVertexAttribArray(i);
  }

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::update_shader_vertex_arrays
//       Access: Public
//  Description: Disables all vertex arrays used by the previous
//               shader, then enables all the vertex arrays needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable arrays then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
update_shader_vertex_arrays(ShaderContext *prev, bool force) {
  if (prev) prev->disable_shader_vertex_arrays();
  if (!valid()) {
    return true;
  }

#ifdef SUPPORT_IMMEDIATE_MODE
  if (_glgsg->_use_sender) {
    GLCAT.error() << "immediate mode shaders not implemented yet\n";
  } else
#endif // SUPPORT_IMMEDIATE_MODE
  {
    const GeomVertexArrayDataHandle *array_reader;
    Geom::NumericType numeric_type;
    int start, stride, num_values;
    int nvarying = _shader->_var_spec.size();
    for (int i = 0; i < nvarying; ++i) {
      InternalName *name = _shader->_var_spec[i]._name;
      int texslot = _shader->_var_spec[i]._append_uv;
      if (texslot >= 0 && texslot < _glgsg->_state_texture->get_num_on_stages()) {
        TextureStage *stage = _glgsg->_state_texture->get_on_stage(texslot);
        InternalName *texname = stage->get_texcoord_name();

        if (name == InternalName::get_texcoord()) {
          name = texname;
        } else if (texname != InternalName::get_texcoord()) {
          name = name->append(texname->get_basename());
        }
      }
      if (_glgsg->_data_reader->get_array_info(name,
                                            array_reader, num_values, numeric_type,
                                            start, stride)) {
        const unsigned char *client_pointer;
        if (!_glgsg->setup_array_data(client_pointer, array_reader, force)) {
          return false;
        }

        const GLint p = _glsl_parameter_map[_shader->_var_spec[i]._id._seqno];
        _glgsg->_glEnableVertexAttribArray(p);
        _glgsg->_glVertexAttribPointer(p, num_values, _glgsg->get_numeric_type(numeric_type),
                                    GL_TRUE, stride, client_pointer + start);
      }
    }
  }

  _glgsg->report_my_gl_errors();
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_texture_bindings
//       Access: Public
//  Description: Disable all the texture bindings used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_texture_bindings() {
  if (!valid()) {
    return;
  }

#ifndef OPENGLES_2
  for (int i=0; i<(int)_shader->_tex_spec.size(); i++) {
    if (_shader->_tex_spec[i]._name == 0) {
      _glgsg->_glActiveTexture(GL_TEXTURE0 + _shader->_tex_spec[i]._stage);
    } else {
      _glgsg->_glActiveTexture(GL_TEXTURE0 + _shader->_tex_spec[i]._stage + _stage_offset);
    }

#ifndef OPENGLES
    glBindTexture(GL_TEXTURE_1D, 0);
#endif  // OPENGLES
    glBindTexture(GL_TEXTURE_2D, 0);
#ifndef OPENGLES_1
    if (_glgsg->_supports_3d_texture) {
      glBindTexture(GL_TEXTURE_3D, 0);
    }
#endif  // OPENGLES_1
#ifndef OPENGLES
    if (_glgsg->_supports_2d_texture_array) {
      glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
    }
#endif
    if (_glgsg->_supports_cube_map) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    // This is probably faster - but maybe not as safe?
    // cgGLDisableTextureParameter(p);
  }
#endif  // OPENGLES_2
  _stage_offset = 0;

#ifndef OPENGLES
  // Now unbind all the image units.  Not sure if we *have* to do this.
  int num_image_units = min(_glsl_img_inputs.size(), (size_t)_glgsg->_max_image_units);

  if (_glgsg->_supports_multi_bind) {
    _glgsg->_glBindImageTextures(0, num_image_units, NULL);

  } else {
    for (int i = 0; i < num_image_units; ++i) {
      _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
    }
  }
#endif

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::update_shader_texture_bindings
//       Access: Public
//  Description: Disables all texture bindings used by the previous
//               shader, then enables all the texture bindings needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable textures then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
update_shader_texture_bindings(ShaderContext *prev) {
  if (prev) {
    prev->disable_shader_texture_bindings();
  }

  if (!valid()) {
    return;
  }

#ifndef OPENGLES
  // First bind all the 'image units'; a bit of an esoteric OpenGL feature right now.
  int num_image_units = min(_glsl_img_inputs.size(), (size_t)_glgsg->_max_image_units);

  if (num_image_units > 0) {
    GLuint *multi_img = NULL;
    // If we support multi-bind, prepare an array.
    if (_glgsg->_supports_multi_bind) {
      multi_img = new GLuint[num_image_units];
    }

    for (int i = 0; i < num_image_units; ++i) {
      const InternalName *name = _glsl_img_inputs[i];
      Texture *tex = _glgsg->_target_shader->get_shader_input_texture(name);

      GLuint gl_tex = 0;
      if (tex != NULL) {
        int view = _glgsg->get_current_tex_view_offset();

        CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tex->prepare_now(view, _glgsg->_prepared_objects, _glgsg));
        if (gtc != (TextureContext*)NULL) {
          gl_tex = gtc->_index;
          _glgsg->update_texture(gtc, true);
        }
      }

      if (multi_img != NULL) {
        // Put in array so we can multi-bind later.
        multi_img[i] = gl_tex;

      } else {
        // We don't support multi-bind, so bind now in the same way that multi-bind would have done it.
        if (gl_tex == 0) {
          _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
        } else {
          //TODO: automatically convert to sized type instead of plain GL_RGBA
          // If a base type is used, it will crash.
          GLint internal_format = _glgsg->get_internal_image_format(tex);
          _glgsg->_glBindImageTexture(i, gl_tex, 0, GL_TRUE, 0, GL_READ_WRITE, internal_format);
        }
      }
    }

    if (multi_img != NULL) {
      _glgsg->_glBindImageTextures(0, num_image_units, multi_img);
      delete[] multi_img;
    }
  }
#endif

  // We get the TextureAttrib directly from the _target_rs, not the
  // filtered TextureAttrib in _target_texture.
  const TextureAttrib *texattrib = DCAST(TextureAttrib, _glgsg->_target_rs->get_attrib_def(TextureAttrib::get_class_slot()));
  nassertv(texattrib != (TextureAttrib *)NULL);
  _stage_offset = texattrib->get_num_on_stages();

  for (int i = 0; i < (int)_shader->_tex_spec.size(); ++i) {
    InternalName *id = _shader->_tex_spec[i]._name;
    int texunit = _shader->_tex_spec[i]._stage;
    if (id != 0) {
      texunit += _stage_offset;
    }

    Texture *tex = 0;
    int view = _glgsg->get_current_tex_view_offset();
    if (id != 0) {
      const ShaderInput *input = _glgsg->_target_shader->get_shader_input(id);
      tex = input->get_texture();
    } else {
      if (_shader->_tex_spec[i]._stage >= texattrib->get_num_on_stages()) {
        continue;
      }
      TextureStage *stage = texattrib->get_on_stage(_shader->_tex_spec[i]._stage);
      tex = texattrib->get_on_texture(stage);
      view += stage->get_tex_view_offset();
    }
    if (_shader->_tex_spec[i]._suffix != 0) {
      // The suffix feature is inefficient. It is a temporary hack.
      if (tex == 0) {
        continue;
      }
      tex = tex->load_related(_shader->_tex_spec[i]._suffix);
    }
    if ((tex == 0) || (tex->get_texture_type() != _shader->_tex_spec[i]._desired_type)) {
      continue;
    }

    _glgsg->_glActiveTexture(GL_TEXTURE0 + texunit);

    TextureContext *tc = tex->prepare_now(view, _glgsg->_prepared_objects, _glgsg);
    if (tc == (TextureContext*)NULL) {
      continue;
    }

    GLenum target = _glgsg->get_texture_target(tex->get_texture_type());
    if (target == GL_NONE) {
      // Unsupported texture mode.
      continue;
    }

    GLint p = _glsl_parameter_map[_shader->_tex_spec[i]._id._seqno];
    _glgsg->_glUniform1i(p, texunit);

    if (!_glgsg->update_texture(tc, false)) {
      continue;
    }
  }

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_shader_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_shader_errors(unsigned int shader) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

  if (length > 1) {
    info_log = (char *) malloc(length);
    _glgsg->_glGetShaderInfoLog(shader, length, &num_chars, info_log);
    if (strcmp(info_log, "Success.\n") != 0 && strcmp(info_log, "No errors.\n") != 0) {
      GLCAT.error(false) << info_log << "\n";
    }
    free(info_log);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_program_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a program.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_program_errors(unsigned int program) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

  if (length > 1) {
    info_log = (char *) malloc(length);
    _glgsg->_glGetProgramInfoLog(program, length, &num_chars, info_log);
    if (strcmp(info_log, "Success.\n") != 0 && strcmp(info_log, "No errors.\n") != 0) {
      GLCAT.error(false) << info_log << "\n";
    }
    free(info_log);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_compile_entry_point
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int CLP(ShaderContext)::
glsl_compile_entry_point(Shader::ShaderType type) {
  unsigned int handle = 0;
  switch (type) {
    case Shader::ST_vertex:
      handle = _glgsg->_glCreateShader(GL_VERTEX_SHADER);
      break;
    case Shader::ST_fragment:
      handle = _glgsg->_glCreateShader(GL_FRAGMENT_SHADER);
      break;
#ifndef OPENGLES
    case Shader::ST_geometry:
      if (_glgsg->get_supports_geometry_shaders()) {
        handle = _glgsg->_glCreateShader(GL_GEOMETRY_SHADER);
      }
      break;
    case Shader::ST_tess_control:
      if (_glgsg->get_supports_tessellation_shaders()) {
        handle = _glgsg->_glCreateShader(GL_TESS_CONTROL_SHADER);
      }
      break;
    case Shader::ST_tess_evaluation:
      if (_glgsg->get_supports_tessellation_shaders()) {
        handle = _glgsg->_glCreateShader(GL_TESS_EVALUATION_SHADER);
      }
      break;
    case Shader::ST_compute:
      if (_glgsg->get_supports_compute_shaders()) {
        handle = _glgsg->_glCreateShader(GL_COMPUTE_SHADER);
      }
      break;
#endif
  }
  if (!handle) {
    _glgsg->report_my_gl_errors();
    return 0;
  }

  string text_str = _shader->get_text(type);
  const char* text = text_str.c_str();
  _glgsg->_glShaderSource(handle, 1, &text, NULL);
  _glgsg->_glCompileShader(handle);
  GLint status;
  _glgsg->_glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
  
  if (status != GL_TRUE) {
    GLCAT.error() 
      << "An error occurred while compiling shader " 
      << _shader->get_filename(type) << "\n";
    glsl_report_shader_errors(handle);
    _glgsg->_glDeleteShader(handle);
    _glgsg->report_my_gl_errors();
    return 0;
  }

  _glgsg->report_my_gl_errors();
  return handle;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_compile_shader
//       Access: Private
//  Description: This subroutine compiles a GLSL shader.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
glsl_compile_shader() {
  _glsl_program = _glgsg->_glCreateProgram();
  if (!_glsl_program) return false;

  if (!_shader->get_text(Shader::ST_vertex).empty()) {
    _glsl_vshader = glsl_compile_entry_point(Shader::ST_vertex);
    if (!_glsl_vshader) return false;
    _glgsg->_glAttachShader(_glsl_program, _glsl_vshader);
  }

  if (!_shader->get_text(Shader::ST_fragment).empty()) {
    _glsl_fshader = glsl_compile_entry_point(Shader::ST_fragment);
    if (!_glsl_fshader) return false;
    _glgsg->_glAttachShader(_glsl_program, _glsl_fshader);
  }

  if (!_shader->get_text(Shader::ST_geometry).empty()) {
    _glsl_gshader = glsl_compile_entry_point(Shader::ST_geometry);
    if (!_glsl_gshader) return false;
    _glgsg->_glAttachShader(_glsl_program, _glsl_gshader);

#ifdef OPENGLES
    nassertr(false, false); // OpenGL ES has no geometry shaders.
#else
    // Set the vertex output limit to the maximum
    nassertr(_glgsg->_glProgramParameteri != NULL, false);
    GLint max_vertices;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_vertices);
    _glgsg->_glProgramParameteri(_glsl_program, GL_GEOMETRY_VERTICES_OUT_ARB, max_vertices); 
#endif
    _glgsg->report_my_gl_errors();
  }

  if (!_shader->get_text(Shader::ST_tess_control).empty()) {
    _glsl_tcshader = glsl_compile_entry_point(Shader::ST_tess_control);
    if (!_glsl_tcshader) return false;
    _glgsg->_glAttachShader(_glsl_program, _glsl_tcshader);
  }

  if (!_shader->get_text(Shader::ST_tess_evaluation).empty()) {
    _glsl_teshader = glsl_compile_entry_point(Shader::ST_tess_evaluation);
    if (!_glsl_teshader) return false;
    _glgsg->_glAttachShader(_glsl_program, _glsl_teshader);
  }

  if (!_shader->get_text(Shader::ST_compute).empty()) {
    _glsl_cshader = glsl_compile_entry_point(Shader::ST_compute);
    if (!_glsl_cshader) return false;
    _glgsg->_glAttachShader(_glsl_program, _glsl_cshader);
  }

  // There might be warnings. Only report them for one shader program.
  if (_glsl_vshader != 0) {
    glsl_report_shader_errors(_glsl_vshader);
  } else if (_glsl_fshader != 0) {
    glsl_report_shader_errors(_glsl_fshader);
  } else if (_glsl_gshader != 0) {
    glsl_report_shader_errors(_glsl_gshader);
  } else if (_glsl_tcshader != 0) {
    glsl_report_shader_errors(_glsl_tcshader);
  } else if (_glsl_teshader != 0) {
    glsl_report_shader_errors(_glsl_teshader);
  }

  // If we requested to retrieve the shader, we should indicate that before linking.
#if !defined(NDEBUG) && !defined(OPENGLES)
  if (gl_dump_compiled_shaders && _glgsg->_supports_get_program_binary) {
    _glgsg->_glProgramParameteri(_glsl_program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
  }
#endif

  _glgsg->_glLinkProgram(_glsl_program);

  GLint status;
  _glgsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    GLCAT.error() << "An error occurred while linking shader program!\n";
    glsl_report_program_errors(_glsl_program);
    return false;
  }

  // Dump the binary if requested.
#if !defined(NDEBUG) && !defined(OPENGLES)
  if (gl_dump_compiled_shaders && _glgsg->_supports_get_program_binary) {
    GLint length = 0;
    _glgsg->_glGetProgramiv(_glsl_program, GL_PROGRAM_BINARY_LENGTH, &length);
    length += 2;

    char filename[64];
    static int gl_dump_count = 0;
    sprintf(filename, "glsl_program%d.dump", gl_dump_count++);

    char *binary = new char[length];
    GLenum format;
    GLsizei num_bytes;
    _glgsg->_glGetProgramBinary(_glsl_program, length, &num_bytes, &format, (void*)binary);

    pofstream s;
    s.open(filename, ios::out | ios::binary);
    s.write(binary, num_bytes);
    s.close();

    GLCAT.info()
      << "Dumped " << num_bytes << " bytes of program binary with format 0x"
      << hex << format << dec << "  to " << filename << "\n";
    delete[] binary;
  }
#endif  // NDEBUG

  _glgsg->report_my_gl_errors();
  return true;
}

#endif  // OPENGLES_1
