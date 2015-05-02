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

#include "pStatGPUTimer.h"

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
  _uses_standard_vertex_arrays = false;
  _has_divisor = false;
  _color_attrib_index = -1;

  nassertv(s->get_language() == Shader::SL_GLSL);

  // We compile and analyze the shader here, instead of in shader.cxx, to avoid gobj getting a dependency on GL stuff.
  if (!glsl_compile_and_link()) {
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
            bind._func = Shader::SMF_compose;
            if (transpose) {
              bind._piece = Shader::SMP_transpose;
            } else {
              bind._piece = Shader::SMP_whole;
            }
            bind._arg[0] = NULL;
            bind._arg[1] = NULL;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
            bind._dep[1] = Shader::SSD_general | Shader::SSD_transform;

            if (matrix_name == "ModelViewProjectionMatrix") {
              if (inverse) {
                bind._part[0] = Shader::SMO_apiclip_to_view;
                bind._part[1] = Shader::SMO_view_to_model;
              } else {
                bind._part[0] = Shader::SMO_model_to_view;
                bind._part[1] = Shader::SMO_view_to_apiclip;
              }

            } else if (matrix_name == "ModelViewMatrix") {
              if (inverse) {
                bind._part[0] = Shader::SMO_apiview_to_view;
                bind._part[1] = Shader::SMO_view_to_model;
              } else {
                bind._part[0] = Shader::SMO_model_to_view;
                bind._part[1] = Shader::SMO_view_to_apiview;
              }

            } else if (matrix_name == "ProjectionMatrix") {
              if (inverse) {
                bind._part[0] = Shader::SMO_apiclip_to_view;
                bind._part[1] = Shader::SMO_view_to_apiview;
              } else {
                bind._part[0] = Shader::SMO_apiview_to_view;
                bind._part[1] = Shader::SMO_view_to_apiclip;
              }

            } else if (matrix_name == "NormalMatrix") {
              // This is really the upper 3x3 of the ModelViewMatrixInverseTranspose.
              if (inverse) {
                bind._part[0] = Shader::SMO_model_to_view;
                bind._part[1] = Shader::SMO_view_to_apiview;
              } else {
                bind._part[0] = Shader::SMO_apiview_to_view;
                bind._part[1] = Shader::SMO_view_to_model;
              }

              if (param_type != GL_FLOAT_MAT3) {
                GLCAT.error() << "p3d_NormalMatrix input should be mat3, not mat4!\n";
              } else {
                if (transpose) {
                  bind._piece = Shader::SMP_upper3x3;
                } else {
                  bind._piece = Shader::SMP_transpose3x3;
                }
              }

            } else if (matrix_name == "ModelMatrix") {
              if (inverse) {
                bind._part[0] = Shader::SMO_world_to_view;
                bind._part[1] = Shader::SMO_view_to_model;
              } else {
                bind._part[0] = Shader::SMO_model_to_view;
                bind._part[1] = Shader::SMO_view_to_world;
              }

            } else if (matrix_name == "ViewMatrix") {
              if (inverse) {
                bind._part[0] = Shader::SMO_apiview_to_view;
                bind._part[1] = Shader::SMO_view_to_world;
              } else {
                bind._part[0] = Shader::SMO_world_to_view;
                bind._part[1] = Shader::SMO_view_to_apiview;
              }

            } else if (matrix_name == "ViewProjectionMatrix") {
              if (inverse) {
                bind._part[0] = Shader::SMO_apiclip_to_view;
                bind._part[1] = Shader::SMO_view_to_world;
              } else {
                bind._part[0] = Shader::SMO_world_to_view;
                bind._part[1] = Shader::SMO_view_to_apiclip;
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

            string tail;
            bind._stage = string_to_int(noprefix.substr(7), tail);
            if (!tail.empty()) {
              GLCAT.error()
                << "Error parsing shader input name: unexpected '"
                << tail << "' in '" << param_name << "'\n";
              continue;
            }

            if (get_sampler_texture_type(bind._desired_type, param_type)) {
              _glgsg->_glUniform1i(p, s->_tex_spec.size());
              s->_tex_spec.push_back(bind);
            } else {
              GLCAT.error()
                << "Could not bind texture input " << param_name << "\n";
            }
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
              if (param_type != GL_FLOAT_VEC4) {
                GLCAT.error()
                  << "p3d_Material.ambient should be vec4\n";
              }
              bind._piece = Shader::SMP_row0;
              s->_mat_spec.push_back(bind);
              continue;

            } else if (noprefix == "Material.diffuse") {
              if (param_type != GL_FLOAT_VEC4) {
                GLCAT.error()
                  << "p3d_Material.diffuse should be vec4\n";
              }
              bind._piece = Shader::SMP_row1;
              s->_mat_spec.push_back(bind);
              continue;

            } else if (noprefix == "Material.emission") {
              if (param_type != GL_FLOAT_VEC4) {
                GLCAT.error()
                  << "p3d_Material.emission should be vec4\n";
              }
              bind._piece = Shader::SMP_row2;
              s->_mat_spec.push_back(bind);
              continue;

            } else if (noprefix == "Material.specular") {
              if (param_type != GL_FLOAT_VEC3) {
                GLCAT.error()
                  << "p3d_Material.specular should be vec3\n";
              }
              bind._piece = Shader::SMP_row3x3;
              s->_mat_spec.push_back(bind);
              continue;

            } else if (noprefix == "Material.shininess") {
              if (param_type != GL_FLOAT) {
                GLCAT.error()
                  << "p3d_Material.shininess should be float\n";
              }
              bind._piece = Shader::SMP_cell15;
              s->_mat_spec.push_back(bind);
              continue;
            }
          }
          if (noprefix == "ColorScale") {
            Shader::ShaderMatSpec bind;
            bind._id = arg_id;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_attr_colorscale;
            bind._arg[0] = NULL;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_colorscale;
            bind._part[1] = Shader::SMO_identity;
            bind._arg[1] = NULL;
            bind._dep[1] = Shader::SSD_NONE;

            if (param_type == GL_FLOAT_VEC3) {
              bind._piece = Shader::SMP_row3x3;
            } else if (param_type == GL_FLOAT_VEC4) {
              bind._piece = Shader::SMP_row3;
            } else {
              GLCAT.error()
                << "p3d_ColorScale should be vec3 or vec4\n";
              continue;
            }
            s->_mat_spec.push_back(bind);
            continue;
          }
          if (noprefix == "Color") {
            Shader::ShaderMatSpec bind;
            bind._id = arg_id;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_attr_color;
            bind._arg[0] = NULL;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_color;
            bind._part[1] = Shader::SMO_identity;
            bind._arg[1] = NULL;
            bind._dep[1] = Shader::SSD_NONE;

            if (param_type == GL_FLOAT_VEC3) {
              bind._piece = Shader::SMP_row3x3;
            } else if (param_type == GL_FLOAT_VEC4) {
              bind._piece = Shader::SMP_row3;
            } else {
              GLCAT.error()
                << "p3d_Color should be vec3 or vec4\n";
              continue;
            }
            s->_mat_spec.push_back(bind);
            continue;
          }
          if (noprefix == "ClipPlane") {
            if (param_type != GL_FLOAT_VEC4) {
              GLCAT.error()
                << "p3d_ClipPlane should be vec4 or vec4[]\n";
              continue;
            }
            for (int i = 0; i < param_size; ++i) {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              bind._id._seqno = seqno++;
              bind._piece = Shader::SMP_row3;
              bind._func = Shader::SMF_first;
              bind._index = i;
              bind._part[0] = Shader::SMO_apiview_clipplane_i;
              bind._arg[0] = NULL;
              bind._dep[0] = Shader::SSD_general | Shader::SSD_clip_planes;
              bind._part[1] = Shader::SMO_identity;
              bind._arg[1] = NULL;
              bind._dep[1] = Shader::SSD_NONE;
              s->_mat_spec.push_back(bind);
              _glsl_parameter_map.push_back(p + i);
            }
            continue;
          }
          if (noprefix == "LightModel.ambient") {
            Shader::ShaderMatSpec bind;
            bind._id = arg_id;
            bind._func = Shader::SMF_first;
            bind._part[0] = Shader::SMO_light_ambient;
            bind._arg[0] = NULL;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_light;
            bind._part[1] = Shader::SMO_identity;
            bind._arg[1] = NULL;
            bind._dep[1] = Shader::SSD_NONE;

            if (param_type == GL_FLOAT_VEC3) {
              bind._piece = Shader::SMP_row3x3;
            } else if (param_type == GL_FLOAT_VEC4) {
              bind._piece = Shader::SMP_row3;
            } else {
              GLCAT.error()
                << "p3d_LightModel.ambient should be vec3 or vec4\n";
              continue;
            }
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
            bind._func = Shader::SMF_compose;
            bind._part[0] = Shader::SMO_world_to_view;
            bind._part[1] = Shader::SMO_view_to_apiview;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
            bind._dep[1] = Shader::SSD_general | Shader::SSD_transform;
            s->_mat_spec.push_back(bind);
            continue;

          } else if (param_name == "osg_InverseViewMatrix") {
            bind._piece = Shader::SMP_whole;
            bind._func = Shader::SMF_compose;
            bind._part[0] = Shader::SMO_apiview_to_view;
            bind._part[1] = Shader::SMO_view_to_world;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
            bind._dep[1] = Shader::SSD_general | Shader::SSD_transform;
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
            case GL_INT_SAMPLER_1D:
            case GL_INT_SAMPLER_2D:
            case GL_INT_SAMPLER_3D:
            case GL_INT_SAMPLER_2D_ARRAY:
            case GL_INT_SAMPLER_CUBE:
            case GL_INT_SAMPLER_BUFFER:
            case GL_UNSIGNED_INT_SAMPLER_1D:
            case GL_UNSIGNED_INT_SAMPLER_2D:
            case GL_UNSIGNED_INT_SAMPLER_3D:
            case GL_UNSIGNED_INT_SAMPLER_CUBE:
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_BUFFER:
            case GL_SAMPLER_1D_SHADOW:
            case GL_SAMPLER_1D:
            case GL_SAMPLER_CUBE_SHADOW:
            case GL_SAMPLER_2D_ARRAY:
            case GL_SAMPLER_2D_ARRAY_SHADOW:
            case GL_SAMPLER_BUFFER:
#endif  // !OPENGLES
            case GL_SAMPLER_2D:
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_3D:
            case GL_SAMPLER_CUBE: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = InternalName::make(param_name);
              bind._desired_type = Texture::TT_2d_texture;
              bind._stage = texunitno++;
              if (get_sampler_texture_type(bind._desired_type, param_type)) {
                _glgsg->_glUniform1i(p, s->_tex_spec.size());
                s->_tex_spec.push_back(bind);
              }
              continue;
            }
            case GL_FLOAT_MAT2:
#ifndef OPENGLES
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
#endif
              GLCAT.warning() << "GLSL shader requested an unsupported matrix type\n";
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
              continue;
            }
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
              continue;
            }
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4: {
              PT(InternalName) iname = InternalName::make(param_name);
              if (iname->get_parent() != InternalName::get_root()) {
                // It might be something like an attribute of a shader
                // input, like a light parameter.  It might also just be
                // a custom struct parameter.  We can't know yet, sadly.
                Shader::ShaderMatSpec bind;
                bind._id = arg_id;
                switch (param_type) {
                case GL_FLOAT:
                  bind._piece = Shader::SMP_row3x1;
                  break;
                case GL_FLOAT_VEC2:
                  bind._piece = Shader::SMP_row3x2;
                  break;
                case GL_FLOAT_VEC3:
                  bind._piece = Shader::SMP_row3x3;
                  break;
                default:
                  bind._piece = Shader::SMP_row3;
                }
                bind._func = Shader::SMF_first;
                bind._part[0] = Shader::SMO_vec_constant_x_attrib;
                bind._arg[0] = iname;
                bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs;
                bind._part[1] = Shader::SMO_identity;
                bind._arg[1] = NULL;
                bind._dep[1] = Shader::SSD_NONE;
                s->_mat_spec.push_back(bind);
                continue;
              } // else fall through
            }
            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4: {
              Shader::ShaderPtrSpec bind;
              bind._id = arg_id;
              switch (param_type) {
                case GL_BOOL:
                case GL_INT:
                case GL_FLOAT:      bind._dim[1] = 1; break;
                case GL_BOOL_VEC2:
                case GL_INT_VEC2:
                case GL_FLOAT_VEC2: bind._dim[1] = 2; break;
                case GL_BOOL_VEC3:
                case GL_INT_VEC3:
                case GL_FLOAT_VEC3: bind._dim[1] = 3; break;
                case GL_BOOL_VEC4:
                case GL_INT_VEC4:
                case GL_FLOAT_VEC4: bind._dim[1] = 4; break;
                case GL_FLOAT_MAT3: bind._dim[1] = 9; break;
                case GL_FLOAT_MAT4: bind._dim[1] = 16; break;
              }
              switch (param_type) {
              case GL_BOOL:
              case GL_BOOL_VEC2:
              case GL_BOOL_VEC3:
              case GL_BOOL_VEC4:
              case GL_INT:
              case GL_INT_VEC2:
              case GL_INT_VEC3:
              case GL_INT_VEC4:
                bind._type = Shader::SPT_int;
                break;
              case GL_FLOAT:
              case GL_FLOAT_VEC2:
              case GL_FLOAT_VEC3:
              case GL_FLOAT_VEC4:
              case GL_FLOAT_MAT3:
              case GL_FLOAT_MAT4:
                bind._type = Shader::SPT_float;
                break;
              }
              bind._arg = InternalName::make(param_name);
              bind._dim[0] = 1;
              bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs;
              bind._dep[1] = Shader::SSD_NONE;
              s->_ptr_spec.push_back(bind);
              continue;
            }
#ifndef OPENGLES
            case GL_IMAGE_1D_EXT:
            case GL_IMAGE_2D_EXT:
            case GL_IMAGE_3D_EXT:
            case GL_IMAGE_CUBE_EXT:
            case GL_IMAGE_2D_ARRAY_EXT:
            case GL_IMAGE_BUFFER_EXT:
            case GL_INT_IMAGE_1D_EXT:
            case GL_INT_IMAGE_2D_EXT:
            case GL_INT_IMAGE_3D_EXT:
            case GL_INT_IMAGE_CUBE_EXT:
            case GL_INT_IMAGE_2D_ARRAY_EXT:
            case GL_INT_IMAGE_BUFFER_EXT:
            case GL_UNSIGNED_INT_IMAGE_1D_EXT:
            case GL_UNSIGNED_INT_IMAGE_2D_EXT:
            case GL_UNSIGNED_INT_IMAGE_3D_EXT:
            case GL_UNSIGNED_INT_IMAGE_CUBE_EXT:
            case GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT:
            case GL_UNSIGNED_INT_IMAGE_BUFFER_EXT:
              // This won't really change at runtime, so we might as well
              // bind once and then forget about it.
              _glgsg->_glUniform1i(p, imgunitno++);
              _glsl_img_inputs.push_back(InternalName::make(param_name));
              _glsl_img_textures.push_back(NULL);
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
          case GL_INT:
          case GL_INT_VEC2:
          case GL_INT_VEC3:
          case GL_INT_VEC4:
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
              case GL_INT:
              case GL_FLOAT:      bind._dim[1] = 1; break;
              case GL_BOOL_VEC2:
              case GL_INT_VEC2:
              case GL_FLOAT_VEC2: bind._dim[1] = 2; break;
              case GL_BOOL_VEC3:
              case GL_INT_VEC3:
              case GL_FLOAT_VEC3: bind._dim[1] = 3; break;
              case GL_BOOL_VEC4:
              case GL_INT_VEC4:
              case GL_FLOAT_VEC4: bind._dim[1] = 4; break;
              case GL_FLOAT_MAT3: bind._dim[1] = 9; break;
              case GL_FLOAT_MAT4: bind._dim[1] = 16; break;
            }
            switch (param_type) {
            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
              bind._type = Shader::SPT_int;
              break;
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4:
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT4:
              bind._type = Shader::SPT_float;
              break;
            }
            bind._arg = InternalName::make(param_name);
            bind._dim[0] = param_size;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs;
            bind._dep[1] = Shader::SSD_NONE;
            s->_ptr_spec.push_back(bind);
            continue;
          }
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
      bind._elements = 1;

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

      bind._integer = (param_type == GL_BOOL ||
                       param_type == GL_BOOL_VEC2 ||
                       param_type == GL_BOOL_VEC3 ||
                       param_type == GL_BOOL_VEC4 ||
                       param_type == GL_INT ||
                       param_type == GL_INT_VEC2 ||
                       param_type == GL_INT_VEC3 ||
                       param_type == GL_INT_VEC4 ||
#ifndef OPENGLES
                       param_type == GL_UNSIGNED_INT_VEC2 ||
                       param_type == GL_UNSIGNED_INT_VEC3 ||
                       param_type == GL_UNSIGNED_INT_VEC4 ||
#endif
                       param_type == GL_UNSIGNED_INT);

      if (noprefix.empty()) {
        // Arbitrarily named attribute.
        bind._name = InternalName::make(param_name);

      } else if (noprefix == "Vertex") {
        bind._name = InternalName::get_vertex();

      } else if (noprefix == "Normal") {
        bind._name = InternalName::get_normal();

      } else if (noprefix == "Color") {
        bind._name = InternalName::get_color();

        // Save the index, so we can apply special handling to this attrib.
        _color_attrib_index = p;

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

      // Get the number of bind points.
      switch (param_type) {
      case GL_FLOAT_MAT3:
#ifndef OPENGLES
      case GL_DOUBLE_MAT3:
#endif
        bind._elements = 3 * param_size;
        break;

      case GL_FLOAT_MAT4:
#ifndef OPENGLES
      case GL_DOUBLE_MAT4:
#endif
        bind._elements = 4 * param_size;
        break;

      default:
        bind._elements = param_size;
        break;
      }

      s->_var_spec.push_back(bind);
    }
  }
  _glgsg->_glUseProgram(0);

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::get_sampler_texture_type
//       Access: Public
//  Description: Returns the texture type required for the given
//               GL sampler type.  Returns false if unsupported.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
get_sampler_texture_type(int &out, GLenum param_type) {
  switch (param_type) {
#ifndef OPENGLES
  case GL_SAMPLER_1D_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_1D:
  case GL_UNSIGNED_INT_SAMPLER_1D:
  case GL_SAMPLER_1D:
    out = Texture::TT_1d_texture;
    return true;

  case GL_INT_SAMPLER_2D:
  case GL_UNSIGNED_INT_SAMPLER_2D:
#endif
  case GL_SAMPLER_2D:
    out = Texture::TT_2d_texture;
    return true;

  case GL_SAMPLER_2D_SHADOW:
    out = Texture::TT_2d_texture;
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    return true;

#ifndef OPENGLES
  case GL_INT_SAMPLER_3D:
  case GL_UNSIGNED_INT_SAMPLER_3D:
#endif
  case GL_SAMPLER_3D:
    out = Texture::TT_3d_texture;
    if (_glgsg->_supports_3d_texture) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses 3D texture, which is unsupported by the driver.\n";
      return false;
    }

#ifndef OPENGLES
  case GL_SAMPLER_CUBE_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_CUBE:
  case GL_UNSIGNED_INT_SAMPLER_CUBE:
#endif
  case GL_SAMPLER_CUBE:
    out = Texture::TT_cube_map;
    if (!_glgsg->_supports_cube_map) {
      GLCAT.error()
        << "GLSL shader uses cube map, which is unsupported by the driver.\n";
      return false;
    }
    return true;

#ifndef OPENGLES
  case GL_SAMPLER_2D_ARRAY_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_2D_ARRAY:
  case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
  case GL_SAMPLER_2D_ARRAY:
    out = Texture::TT_2d_texture_array;
    if (_glgsg->_supports_2d_texture_array) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses 2D texture array, which is unsupported by the driver.\n";
      return false;
    }

  case GL_INT_SAMPLER_BUFFER:
  case GL_UNSIGNED_INT_SAMPLER_BUFFER:
  case GL_SAMPLER_BUFFER:
    out = Texture::TT_buffer_texture;
    if (_glgsg->_supports_buffer_texture) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses buffer texture, which is unsupported by the driver.\n";
      return false;
    }
#endif

  default:
    GLCAT.error()
      << "GLSL shader uses unsupported sampler type for texture input.\n";
    return false;
  }
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
    GLSLShaders::const_iterator it;
    for (it = _glsl_shaders.begin(); it != _glsl_shaders.end(); ++it) {
      _glgsg->_glDetachShader(_glsl_program, *it);
    }
    _glgsg->_glDeleteProgram(_glsl_program);
    _glsl_program = 0;
  }

  GLSLShaders::const_iterator it;
  for (it = _glsl_shaders.begin(); it != _glsl_shaders.end(); ++it) {
    _glgsg->_glDeleteShader(*it);
  }

  _glsl_shaders.clear();

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
  //PStatGPUTimer timer(_glgsg, _glgsg->_draw_set_state_shader_parameters_pcollector);

  if (!valid()) {
    return;
  }

  // Iterate through _ptr parameters
  for (int i = 0; i < (int)_shader->_ptr_spec.size(); ++i) {
    Shader::ShaderPtrSpec &spec = _shader->_ptr_spec[i];

    if (altered & (spec._dep[0] | spec._dep[1])) {
      const Shader::ShaderPtrData* ptr_data = _glgsg->fetch_ptr_parameter(spec);
      if (ptr_data == NULL) { //the input is not contained in ShaderPtrData
        release_resources();
        return;
      }

      GLint p = _glsl_parameter_map[spec._id._seqno];
      switch (spec._type) {
      case Shader::SPT_float:
        {
          float *data = NULL;

          switch (ptr_data->_type) {
          case Shader::SPT_int:
            // Convert int data to float data.
            data = (float*) alloca(sizeof(float) * spec._dim[0] * spec._dim[1]);
            for (int i = 0; i < (spec._dim[0] * spec._dim[1]); ++i) {
              data[i] = (float)(((int*)ptr_data->_ptr)[i]);
            }
            break;

          case Shader::SPT_double:
            // Downgrade double data to float data.
            data = (float*) alloca(sizeof(float) * spec._dim[0] * spec._dim[1]);
            for (int i = 0; i < (spec._dim[0] * spec._dim[1]); ++i) {
              data[i] = (float)(((double*)ptr_data->_ptr)[i]);
            }
            break;

          case Shader::SPT_float:
            data = (float*)ptr_data->_ptr;
            break;

          default:
            nassertd(false) continue;
          }

          switch (spec._dim[1]) {
          case 1: _glgsg->_glUniform1fv(p, spec._dim[0], (float*)data); continue;
          case 2: _glgsg->_glUniform2fv(p, spec._dim[0], (float*)data); continue;
          case 3: _glgsg->_glUniform3fv(p, spec._dim[0], (float*)data); continue;
          case 4: _glgsg->_glUniform4fv(p, spec._dim[0], (float*)data); continue;
          case 9: _glgsg->_glUniformMatrix3fv(p, spec._dim[0], GL_FALSE, (float*)data); continue;
          case 16: _glgsg->_glUniformMatrix4fv(p, spec._dim[0], GL_FALSE, (float*)data); continue;
          }
          nassertd(false) continue;
        }
        break;

      case Shader::SPT_int:
        if (ptr_data->_type != Shader::SPT_int) {
          GLCAT.error()
            << "Cannot pass floating-point data to integer shader input '" << spec._id._name << "'\n";

          // Deactivate it to make sure the user doesn't get flooded with this error.
          spec._dep[0] = 0;
          spec._dep[1] = 0;

        } else {
          switch (spec._dim[1]) {
          case 1: _glgsg->_glUniform1iv(p, spec._dim[0], (int*)ptr_data->_ptr); continue;
          case 2: _glgsg->_glUniform2iv(p, spec._dim[0], (int*)ptr_data->_ptr); continue;
          case 3: _glgsg->_glUniform3iv(p, spec._dim[0], (int*)ptr_data->_ptr); continue;
          case 4: _glgsg->_glUniform4iv(p, spec._dim[0], (int*)ptr_data->_ptr); continue;
          }
          nassertd(false) continue;
        }
        break;

      case Shader::SPT_double:
        GLCAT.error() << "Passing double-precision shader inputs to GLSL shaders is not currently supported\n";

        // Deactivate it to make sure the user doesn't get flooded with this error.
        spec._dep[0] = 0;
        spec._dep[1] = 0;

      default:
        continue;
      }
    }
  }

  for (int i = 0; i < (int)_shader->_mat_spec.size(); ++i) {
    Shader::ShaderMatSpec &spec = _shader->_mat_spec[i];

    if (altered & (spec._dep[0] | spec._dep[1])) {
      const LMatrix4 *val = _glgsg->fetch_specified_value(spec, altered);
      if (!val) continue;
#ifndef STDFLOAT_DOUBLE
      // In this case, the data is already single-precision.
      const PN_float32 *data = val->get_data();
#else
      // In this case, we have to convert it.
      LMatrix4f valf = LCAST(PN_float32, *val);
      const PN_float32 *data = valf.get_data();
#endif

      GLint p = _glsl_parameter_map[spec._id._seqno];
      switch (spec._piece) {
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
#ifndef STDFLOAT_DOUBLE
          LMatrix3f upper3 = val->get_upper_3();
#else
          LMatrix3f upper3 = valf.get_upper_3();
#endif
          _glgsg->_glUniformMatrix3fv(p, 1, false, upper3.get_data());
          continue;
        }
      case Shader::SMP_transpose3x3:
        {
#ifndef STDFLOAT_DOUBLE
          LMatrix3f upper3 = val->get_upper_3();
#else
          LMatrix3f upper3 = valf.get_upper_3();
#endif
          _glgsg->_glUniformMatrix3fv(p, 1, true, upper3.get_data());
          continue;
        }
      case Shader::SMP_cell15:
        _glgsg->_glUniform1fv(p, 1, data+15);
        continue;
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
    const Shader::ShaderVarSpec &bind = _shader->_var_spec[i];
    const GLint p = _glsl_parameter_map[bind._id._seqno];
    if (_has_divisor) {
      _glgsg->_glVertexAttribDivisor(p, 0);
    }
    for (int i = 0; i < bind._elements; ++i) {
      _glgsg->_glDisableVertexAttribArray(p + i);
    }
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
  if (prev) {
    prev->disable_shader_vertex_arrays();
  }
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
      const Shader::ShaderVarSpec &bind = _shader->_var_spec[i];
      InternalName *name = bind._name;
      int texslot = bind._append_uv;

      if (texslot >= 0 && texslot < _glgsg->_state_texture->get_num_on_stages()) {
        TextureStage *stage = _glgsg->_state_texture->get_on_stage(texslot);
        InternalName *texname = stage->get_texcoord_name();

        if (name == InternalName::get_texcoord()) {
          name = texname;
        } else if (texname != InternalName::get_texcoord()) {
          name = name->append(texname->get_basename());
        }
      }
      GLint p = _glsl_parameter_map[bind._id._seqno];

      // Don't apply vertex colors if they are disabled with a ColorAttrib.
      int num_elements, element_stride, divisor;
      bool normalized;
      if ((p != _color_attrib_index || _glgsg->_vertex_colors_enabled) &&
          _glgsg->_data_reader->get_array_info(name, array_reader,
                                               num_values, numeric_type,
                                               normalized, start, stride, divisor,
                                               num_elements, element_stride)) {
        const unsigned char *client_pointer;
        if (!_glgsg->setup_array_data(client_pointer, array_reader, force)) {
          return false;
        }
        client_pointer += start;

        for (int i = 0; i < num_elements; ++i) {
          _glgsg->_glEnableVertexAttribArray(p);

#ifndef OPENGLES
          if (bind._integer) {
            _glgsg->_glVertexAttribIPointer(p, num_values, _glgsg->get_numeric_type(numeric_type),
                                            stride, client_pointer);
          } else
#endif
          if (numeric_type == GeomEnums::NT_packed_dabc) {
            // GL_BGRA is a special accepted value available since OpenGL 3.2.
            // It requires us to pass GL_TRUE for normalized.
            _glgsg->_glVertexAttribPointer(p, GL_BGRA, GL_UNSIGNED_BYTE,
                                           GL_TRUE, stride, client_pointer);
          } else {
            _glgsg->_glVertexAttribPointer(p, num_values,
                                           _glgsg->get_numeric_type(numeric_type),
                                           normalized, stride, client_pointer);
          }

          if (_glgsg->_supports_vertex_attrib_divisor) {
            _glgsg->_glVertexAttribDivisor(p, divisor);
            _has_divisor = true;
          }

          ++p;
          client_pointer += element_stride;
        }
      } else {
        for (int i = 0; i < bind._elements; ++i) {
          _glgsg->_glDisableVertexAttribArray(p + i);
        }
        if (p == _color_attrib_index) {
          // Vertex colors are disabled or not present.  Apply flat color.
#if defined(STDFLOAT_DOUBLE) && !defined(OPENGLES)
          _glgsg->_glVertexAttrib4dv(p, _glgsg->_scene_graph_color.get_data());
#else
          _glgsg->_glVertexAttrib4fv(p, _glgsg->_scene_graph_color.get_data());
#endif
        }
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

  for (int i = 0; i < _shader->_tex_spec.size(); ++i) {
#ifndef OPENGLES
    // Check if bindless was used, if so, there's nothing to unbind.
    if (_glgsg->_supports_bindless_texture) {
      GLint p = _glsl_parameter_map[_shader->_tex_spec[i]._id._seqno];

      if (_glsl_uniform_handles.count(p) > 0) {
        continue;
      }
    }

    if (_glgsg->_supports_multi_bind) {
      // There are non-bindless textures to unbind, and we're lazy,
      // so let's go and unbind everything after this point using one
      // multi-bind call, and then break out of the loop.
      _glgsg->_glBindTextures(i, _shader->_tex_spec.size() - i, NULL);
      break;
    }
#endif

    _glgsg->_glActiveTexture(GL_TEXTURE0 + i);

    switch (_shader->_tex_spec[i]._desired_type) {
    case Texture::TT_1d_texture:
#ifndef OPENGLES
      glBindTexture(GL_TEXTURE_1D, 0);
#endif
      break;

    case Texture::TT_2d_texture:
      glBindTexture(GL_TEXTURE_2D, 0);
      break;

    case Texture::TT_3d_texture:
#ifndef OPENGLES_1
      glBindTexture(GL_TEXTURE_3D, 0);
#endif
      break;

    case Texture::TT_2d_texture_array:
#ifndef OPENGLES
      glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
#endif
      break;

    case Texture::TT_cube_map:
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
      break;

    case Texture::TT_buffer_texture:
#ifndef OPENGLES
      glBindTexture(GL_TEXTURE_BUFFER, 0);
#endif
      break;
    }
  }

#ifndef OPENGLES
  // Now unbind all the image units.  Not sure if we *have* to do this.
  int num_image_units = min(_glsl_img_inputs.size(), (size_t)_glgsg->_max_image_units);

  if (num_image_units > 0) {
    if (_glgsg->_supports_multi_bind) {
      _glgsg->_glBindImageTextures(0, num_image_units, NULL);

    } else {
      for (int i = 0; i < num_image_units; ++i) {
        _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
      }
    }

    if (gl_enable_memory_barriers) {
      for (int i = 0; i < num_image_units; ++i) {
        const InternalName *name = _glsl_img_inputs[i];
        const ShaderInput *input = _glgsg->_target_shader->get_shader_input(name);

        if ((input->_access & ShaderInput::A_write) != 0) {
          _glsl_img_textures[i]->mark_incoherent(true);
        } else {
          _glsl_img_textures[i]->mark_incoherent(false);
        }
        _glsl_img_textures[i] = NULL;
      }
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
  GLbitfield barriers = 0;

  // First bind all the 'image units'; a bit of an esoteric OpenGL feature right now.
  int num_image_units = min(_glsl_img_inputs.size(), (size_t)_glgsg->_max_image_units);

  if (num_image_units > 0) {
    for (int i = 0; i < num_image_units; ++i) {
      const InternalName *name = _glsl_img_inputs[i];
      const ShaderInput *input = _glgsg->_target_shader->get_shader_input(name);
      Texture *tex = input->get_texture();

      GLuint gl_tex = 0;
      CLP(TextureContext) *gtc;

      if (tex != NULL) {
        int view = _glgsg->get_current_tex_view_offset();

        gtc = DCAST(CLP(TextureContext), tex->prepare_now(view, _glgsg->_prepared_objects, _glgsg));
        if (gtc != (TextureContext*)NULL) {
          _glsl_img_textures[i] = gtc;

          gl_tex = gtc->_index;
          _glgsg->update_texture(gtc, true);

          if (gtc->needs_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)) {
            barriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
          }
        }
      }

      if (gl_tex == 0) {
        _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
      } else {
        //TODO: automatically convert to sized type instead of plain GL_RGBA
        // If a base type is used, it will crash.
        GLenum internal_format = gtc->_internal_format;
        if (internal_format == GL_RGBA || internal_format == GL_RGB) {
          GLCAT.error()
            << "Texture " << tex->get_name() << " has an unsized format.  Textures bound "
            << "to a shader as an image need a sized format.\n";

          // This may not actually be right, but may still prevent a crash.
          internal_format = _glgsg->get_internal_image_format(tex, true);
        }

        GLenum access = GL_READ_ONLY;
        GLboolean layered = (input->_access & ShaderInput::A_layered) != 0;

        if ((input->_access & ShaderInput::A_read) != 0 &&
            (input->_access & ShaderInput::A_write) != 0) {
          access = GL_READ_WRITE;

        } else if ((input->_access & ShaderInput::A_read) != 0) {
          access = GL_READ_ONLY;

        } else if ((input->_access & ShaderInput::A_write) != 0) {
          access = GL_WRITE_ONLY;

        } else {
          access = GL_READ_ONLY;
          gl_tex = 0;
        }
        _glgsg->_glBindImageTexture(i, gl_tex, input->_bind_level, layered,
                                    input->_bind_layer, access, gtc->_internal_format);
      }
    }
  }
#endif

  // We get the TextureAttrib directly from the _target_rs, not the
  // filtered TextureAttrib in _target_texture.
  const TextureAttrib *texattrib = DCAST(TextureAttrib, _glgsg->_target_rs->get_attrib_def(TextureAttrib::get_class_slot()));
  nassertv(texattrib != (TextureAttrib *)NULL);

  for (int i = 0; i < (int)_shader->_tex_spec.size(); ++i) {
    Shader::ShaderTexSpec &spec = _shader->_tex_spec[i];
    const InternalName *id = spec._name;
    int texunit = spec._stage;

    Texture *tex = NULL;
    int view = _glgsg->get_current_tex_view_offset();
    SamplerState sampler;

    if (id != NULL) {
      const ShaderInput *input = _glgsg->_target_shader->get_shader_input(id);
      tex = input->get_texture();
      sampler = input->get_sampler();

    } else {
      if (texunit >= texattrib->get_num_on_stages()) {
        continue;
      }
      TextureStage *stage = texattrib->get_on_stage(texunit);
      tex = texattrib->get_on_texture(stage);
      sampler = texattrib->get_on_sampler(stage);
      view += stage->get_tex_view_offset();
    }

    if (tex == NULL) {
      continue;
    }

    if (tex->get_texture_type() != spec._desired_type) {
      if (id != NULL) {
        GLCAT.error()
          << "Sampler type of GLSL shader input '" << *id << "' does not "
             "match type of texture " << *tex << ".\n";
      } else {
        GLCAT.error()
          << "Sampler type of GLSL shader input p3d_Texture" << texunit
          << " does not match type of texture " << *tex << ".\n";
      }
      //TODO: also check whether shadow sampler textures have shadow filter enabled.
    }

    CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tex->prepare_now(view, _glgsg->_prepared_objects, _glgsg));
    if (gtc == NULL) {
      continue;
    }

    GLint p = _glsl_parameter_map[spec._id._seqno];

#ifndef OPENGLES
    // If it was recently written to, we will have to issue a memory barrier soon.
    if (gtc->needs_barrier(GL_TEXTURE_FETCH_BARRIER_BIT)) {
      barriers |= GL_TEXTURE_FETCH_BARRIER_BIT;
    }

    // Try bindless texturing first, if supported.
    if (gl_use_bindless_texture && _glgsg->_supports_bindless_texture) {
      // We demand the real texture, since we won't be able
      // to change the texture properties after this point.
      if (!_glgsg->update_texture(gtc, true)) {
        continue;
      }

      GLuint64 handle = gtc->get_handle();
      if (handle != 0) {
        gtc->make_handle_resident();
        gtc->set_active(true);

        // Check if we have already specified this texture handle.
        // If so, no need to call glUniformHandle again.
        pmap<GLint, GLuint64>::const_iterator it;
        it = _glsl_uniform_handles.find(p);
        if (it != _glsl_uniform_handles.end() && it->second == handle) {
          // Already specified.
          continue;
        } else {
          _glgsg->_glUniformHandleui64(p, handle);
          _glsl_uniform_handles[p] = handle;
        }
        continue;
      }
    }
#endif

    // Bindless texturing wasn't supported or didn't work, so
    // let's just bind the texture normally.
    _glgsg->_glActiveTexture(GL_TEXTURE0 + i);
    if (!_glgsg->update_texture(gtc, false)) {
      continue;
    }
    _glgsg->apply_texture(gtc);
    _glgsg->apply_sampler(i, sampler, gtc);
  }

#ifndef OPENGLES
  if (barriers != 0) {
    // Issue a memory barrier prior to this shader's execution.
    _glgsg->issue_memory_barrier(barriers);
  }
#endif

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_shader_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_shader_errors(GLuint shader, Shader::ShaderType type, bool fatal) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

  if (length <= 1) {
    return;
  }

  info_log = (char *) alloca(length);
  _glgsg->_glGetShaderInfoLog(shader, length, &num_chars, info_log);
  if (strcmp(info_log, "Success.\n") == 0 ||
      strcmp(info_log, "No errors.\n") == 0) {
    return;
  }

  // Parse the errors so that we can substitute in actual file
  // locations instead of source indices.
  istringstream log(info_log);
  string line;
  while (getline(log, line)) {
    int fileno, lineno;
    int prefixlen = 0;

    // First is AMD/Intel driver syntax, second is NVIDIA syntax.
    if (sscanf(line.c_str(), "ERROR: %d:%d: %n", &fileno, &lineno, &prefixlen) == 2
        && prefixlen > 0) {

      Filename fn = _shader->get_filename_from_index(fileno, type);
      GLCAT.error(false)
        << "ERROR: " << fn << ":" << lineno << ": " << (line.c_str() + prefixlen) << "\n";

    } else if (sscanf(line.c_str(), "WARNING: %d:%d: %n", &fileno, &lineno, &prefixlen) == 2
        && prefixlen > 0) {

      Filename fn = _shader->get_filename_from_index(fileno, type);
      GLCAT.warning(false)
        << "WARNING: " << fn << ":" << lineno << ": " << (line.c_str() + prefixlen) << "\n";


    } else if (sscanf(line.c_str(), "%d(%d) : %n", &fileno, &lineno, &prefixlen) == 2
               && prefixlen > 0) {

      Filename fn = _shader->get_filename_from_index(fileno, type);
      GLCAT.error(false)
        << fn << "(" << lineno << ") : " << (line.c_str() + prefixlen) << "\n";

    } else if (!fatal) {
      GLCAT.warning(false) << line << "\n";

    } else {
      GLCAT.error(false) << line << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_program_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a program.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_program_errors(GLuint program, bool fatal) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

  if (length > 1) {
    info_log = (char *) alloca(length);
    _glgsg->_glGetProgramInfoLog(program, length, &num_chars, info_log);

    if (strcmp(info_log, "Success.\n") != 0 && strcmp(info_log, "No errors.\n") != 0) {
      if (!fatal) {
        GLCAT.warning(false) << info_log << "\n";
      } else {
        GLCAT.error(false) << info_log << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_compile_shader
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
glsl_compile_shader(Shader::ShaderType type) {
  GLuint handle = 0;
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
    default:
      break;
  }
  if (!handle) {
    GLCAT.error()
      << "Could not create a GLSL shader of the requested type.\n";
    _glgsg->report_my_gl_errors();
    return false;
  }

  if (_glgsg->_use_object_labels) {
    string name = _shader->get_filename(type);
    _glgsg->_glObjectLabel(GL_SHADER, handle, name.size(), name.data());
  }

  string text_str = _shader->get_text(type);
  const char* text = text_str.c_str();
  _glgsg->_glShaderSource(handle, 1, &text, NULL);
  _glgsg->_glCompileShader(handle);
  GLint status;
  _glgsg->_glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    GLCAT.error()
      << "An error occurred while compiling GLSL shader "
      << _shader->get_filename(type) << ":\n";
    glsl_report_shader_errors(handle, type, true);
    _glgsg->_glDeleteShader(handle);
    _glgsg->report_my_gl_errors();
    return false;
  }

  _glgsg->_glAttachShader(_glsl_program, handle);
  _glsl_shaders.push_back(handle);

  // There might be warnings, so report those.
  glsl_report_shader_errors(handle, type, false);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_compile_and_link
//       Access: Private
//  Description: This subroutine compiles a GLSL shader.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
glsl_compile_and_link() {
  _glsl_shaders.clear();
  _glsl_program = _glgsg->_glCreateProgram();
  if (!_glsl_program) {
    return false;
  }

  if (_glgsg->_use_object_labels) {
    string name = _shader->get_filename();
    _glgsg->_glObjectLabel(GL_PROGRAM, _glsl_program, name.size(), name.data());
  }

  bool valid = true;

  if (!_shader->get_text(Shader::ST_vertex).empty()) {
    valid &= glsl_compile_shader(Shader::ST_vertex);
  }

  if (!_shader->get_text(Shader::ST_fragment).empty()) {
    valid &= glsl_compile_shader(Shader::ST_fragment);
  }

  // OpenGL ES has no geometry shaders.
#ifndef OPENGLES
  if (!_shader->get_text(Shader::ST_geometry).empty()) {
    valid &= glsl_compile_shader(Shader::ST_geometry);

    // Set the vertex output limit to the maximum.
    // This is slow, but it is probably reasonable to require
    // the user to override this in his shader using layout().
    nassertr(_glgsg->_glProgramParameteri != NULL, false);
    GLint max_vertices;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_vertices);
    _glgsg->_glProgramParameteri(_glsl_program, GL_GEOMETRY_VERTICES_OUT_ARB, max_vertices);
  }
#endif

  if (!_shader->get_text(Shader::ST_tess_control).empty()) {
    valid &= glsl_compile_shader(Shader::ST_tess_control);
  }

  if (!_shader->get_text(Shader::ST_tess_evaluation).empty()) {
    valid &= glsl_compile_shader(Shader::ST_tess_evaluation);
  }

  if (!_shader->get_text(Shader::ST_compute).empty()) {
    valid &= glsl_compile_shader(Shader::ST_compute);
  }

  // There might be warnings, so report those.
  //GLSLShaders::const_iterator it;
  //for (it = _glsl_shaders.begin(); it != _glsl_shaders.end(); ++it) {
  //  glsl_report_shader_errors(*it);
  //}

  // Under OpenGL's compatibility profile, we have to make sure that we bind
  // something to attribute 0.  Make sure that this is the position array.
  _glgsg->_glBindAttribLocation(_glsl_program, 0, "p3d_Vertex");
  _glgsg->_glBindAttribLocation(_glsl_program, 0, "vertex");

  // While we're at it, let's also map these to fixed locations.  These
  // attributes were historically fixed to these locations, so it might
  // help a buggy driver.
  _glgsg->_glBindAttribLocation(_glsl_program, 2, "p3d_Normal");
  _glgsg->_glBindAttribLocation(_glsl_program, 3, "p3d_Color");

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
    GLCAT.error() << "An error occurred while linking GLSL shader program!\n";
    glsl_report_program_errors(_glsl_program, true);
    return false;
  }

  // Report any warnings.
  glsl_report_program_errors(_glsl_program, false);

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
    s.open(filename, ios::out | ios::binary | ios::trunc);
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
