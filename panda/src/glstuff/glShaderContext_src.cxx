/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glShaderContext_src.cxx
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 *   parameter types only supported under Cg)
 */

#ifndef OPENGLES_1

#include "pStatGPUTimer.h"

#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "materialAttrib.h"
#include "shaderAttrib.h"
#include "fogAttrib.h"
#include "lightAttrib.h"
#include "clipPlaneAttrib.h"
#include "bamCache.h"

using std::dec;
using std::hex;
using std::max;
using std::min;
using std::string;

TypeHandle CLP(ShaderContext)::_type_handle;

/**
 * The Panda CG shader syntax defines a useful set of shorthand notations for
 * setting nodepath properties as shaderinputs.  For example, float4 mspos_XXX
 * refers to nodepath XXX's position in model space.  This function is a rough
 * attempt to reimplement some of the shorthand notations for GLSL. The code
 * is ~99% composed of excerpts dealing with matrix shaderinputs from
 * Shader::compile_parameter.
 *
 * Given a uniform variable name queried from the compiled shader passed in
 * via arg_id, 1) parse the name 2a) if the name refers to a Panda shorthand
 * notation push the appropriate matrix into shader._mat_spec returns True 2b)
 * If the name doesn't refer to a Panda shorthand notation returns False
 *
 * The boolean return is used to notify down-river processing whether the
 * shader var/parm was actually picked up and the appropriate ShaderMatSpec
 * pushed onto _mat_spec.
 */
bool CLP(ShaderContext)::
parse_and_set_short_hand_shader_vars(Shader::ShaderArgId &arg_id, GLenum param_type, GLint param_size, Shader *objShader) {
  Shader::ShaderArgInfo p;
  p._id = arg_id;
  p._cat = GLCAT;

  string basename(arg_id._name);

  // Split it at the underscores.
  vector_string pieces;
  tokenize(basename, pieces, "_");

  if (pieces.size() == 0 || pieces[0].size() < 3) {
    return false;
  }

  // mstrans, wstrans, vstrans, cstrans, mspos, wspos, vspos, cspos
  if (strcmp(pieces[0].c_str() + 1, "strans") == 0 ||
      strcmp(pieces[0].c_str() + 1, "spos") == 0) {
    pieces.push_back("to");

    switch (pieces[0][0]) {
    case 'm':
      pieces.push_back("model");
      break;
    case 'w':
      pieces.push_back("world");
      break;
    case 'v':
      pieces.push_back("view");
      break;
    case 'c':
      pieces.push_back("clip");
      break;
    default:
      return false;
    }
    if (strcmp(pieces[0].c_str() + 1, "strans") == 0) {
      pieces[0] = "trans";
    } else {
      pieces[0] = "row3";
    }
  // mat_modelproj et al
  } else if (pieces[0].size() == 3 &&
             (pieces[0] == "mat" || pieces[0] == "inv" ||
              pieces[0] == "tps" || pieces[0] == "itp")) {
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
    if (param_type == GL_FLOAT_MAT4) {
      if      (pieces[0] == "trans") bind._piece = Shader::SMP_whole;
      else if (pieces[0] == "tpose") bind._piece = Shader::SMP_transpose;
      else {
        GLCAT.error() << basename << " should be vec4, not mat3\n";
        return false;
      }
    } else if (param_type == GL_FLOAT_MAT3) {
      if      (pieces[0] == "trans") bind._piece = Shader::SMP_upper3x3;
      else if (pieces[0] == "tpose") bind._piece = Shader::SMP_transpose3x3;
      else {
        GLCAT.error() << basename << " should be vec4, not mat3\n";
        return false;
      }
    } else if (param_type == GL_FLOAT_VEC4) {
      if      (pieces[0] == "trans") bind._piece = Shader::SMP_col0;
      else if (pieces[0] == "tpose") bind._piece = Shader::SMP_row0;
      else if (pieces[0] == "row0")  bind._piece = Shader::SMP_row0;
      else if (pieces[0] == "row1")  bind._piece = Shader::SMP_row1;
      else if (pieces[0] == "row2")  bind._piece = Shader::SMP_row2;
      else if (pieces[0] == "row3")  bind._piece = Shader::SMP_row3;
      else if (pieces[0] == "col0")  bind._piece = Shader::SMP_col0;
      else if (pieces[0] == "col1")  bind._piece = Shader::SMP_col1;
      else if (pieces[0] == "col2")  bind._piece = Shader::SMP_col2;
      else if (pieces[0] == "col3")  bind._piece = Shader::SMP_col3;
      else {
        GLCAT.error() << basename << " should be mat4, not vec4\n";
        return false;
      }
    } else if (pieces[0] == "row3") {
      // We'll permit this too, simply because we can support it.
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
        GLCAT.error() << basename << " should be vec4\n";
        return false;
      }
    } else if (pieces[0] == "trans" || pieces[0] == "tpose") {
      GLCAT.error() << basename << " should be mat4 or mat3\n";
      return false;
    } else {
      GLCAT.error() << basename << " should be vec4\n";
      return false;
    }

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

    // clip == apiclip in OpenGL, and the apiclip matrices are cached.
    if (bind._part[0] == Shader::SMO_view_to_clip) {
      bind._part[0] = Shader::SMO_view_to_apiclip;
    } else if (bind._part[0] == Shader::SMO_clip_to_view) {
      bind._part[0] = Shader::SMO_apiclip_to_view;
    }
    if (bind._part[1] == Shader::SMO_view_to_clip) {
      bind._part[1] = Shader::SMO_view_to_apiclip;
    } else if (bind._part[1] == Shader::SMO_clip_to_view) {
      bind._part[1] = Shader::SMO_apiclip_to_view;
    }

    objShader->cp_optimize_mat_spec(bind);
    objShader->_mat_spec.push_back(bind);
    objShader->_mat_deps |= bind._dep[0] | bind._dep[1];

    if (param_size > 1) {
      // We support arrays of rows and arrays of columns, so we can run the
      // GLSL shaders that cgc spits out.
      if (bind._piece == Shader::SMP_row0 || bind._piece == Shader::SMP_col0) {
        if (param_size > 4) {
          GLCAT.warning() << basename << "[" << param_size << "] is too large, only the first four elements will be defined\n";
          param_size = 4;
        }
        for (int i = 1; i < param_size; ++i) {
          bind._id._seqno += 1;
          bind._piece = (Shader::ShaderMatPiece)((int)bind._piece + 1);
          objShader->_mat_spec.push_back(bind);
        }
      } else {
        GLCAT.warning() << basename << "[" << param_size << "] should not be an array, only the first element will be defined\n";
      }
    }

    return true;
  }
  return false;
}

/**
 * xyz
 */
CLP(ShaderContext)::
CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s) : ShaderContext(s) {
  _glgsg = glgsg;
  _glsl_program = 0;
  _uses_standard_vertex_arrays = false;
  _enabled_attribs.clear();
  _color_attrib_index = -1;
  _transform_table_index = -1;
  _slider_table_index = -1;
  _frame_number_loc = -1;
  _frame_number = -1;
  _validated = !gl_validate_shaders;

  nassertv(s->get_language() == Shader::SL_GLSL);

  // We compile and analyze the shader here, instead of in shader.cxx, to
  // avoid gobj getting a dependency on GL stuff.
  if (!glsl_compile_and_link()) {
    release_resources();
    s->_error_flag = true;
    return;
  }

  // Process the vertex attributes first.
  GLint param_count = 0;
  GLint name_buflen = 0;
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTES, &param_count);
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_buflen);
  name_buflen = max(64, name_buflen);
  char *name_buffer = (char *)alloca(name_buflen);

  _shader->_var_spec.clear();
  for (int i = 0; i < param_count; ++i) {
    reflect_attribute(i, name_buffer, name_buflen);
  }

  /*if (gl_fixed_vertex_attrib_locations) {
    // Relink the shader for glBindAttribLocation to take effect.
    _glgsg->_glLinkProgram(_glsl_program);
  }*/

  // Create a buffer the size of the longest uniform name.  Note that Intel HD
  // drivers report values that are too low.
  name_buflen = 0;
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_buflen);
  name_buflen = max(64, name_buflen);
  name_buffer = (char *)alloca(name_buflen);

  // Get the used uniform blocks.
  if (_glgsg->_supports_uniform_buffers) {
    GLint block_count = 0, block_maxlength = 0;
    _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_BLOCKS, &block_count);

    // Intel HD drivers report GL_INVALID_ENUM here.  They reportedly fixed
    // it, but I don't know in which driver version the fix is.
    if (_glgsg->_gl_vendor != "Intel") {
      _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &block_maxlength);
      block_maxlength = max(64, block_maxlength);
    } else {
      block_maxlength = 1024;
    }

    char *block_name_cstr = (char *)alloca(block_maxlength);

    for (int i = 0; i < block_count; ++i) {
      block_name_cstr[0] = 0;
      _glgsg->_glGetActiveUniformBlockName(_glsl_program, i, block_maxlength, nullptr, block_name_cstr);

      reflect_uniform_block(i, block_name_cstr, name_buffer, name_buflen);
    }
  }

#ifndef OPENGLES
  // Get the used shader storage blocks.
  if (_glgsg->_supports_shader_buffers) {
    GLint block_count = 0, block_maxlength = 0;

    _glgsg->_glGetProgramInterfaceiv(_glsl_program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &block_count);
    _glgsg->_glGetProgramInterfaceiv(_glsl_program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &block_maxlength);

    block_maxlength = max(64, block_maxlength);
    char *block_name_cstr = (char *)alloca(block_maxlength);

    for (int i = 0; i < block_count; ++i) {
      block_name_cstr[0] = 0;
      _glgsg->_glGetProgramResourceName(_glsl_program, GL_SHADER_STORAGE_BLOCK, i, block_maxlength, nullptr, block_name_cstr);

      const GLenum props[] = {GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
      GLint values[2];
      _glgsg->_glGetProgramResourceiv(_glsl_program, GL_SHADER_STORAGE_BLOCK, i, 2, props, 2, nullptr, values);

      StorageBlock block;
      block._name = InternalName::make(block_name_cstr);
      block._binding_index = values[0];
      block._min_size = (GLuint)values[1];
      _storage_blocks.push_back(block);
    }
  }
#endif

  // Bind the program, so that we can call glUniform1i for the textures.
  _glgsg->_glUseProgram(_glsl_program);

  // Analyze the uniforms.
  param_count = 0;
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORMS, &param_count);

  _shader->_ptr_spec.clear();
  _shader->_mat_spec.clear();
  _shader->_tex_spec.clear();
  for (int i = 0; i < param_count; ++i) {
    reflect_uniform(i, name_buffer, name_buflen);
  }

  _glgsg->report_my_gl_errors();

  // Restore the active shader.
  if (_glgsg->_current_shader_context == nullptr) {
    _glgsg->_glUseProgram(0);
  } else {
    _glgsg->_current_shader_context->bind();
  }
}

/**
 * Analyzes the vertex attribute and stores the information it needs to
 * remember.
 */
void CLP(ShaderContext)::
reflect_attribute(int i, char *name_buffer, GLsizei name_buflen) {
  GLint param_size;
  GLenum param_type;

  // Get the name, size, and type of this attribute.
  name_buffer[0] = 0;
  _glgsg->_glGetActiveAttrib(_glsl_program, i, name_buflen, nullptr,
                             &param_size, &param_type, name_buffer);

  // Get the attrib location.
  GLint p = _glgsg->_glGetAttribLocation(_glsl_program, name_buffer);

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Active attribute " << name_buffer << " with size " << param_size
      << " and type 0x" << hex << param_type << dec
      << " is bound to location " << p << "\n";
  }

  if (p == -1 || strncmp(name_buffer, "gl_", 3) == 0) {
    // A gl_ attribute such as gl_Vertex requires us to pass the standard
    // vertex arrays as we would do without shader.  Not all drivers return -1
    // in glGetAttribLocation for gl_ prefixed attributes, so we check the
    // prefix of the input ourselves, just to be sure.
    _uses_standard_vertex_arrays = true;
    return;
  }

  Shader::ShaderArgId arg_id;
  arg_id._name = name_buffer;
  arg_id._seqno = p;

  Shader::ShaderVarSpec bind;
  bind._id = arg_id;
  bind._name = nullptr;
  bind._append_uv = -1;
  bind._elements = 1;

  // Check if this is an integer input- if so, we have to bind it differently.
  switch (param_type) {
  case GL_INT:
  case GL_INT_VEC2:
  case GL_INT_VEC3:
  case GL_INT_VEC4:
    bind._numeric_type = Shader::SPT_int;
    break;
  case GL_BOOL:
  case GL_BOOL_VEC2:
  case GL_BOOL_VEC3:
  case GL_BOOL_VEC4:
  case GL_UNSIGNED_INT:
  case GL_UNSIGNED_INT_VEC2:
  case GL_UNSIGNED_INT_VEC3:
  case GL_UNSIGNED_INT_VEC4:
    bind._numeric_type = Shader::SPT_uint;
    break;
#ifndef OPENGLES
  case GL_DOUBLE:
  case GL_DOUBLE_VEC2:
  case GL_DOUBLE_VEC3:
  case GL_DOUBLE_VEC4:
    bind._numeric_type = Shader::SPT_double;
    break;
#endif
  default:
    bind._numeric_type = Shader::SPT_float;
  }

  // Check if it has a p3d_ prefix - if so, assign special meaning.
  if (strncmp(name_buffer, "p3d_", 4) == 0) {
    string noprefix(name_buffer + 4);

    if (noprefix == "Vertex") {
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
      GLCAT.error() << "Unrecognized vertex attrib '" << name_buffer << "'!\n";
      return;
    }
  } else {
    // Arbitrarily named attribute.
    bind._name = InternalName::make(name_buffer);
  }

  // Get the number of bind points for arrays and matrices.
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

  // Experimental feature.
  if (gl_fixed_vertex_attrib_locations) {
    GLint loc;
    if (bind._name == InternalName::get_vertex()) {
      loc = 0;
    } else if (bind._name == InternalName::get_transform_weight()) {
      loc = 1;
    } else if (bind._name == InternalName::get_normal()) {
      loc = 2;
    } else if (bind._name == InternalName::get_color()) {
      loc = 3;
    } else if (bind._name == InternalName::get_transform_index()) {
      loc = 7;
    } else if (bind._name == InternalName::get_texcoord() &&
               bind._append_uv >= 0 && bind._append_uv < 8) {
      loc = 8 + bind._append_uv;
    } else {
      GLCAT.error()
        << "Vertex attrib '" << name_buffer << "' not yet supported with "
           "gl-fixed-vertex-attrib-locations!\n";
      return;
    }
    if (loc != p) {
      GLCAT.error()
        << "Vertex attrib '" << name_buffer << "' was bound to the wrong slot!\n";
      return;
    }
    // _glgsg->_glBindAttribLocation(_glsl_program, loc, name_buffer);
    _enabled_attribs.set_range(loc, bind._elements);
  }

  _shader->_var_spec.push_back(bind);
}

/**
 * Analyzes the uniform block and stores its format.
 */
void CLP(ShaderContext)::
reflect_uniform_block(int i, const char *name, char *name_buffer, GLsizei name_buflen) {
 // GLint offset = 0;

  GLint data_size = 0;
  GLint param_count = 0;
  _glgsg->_glGetActiveUniformBlockiv(_glsl_program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &data_size);
  _glgsg->_glGetActiveUniformBlockiv(_glsl_program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &param_count);

  if (param_count <= 0) {
    return;
  }

  // We use a GeomVertexArrayFormat to describe the uniform buffer layout.
  // GeomVertexArrayFormat block_format; block_format.set_pad_to(data_size);

  // Get an array containing the indices of all the uniforms in this block.
  GLuint *indices = (GLuint *)alloca(param_count * sizeof(GLint));
  _glgsg->_glGetActiveUniformBlockiv(_glsl_program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint *)indices);

  // Acquire information about the way the uniforms in this block are packed.
  GLint *offsets = (GLint *)alloca(param_count * sizeof(GLint));
  GLint *mstrides = (GLint *)alloca(param_count * sizeof(GLint));
  GLint *astrides = (GLint *)alloca(param_count * sizeof(GLint));
  _glgsg->_glGetActiveUniformsiv(_glsl_program, param_count, indices, GL_UNIFORM_OFFSET, offsets);
  _glgsg->_glGetActiveUniformsiv(_glsl_program, param_count, indices, GL_UNIFORM_MATRIX_STRIDE, mstrides);
  _glgsg->_glGetActiveUniformsiv(_glsl_program, param_count, indices, GL_UNIFORM_ARRAY_STRIDE, astrides);

  for (int ui = 0; ui < param_count; ++ui) {
    name_buffer[0] = 0;
    GLint param_size;
    GLenum param_type;
    _glgsg->_glGetActiveUniform(_glsl_program, indices[ui], name_buflen, nullptr, &param_size, &param_type, name_buffer);

    // Strip off [0] suffix that some drivers append to arrays.
    size_t size = strlen(name_buffer);
    if (size > 3 && strncmp(name_buffer + (size - 3), "[0]", 3) == 0) {
      name_buffer[size - 3] = 0;
    }

    GeomEnums::NumericType numeric_type;
    GeomEnums::Contents contents = GeomEnums::C_other;
    int num_components = 1;

    switch (param_type) {
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
      numeric_type = GeomEnums::NT_int32;
      break;

    case GL_BOOL:
    case GL_BOOL_VEC2:
    case GL_BOOL_VEC3:
    case GL_BOOL_VEC4:
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_VEC2:
    case GL_UNSIGNED_INT_VEC3:
    case GL_UNSIGNED_INT_VEC4:
      numeric_type = GeomEnums::NT_uint32;
      break;

    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT3:
    case GL_FLOAT_MAT4:
      numeric_type = GeomEnums::NT_float32;
      break;

#ifndef OPENGLES
    case GL_DOUBLE:
    case GL_DOUBLE_VEC2:
    case GL_DOUBLE_VEC3:
    case GL_DOUBLE_VEC4:
    case GL_DOUBLE_MAT2:
    case GL_DOUBLE_MAT3:
    case GL_DOUBLE_MAT4:
      numeric_type = GeomEnums::NT_float64;
      break;
#endif

    default:
      GLCAT.info() << "Ignoring uniform '" << name_buffer
        << "' with unsupported type 0x" << hex << param_type << dec << "\n";
      continue;
    }

    switch (param_type) {
    case GL_INT_VEC2:
    case GL_BOOL_VEC2:
    case GL_UNSIGNED_INT_VEC2:
    case GL_FLOAT_VEC2:
#ifndef OPENGLES
    case GL_DOUBLE_VEC2:
#endif
      num_components = 2;
      break;

    case GL_INT_VEC3:
    case GL_BOOL_VEC3:
    case GL_UNSIGNED_INT_VEC3:
    case GL_FLOAT_VEC3:
#ifndef OPENGLES
    case GL_DOUBLE_VEC3:
#endif
      num_components = 3;
      break;

    case GL_INT_VEC4:
    case GL_BOOL_VEC4:
    case GL_UNSIGNED_INT_VEC4:
    case GL_FLOAT_VEC4:
#ifndef OPENGLES
    case GL_DOUBLE_VEC4:
#endif
      num_components = 4;
      break;

    case GL_FLOAT_MAT3:
#ifndef OPENGLES
    case GL_DOUBLE_MAT3:
#endif
      num_components = 3;
      contents = GeomEnums::C_matrix;
      nassertd(param_size <= 1 || astrides[ui] == mstrides[ui] * 3) continue;
      param_size *= 3;
      break;

    case GL_FLOAT_MAT4:
#ifndef OPENGLES
    case GL_DOUBLE_MAT4:
#endif
      num_components = 4;
      contents = GeomEnums::C_matrix;
      nassertd(param_size <= 1 || astrides[ui] == mstrides[ui] * 4) continue;
      param_size *= 4;
      break;
    }

    (void)numeric_type;
    (void)contents;
    (void)num_components;
    // GeomVertexColumn column(InternalName::make(name_buffer),
    // num_components, numeric_type, contents, offsets[ui], 4, param_size,
    // astrides[ui]); block_format.add_column(column);
  }

  // if (GLCAT.is_debug()) { GLCAT.debug() << "Active uniform block " << name
  // << " has format:\n"; block_format.write(GLCAT.debug(false), 2); }

  // UniformBlock block; block._name = InternalName::make(name); block._format
  // = GeomVertexArrayFormat::register_format(&block_format); block._buffer =
  // 0;

  // _uniform_blocks.push_back(block);
}

/**
 * Analyzes a single uniform variable and considers how it should be handled
 * and bound.
 */
void CLP(ShaderContext)::
reflect_uniform(int i, char *name_buffer, GLsizei name_buflen) {
  GLint param_size;
  GLenum param_type;

  // Get the name, location, type and size of this uniform.
  name_buffer[0] = 0;
  _glgsg->_glGetActiveUniform(_glsl_program, i, name_buflen, nullptr, &param_size, &param_type, name_buffer);
  GLint p = _glgsg->_glGetUniformLocation(_glsl_program, name_buffer);

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Active uniform " << name_buffer << " with size " << param_size
      << " and type 0x" << hex << param_type << dec
      << " is bound to location " << p << "\n";
  }

  // Some NVidia drivers (361.43 for example) (incorrectly) include "internal"
  // uniforms in the list starting with "_main_" (for example,
  // "_main_0_gp5fp[0]") we need to skip those, because we don't know anything
  // about them
  if (strncmp(name_buffer, "_main_", 6) == 0) {
    if (GLCAT.is_debug()) {
      GLCAT.debug() << "Ignoring uniform " << name_buffer << " which may be generated by buggy Nvidia driver.\n";
    }
    return;
  }

  if (p < 0) {
    // Special meaning, or it's in a uniform block.  Let it go.
    return;
  }

  // Strip off [0] suffix that some drivers append to arrays.
  size_t size = strlen(name_buffer);
  if (size > 3 && strncmp(name_buffer + (size - 3), "[0]", 3) == 0) {
    size -= 3;
    name_buffer[size] = 0;
  }

  string param_name(name_buffer);

  Shader::ShaderArgId arg_id;
  arg_id._name = param_name;
  arg_id._seqno = p;

  // Check if it has a p3d_ prefix - if so, assign special meaning.
  if (strncmp(name_buffer, "p3d_", 4) == 0) {
    string noprefix(name_buffer + 4);

    // Check for matrix inputs.
    bool transpose = false;
    bool inverse = false;
    string matrix_name(noprefix);
    size = matrix_name.size();

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

    // Now if the suffix that is left over is "Matrix", we know that it is
    // supposed to be a matrix input.
    if (size > 6 && matrix_name.compare(size - 6, 6, "Matrix") == 0) {
      Shader::ShaderMatSpec bind;
      bind._id = arg_id;
      bind._func = Shader::SMF_compose;
      if (param_type == GL_FLOAT_MAT3) {
        if (transpose) {
          bind._piece = Shader::SMP_upper3x3;
        } else {
          bind._piece = Shader::SMP_transpose3x3;
        }
      } else if (param_type == GL_FLOAT_MAT4) {
        if (transpose) {
          bind._piece = Shader::SMP_transpose;
        } else {
          bind._piece = Shader::SMP_whole;
        }
      } else {
        GLCAT.error()
          << "Matrix input p3d_" << matrix_name << " should be mat3 or mat4\n";
        return;
      }
      bind._arg[0] = nullptr;
      bind._arg[1] = nullptr;

      if (matrix_name == "ModelViewProjectionMatrix") {
        if (inverse) {
          bind._part[0] = Shader::SMO_apiclip_to_apiview;
          bind._part[1] = Shader::SMO_apiview_to_model;
        } else {
          bind._part[0] = Shader::SMO_model_to_apiview;
          bind._part[1] = Shader::SMO_apiview_to_apiclip;
        }

      } else if (matrix_name == "ModelViewMatrix") {
        bind._func = Shader::SMF_first;
        bind._part[0] = inverse ? Shader::SMO_apiview_to_model
                                : Shader::SMO_model_to_apiview;
        bind._part[1] = Shader::SMO_identity;

      } else if (matrix_name == "ProjectionMatrix") {
        bind._func = Shader::SMF_first;
        bind._part[0] = inverse ? Shader::SMO_apiclip_to_apiview
                                : Shader::SMO_apiview_to_apiclip;
        bind._part[1] = Shader::SMO_identity;

      } else if (matrix_name == "NormalMatrix") {
        // This is really the upper 3x3 of the
        // ModelViewMatrixInverseTranspose.
        bind._func = Shader::SMF_first;
        bind._part[0] = inverse ? Shader::SMO_model_to_apiview
                                : Shader::SMO_apiview_to_model;
        bind._part[1] = Shader::SMO_identity;

        if (param_type != GL_FLOAT_MAT3) {
          GLCAT.warning() << "p3d_NormalMatrix input should be mat3, not mat4!\n";
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

      } else if (matrix_name == "TextureMatrix") {
        // We may support 2-D texmats later, but let's make sure that people
        // don't think they can just use a mat3 to get the 2-D version.
        if (param_type != GL_FLOAT_MAT4) {
          GLCAT.error() << "p3d_TextureMatrix should be mat4[], not mat3[]!\n";
          return;
        }

        bind._func = Shader::SMF_first;
        bind._part[0] = inverse ? Shader::SMO_inv_texmat_i
                                : Shader::SMO_texmat_i;
        bind._part[1] = Shader::SMO_identity;
        bind._dep[0] = Shader::SSD_general | Shader::SSD_tex_matrix;
        bind._dep[1] = 0;

        // Add it once for each index.
        for (bind._index = 0; bind._index < param_size; ++bind._index) {
          bind._id._seqno = p + bind._index;
          _shader->_mat_spec.push_back(bind);
        }
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (matrix_name.size() > 15 &&
                 matrix_name.substr(0, 12) == "LightSource[" &&
                 sscanf(matrix_name.c_str(), "LightSource[%d].%s", &bind._index, name_buffer) == 2) {
        // A matrix member of a p3d_LightSource struct.
        if (strncmp(name_buffer, "shadowViewMatrix", 127) == 0) {
          if (inverse) {
            // Tack inverse back onto the end.
            strcpy(name_buffer + strlen(name_buffer), "Inverse");
          }

          bind._func = Shader::SMF_first;
          bind._part[0] = Shader::SMO_light_source_i_attrib;
          bind._arg[0] = InternalName::make(name_buffer);
          bind._part[1] = Shader::SMO_identity;
          bind._arg[1] = nullptr;

        } else if (strncmp(name_buffer, "shadowMatrix", 127) == 0) {
          // Only supported for backward compatibility: includes the model
          // matrix.  Not very efficient to do this.
          bind._func = Shader::SMF_compose;
          bind._part[0] = Shader::SMO_model_to_apiview;
          bind._arg[0] = nullptr;
          bind._part[1] = Shader::SMO_light_source_i_attrib;
          bind._arg[1] = InternalName::make("shadowViewMatrix");

          static bool warned = false;
          if (!warned) {
            warned = true;
            GLCAT.warning()
              << "p3d_LightSource[].shadowMatrix is deprecated; use "
                "shadowViewMatrix instead, which transforms from view space "
                "instead of model space.\n";
          }
        } else {
          GLCAT.error() << "p3d_LightSource struct does not provide a matrix named " << matrix_name << "!\n";
          return;
        }

      } else {
        GLCAT.error() << "Unrecognized uniform matrix name '" << matrix_name << "'!\n";
        return;
      }
      _shader->cp_optimize_mat_spec(bind);
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0] | bind._dep[1];
      return;
    }
    if (size > 7 && noprefix.substr(0, 7) == "Texture") {
      Shader::ShaderTexSpec bind;
      bind._id = arg_id;
      bind._part = Shader::STO_stage_i;
      bind._name = 0;

      string tail;
      bind._stage = string_to_int(noprefix.substr(7), tail);
      if (!tail.empty()) {
        GLCAT.error()
          << "Error parsing shader input name: unexpected '"
          << tail << "' in '" << param_name << "'\n";
        return;
      }

      if (get_sampler_texture_type(bind._desired_type, param_type)) {
        _glgsg->_glUniform1i(p, _shader->_tex_spec.size());
        _shader->_tex_spec.push_back(bind);
      } else {
        GLCAT.error()
          << "Could not bind texture input " << param_name << "\n";
      }
      return;
    }
    if (size > 9 && noprefix.substr(0, 9) == "Material.") {
      Shader::ShaderMatSpec bind;
      bind._id = arg_id;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_attr_material;
      bind._arg[0] = nullptr;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_material | Shader::SSD_frame;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;
      bind._dep[1] = Shader::SSD_NONE;

      if (noprefix == "Material.baseColor") {
        if (param_type != GL_FLOAT_VEC4) {
          GLCAT.error()
            << "p3d_Material.baseColor should be vec4\n";
        }
        bind._part[0] = Shader::SMO_attr_material2;
        bind._piece = Shader::SMP_row0;
        bind._dep[0] |= Shader::SSD_color;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.ambient") {
        if (param_type != GL_FLOAT_VEC4) {
          GLCAT.error()
            << "p3d_Material.ambient should be vec4\n";
        }
        bind._piece = Shader::SMP_row0;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.diffuse") {
        if (param_type != GL_FLOAT_VEC4) {
          GLCAT.error()
            << "p3d_Material.diffuse should be vec4\n";
        }
        bind._piece = Shader::SMP_row1;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.emission") {
        if (param_type != GL_FLOAT_VEC4) {
          GLCAT.error()
            << "p3d_Material.emission should be vec4\n";
        }
        bind._piece = Shader::SMP_row2;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.specular") {
        if (param_type != GL_FLOAT_VEC3) {
          GLCAT.error()
            << "p3d_Material.specular should be vec3\n";
        }
        bind._piece = Shader::SMP_row3x3;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.shininess") {
        if (param_type != GL_FLOAT) {
          GLCAT.error()
            << "p3d_Material.shininess should be float\n";
        }
        bind._piece = Shader::SMP_cell15;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.roughness") {
        if (param_type != GL_FLOAT) {
          GLCAT.error()
            << "p3d_Material.roughness should be float\n";
        }
        bind._part[0] = Shader::SMO_attr_material2;
        bind._piece = Shader::SMP_cell15;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.metallic") {
        if (param_type != GL_FLOAT && param_type != GL_BOOL) {
          GLCAT.error()
            << "p3d_Material.metallic should be bool or float\n";
        }
        bind._part[0] = Shader::SMO_attr_material2;
        bind._piece = Shader::SMP_row3x1;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;

      } else if (noprefix == "Material.refractiveIndex") {
        if (param_type != GL_FLOAT) {
          GLCAT.error()
            << "p3d_Material.refractiveIndex should be float\n";
        }
        bind._part[0] = Shader::SMO_attr_material2;
        bind._piece = Shader::SMP_cell13;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;
      }
    }
    if (noprefix == "ColorScale") {
      Shader::ShaderMatSpec bind;
      bind._id = arg_id;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_attr_colorscale;
      bind._arg[0] = nullptr;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_colorscale;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;
      bind._dep[1] = Shader::SSD_NONE;

      if (param_type == GL_FLOAT_VEC3) {
        bind._piece = Shader::SMP_row3x3;
      } else if (param_type == GL_FLOAT_VEC4) {
        bind._piece = Shader::SMP_row3;
      } else {
        GLCAT.error()
          << "p3d_ColorScale should be vec3 or vec4\n";
        return;
      }
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0];
      return;
    }
    if (noprefix == "Color") {
      Shader::ShaderMatSpec bind;
      bind._id = arg_id;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_attr_color;
      bind._arg[0] = nullptr;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_color;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;
      bind._dep[1] = Shader::SSD_NONE;

      if (param_type == GL_FLOAT_VEC3) {
        bind._piece = Shader::SMP_row3x3;
      } else if (param_type == GL_FLOAT_VEC4) {
        bind._piece = Shader::SMP_row3;
      } else {
        GLCAT.error()
          << "p3d_Color should be vec3 or vec4\n";
        return;
      }
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0];
      return;
    }
    if (noprefix == "ClipPlane") {
      if (param_type != GL_FLOAT_VEC4) {
        GLCAT.error()
          << "p3d_ClipPlane should be vec4 or vec4[]\n";
        return;
      }
      for (int i = 0; i < param_size; ++i) {
        Shader::ShaderMatSpec bind;
        bind._id = arg_id;
        bind._id._seqno = p + i;
        bind._piece = Shader::SMP_row3;
        bind._func = Shader::SMF_first;
        bind._index = i;
        bind._part[0] = Shader::SMO_apiview_clipplane_i;
        bind._arg[0] = nullptr;
        bind._dep[0] = Shader::SSD_general | Shader::SSD_clip_planes;
        bind._part[1] = Shader::SMO_identity;
        bind._arg[1] = nullptr;
        bind._dep[1] = Shader::SSD_NONE;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
      }
      return;
    }
    if (size > 4 && noprefix.substr(0, 4) == "Fog.") {
      Shader::ShaderMatSpec bind;
      bind._id = arg_id;
      bind._func = Shader::SMF_first;
      bind._arg[0] = nullptr;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_fog;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;
      bind._dep[1] = Shader::SSD_NONE;

      if (noprefix == "Fog.color") {
        bind._part[0] = Shader::SMO_attr_fogcolor;

        if (param_type == GL_FLOAT_VEC3) {
          bind._piece = Shader::SMP_row3x3;
        } else if (param_type == GL_FLOAT_VEC4) {
          bind._piece = Shader::SMP_row3;
        } else {
          GLCAT.error()
            << "p3d_Fog.color should be vec3 or vec4\n";
          return;
        }

      } else if (noprefix == "Fog.density") {
        bind._part[0] = Shader::SMO_attr_fog;

        if (param_type == GL_FLOAT) {
          bind._piece = Shader::SMP_row3x1;
        } else {
          GLCAT.error()
            << "p3d_Fog.density should be float\n";
          return;
        }

      } else if (noprefix == "Fog.start") {
        bind._part[0] = Shader::SMO_attr_fog;

        if (param_type == GL_FLOAT) {
          bind._piece = Shader::SMP_cell13;
        } else {
          GLCAT.error()
            << "p3d_Fog.start should be float\n";
          return;
        }

      } else if (noprefix == "Fog.end") {
        bind._part[0] = Shader::SMO_attr_fog;

        if (param_type == GL_FLOAT) {
          bind._piece = Shader::SMP_cell14;
        } else {
          GLCAT.error()
            << "p3d_Fog.end should be float\n";
          return;
        }

      } else if (noprefix == "Fog.scale") {
        bind._part[0] = Shader::SMO_attr_fog;

        if (param_type == GL_FLOAT) {
          bind._piece = Shader::SMP_cell15;
        } else {
          GLCAT.error()
            << "p3d_Fog.scale should be float\n";
          return;
        }
      }

      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0];
      return;
    }
    if (noprefix == "LightModel.ambient") {
      Shader::ShaderMatSpec bind;
      bind._id = arg_id;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_light_ambient;
      bind._arg[0] = nullptr;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_light;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;
      bind._dep[1] = Shader::SSD_NONE;

      if (param_type == GL_FLOAT_VEC3) {
        bind._piece = Shader::SMP_row3x3;
      } else if (param_type == GL_FLOAT_VEC4) {
        bind._piece = Shader::SMP_row3;
      } else {
        GLCAT.error()
          << "p3d_LightModel.ambient should be vec3 or vec4\n";
        return;
      }
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0];
      return;
    }
    if (size > 15 && noprefix.substr(0, 12) == "LightSource[") {
      int index;
      if (sscanf(noprefix.c_str(), "LightSource[%d].%s", &index, name_buffer) == 2) {
        // A member of a p3d_LightSource struct.
        string member_name(name_buffer);
        if (member_name == "shadowMap") {
          switch (param_type) {
          case GL_SAMPLER_CUBE_SHADOW:
          case GL_SAMPLER_2D:
          case GL_SAMPLER_2D_SHADOW:
          case GL_SAMPLER_CUBE:
            {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._part = Shader::STO_light_i_shadow_map;
              bind._name = 0;
              bind._desired_type = Texture::TT_2d_texture;
              bind._stage = index;
              if (get_sampler_texture_type(bind._desired_type, param_type)) {
                _glgsg->_glUniform1i(p, _shader->_tex_spec.size());
                _shader->_tex_spec.push_back(bind);
              }
              return;
            }
          default:
            GLCAT.error()
              << "Invalid type for p3d_LightSource[].shadowMap input!\n";
            return;
          }
        } else {
          // A non-sampler attribute of a numbered light source.
          Shader::ShaderMatSpec bind;
          bind._id = arg_id;
          bind._func = Shader::SMF_first;
          bind._index = index;
          bind._part[0] = Shader::SMO_light_source_i_attrib;
          bind._arg[0] = InternalName::make(member_name);
          bind._dep[0] = Shader::SSD_general | Shader::SSD_light | Shader::SSD_frame;
          bind._part[1] = Shader::SMO_identity;
          bind._arg[1] = nullptr;
          bind._dep[1] = Shader::SSD_NONE;

          if (member_name == "position" || member_name == "halfVector" ||
              member_name == "spotDirection") {
            bind._dep[0] |= Shader::SSD_view_transform;
          }

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

          case GL_FLOAT_VEC4:
            bind._piece = Shader::SMP_row3;
            break;

          default:
            GLCAT.error()
              << "p3d_LightSource[]." << member_name << " should be float or vec\n";
            return;
          }
          _shader->_mat_spec.push_back(bind);
          _shader->_mat_deps |= bind._dep[0] | bind._dep[1];
          return;
        }
      }
    }
    if (noprefix == "TransformTable") {
      if (param_type != GL_FLOAT_MAT4) {
        GLCAT.error()
          << "p3d_TransformTable should be uniform mat4[]\n";
        return;
      }
      _transform_table_index = p;
      _transform_table_size = param_size;
      return;
    }
    if (noprefix == "SliderTable") {
      if (param_type != GL_FLOAT) {
        GLCAT.error()
          << "p3d_SliderTable should be uniform float[]\n";
        return;
      }
      _slider_table_index = p;
      _slider_table_size = param_size;
      return;
    }
    if (noprefix == "TexAlphaOnly") {
      Shader::ShaderMatSpec bind;
      bind._id = arg_id;
      bind._func = Shader::SMF_first;
      bind._index = 0;
      bind._part[0] = Shader::SMO_tex_is_alpha_i;
      bind._arg[0] = nullptr;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_texture | Shader::SSD_frame;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;
      bind._dep[1] = Shader::SSD_NONE;
      bind._piece = Shader::SMP_row3;
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0] | bind._dep[1];
      return;
    }
    GLCAT.error() << "Unrecognized uniform name '" << param_name << "'!\n";
    return;

  } else if (strncmp(name_buffer, "osg_", 4) == 0) {
    string noprefix(name_buffer + 4);
    // These inputs are supported by OpenSceneGraph.  We can support them as
    // well, to increase compatibility.

    Shader::ShaderMatSpec bind;
    bind._id = arg_id;
    bind._arg[0] = nullptr;
    bind._arg[1] = nullptr;

    if (noprefix == "ViewMatrix") {
      bind._piece = Shader::SMP_whole;
      bind._func = Shader::SMF_compose;
      bind._part[0] = Shader::SMO_world_to_view;
      bind._part[1] = Shader::SMO_view_to_apiview;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_view_transform;
      bind._dep[1] = Shader::SSD_general;
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0] | bind._dep[1];
      return;

    } else if (noprefix == "InverseViewMatrix" || noprefix == "ViewMatrixInverse") {
      bind._piece = Shader::SMP_whole;
      bind._func = Shader::SMF_compose;
      bind._part[0] = Shader::SMO_apiview_to_view;
      bind._part[1] = Shader::SMO_view_to_world;
      bind._dep[0] = Shader::SSD_general;
      bind._dep[1] = Shader::SSD_general | Shader::SSD_view_transform;
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0] | bind._dep[1];
      return;

    } else if (noprefix == "FrameTime") {
      bind._piece = Shader::SMP_row3x1;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_frame_time;
      bind._part[1] = Shader::SMO_identity;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_frame;
      bind._dep[1] = Shader::SSD_NONE;
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0] | bind._dep[1];
      return;

    } else if (noprefix == "DeltaFrameTime") {
      bind._piece = Shader::SMP_row3x1;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_frame_delta;
      bind._part[1] = Shader::SMO_identity;
      bind._dep[0] = Shader::SSD_general | Shader::SSD_frame;
      bind._dep[1] = Shader::SSD_NONE;
      _shader->_mat_spec.push_back(bind);
      _shader->_mat_deps |= bind._dep[0] | bind._dep[1];
      return;

    } else if (noprefix == "FrameNumber") {
      // We don't currently support ints with this mechanism, so we special-
      // case this one.
      if (param_type != GL_INT) {
        GLCAT.error() << "osg_FrameNumber should be uniform int\n";
      } else {
        _frame_number_loc = p;
      }
      return;
    }

  } else {
    // Tries to parse shorthand notations like mspos_XXX and
    // trans_model_to_clip_of_XXX
    if (parse_and_set_short_hand_shader_vars(arg_id, param_type, param_size, _shader)) {
      return;
    }
  }

  if (param_size == 1) {
    // A single uniform (not an array, or an array of size 1).
    switch (param_type) {
      case GL_INT_SAMPLER_2D:
      case GL_INT_SAMPLER_3D:
      case GL_INT_SAMPLER_2D_ARRAY:
      case GL_INT_SAMPLER_CUBE:
      case GL_UNSIGNED_INT_SAMPLER_2D:
      case GL_UNSIGNED_INT_SAMPLER_3D:
      case GL_UNSIGNED_INT_SAMPLER_CUBE:
      case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
      case GL_SAMPLER_CUBE_SHADOW:
      case GL_SAMPLER_2D_ARRAY:
      case GL_SAMPLER_2D_ARRAY_SHADOW:
#ifndef OPENGLES
      case GL_INT_SAMPLER_1D:
      case GL_INT_SAMPLER_1D_ARRAY:
      case GL_INT_SAMPLER_BUFFER:
      case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
      case GL_UNSIGNED_INT_SAMPLER_1D:
      case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
      case GL_UNSIGNED_INT_SAMPLER_BUFFER:
      case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
      case GL_SAMPLER_1D:
      case GL_SAMPLER_1D_ARRAY:
      case GL_SAMPLER_1D_SHADOW:
      case GL_SAMPLER_BUFFER:
      case GL_SAMPLER_CUBE_MAP_ARRAY:
      case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
#endif  // !OPENGLES
      case GL_SAMPLER_2D:
      case GL_SAMPLER_2D_SHADOW:
      case GL_SAMPLER_3D:
      case GL_SAMPLER_CUBE: {
        Shader::ShaderTexSpec bind;
        bind._id = arg_id;
        bind._part = Shader::STO_named_input;
        bind._name = InternalName::make(param_name);
        bind._desired_type = Texture::TT_2d_texture;
        bind._stage = 0;
        if (get_sampler_texture_type(bind._desired_type, param_type)) {
          _glgsg->_glUniform1i(p, _shader->_tex_spec.size());
          _shader->_tex_spec.push_back(bind);
        }
        return;
      }
      case GL_FLOAT_MAT2:
      case GL_FLOAT_MAT2x3:
      case GL_FLOAT_MAT2x4:
      case GL_FLOAT_MAT3x2:
      case GL_FLOAT_MAT3x4:
      case GL_FLOAT_MAT4x2:
      case GL_FLOAT_MAT4x3:
        GLCAT.warning() << "GLSL shader requested an unsupported matrix type\n";
        return;
      case GL_FLOAT_MAT3: {
        Shader::ShaderMatSpec bind;
        bind._id = arg_id;
        bind._piece = Shader::SMP_upper3x3;
        bind._func = Shader::SMF_first;
        bind._part[0] = Shader::SMO_mat_constant_x;
        bind._arg[0] = InternalName::make(param_name);
        bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs | Shader::SSD_frame;
        bind._part[1] = Shader::SMO_identity;
        bind._arg[1] = nullptr;
        bind._dep[1] = Shader::SSD_NONE;
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;
      }
      case GL_FLOAT_MAT4: {
        Shader::ShaderMatSpec bind;
        bind._id = arg_id;
        bind._piece = Shader::SMP_whole;
        bind._func = Shader::SMF_first;
        bind._part[1] = Shader::SMO_identity;
        bind._arg[1] = nullptr;
        bind._dep[1] = Shader::SSD_NONE;
        PT(InternalName) iname = InternalName::make(param_name);
        if (iname->get_parent() != InternalName::get_root()) {
          // It might be something like an attribute of a shader input, like a
          // light parameter.  It might also just be a custom struct
          // parameter.  We can't know yet, sadly.
          if (iname->get_basename() == "shadowMatrix") {
            // Special exception for shadowMatrix, which is deprecated,
            // because it includes the model transformation.  It is far more
            // efficient to do that in the shader instead.
            static bool warned = false;
            if (!warned) {
              warned = true;
              GLCAT.warning()
                << "light.shadowMatrix inputs are deprecated; use "
                   "shadowViewMatrix instead, which transforms from view "
                   "space instead of model space.\n";
            }
            bind._func = Shader::SMF_compose;
            bind._part[0] = Shader::SMO_model_to_apiview;
            bind._arg[0] = nullptr;
            bind._dep[0] = Shader::SSD_general | Shader::SSD_transform;
            bind._part[1] = Shader::SMO_mat_constant_x_attrib;
            bind._arg[1] = iname->get_parent()->append("shadowViewMatrix");
            bind._dep[1] = Shader::SSD_general | Shader::SSD_shaderinputs | Shader::SSD_frame | Shader::SSD_view_transform;
          } else {
            bind._part[0] = Shader::SMO_mat_constant_x_attrib;
            bind._arg[0] = InternalName::make(param_name);
            bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs | Shader::SSD_frame | Shader::SSD_view_transform;
          }
        } else {
          bind._part[0] = Shader::SMO_mat_constant_x;
          bind._arg[0] = InternalName::make(param_name);
          bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs | Shader::SSD_frame;
        }
        _shader->_mat_spec.push_back(bind);
        _shader->_mat_deps |= bind._dep[0];
        return;
      }
      case GL_FLOAT:
      case GL_FLOAT_VEC2:
      case GL_FLOAT_VEC3:
      case GL_FLOAT_VEC4: {
        PT(InternalName) iname = InternalName::make(param_name);
        if (iname->get_parent() != InternalName::get_root()) {
          // It might be something like an attribute of a shader input, like a
          // light parameter.  It might also just be a custom struct
          // parameter.  We can't know yet, sadly.
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
          // We need SSD_view_transform since some attributes (eg. light
          // position) have to be transformed to view space.
          bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs | Shader::SSD_frame | Shader::SSD_view_transform;
          bind._part[1] = Shader::SMO_identity;
          bind._arg[1] = nullptr;
          bind._dep[1] = Shader::SSD_NONE;
          _shader->_mat_spec.push_back(bind);
          _shader->_mat_deps |= bind._dep[0];
          return;
        } // else fall through
      }
      case GL_BOOL:
      case GL_BOOL_VEC2:
      case GL_BOOL_VEC3:
      case GL_BOOL_VEC4:
      case GL_INT:
      case GL_INT_VEC2:
      case GL_INT_VEC3:
      case GL_INT_VEC4:
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_INT_VEC2:
      case GL_UNSIGNED_INT_VEC3:
      case GL_UNSIGNED_INT_VEC4: {
        Shader::ShaderPtrSpec bind;
        bind._id = arg_id;
        switch (param_type) {
          case GL_BOOL:
          case GL_INT:
          case GL_UNSIGNED_INT:
          case GL_FLOAT:      bind._dim[1] = 1; break;
          case GL_BOOL_VEC2:
          case GL_INT_VEC2:
          case GL_UNSIGNED_INT_VEC2:
          case GL_FLOAT_VEC2: bind._dim[1] = 2; break;
          case GL_BOOL_VEC3:
          case GL_INT_VEC3:
          case GL_UNSIGNED_INT_VEC3:
          case GL_FLOAT_VEC3: bind._dim[1] = 3; break;
          case GL_BOOL_VEC4:
          case GL_INT_VEC4:
          case GL_UNSIGNED_INT_VEC4:
          case GL_FLOAT_VEC4: bind._dim[1] = 4; break;
          case GL_FLOAT_MAT3: bind._dim[1] = 9; break;
          case GL_FLOAT_MAT4: bind._dim[1] = 16; break;
        }
        switch (param_type) {
        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_VEC2:
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
          bind._type = Shader::SPT_uint;
          break;
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
        bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs | Shader::SSD_frame;
        bind._dep[1] = Shader::SSD_NONE;
        _shader->_ptr_spec.push_back(bind);
        return;
      }
      case GL_IMAGE_2D:
      case GL_IMAGE_3D:
      case GL_IMAGE_CUBE:
      case GL_IMAGE_2D_ARRAY:
      case GL_INT_IMAGE_2D:
      case GL_INT_IMAGE_3D:
      case GL_INT_IMAGE_CUBE:
      case GL_INT_IMAGE_2D_ARRAY:
      case GL_UNSIGNED_INT_IMAGE_2D:
      case GL_UNSIGNED_INT_IMAGE_3D:
      case GL_UNSIGNED_INT_IMAGE_CUBE:
      case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
#ifndef OPENGLES
      case GL_IMAGE_1D:
      case GL_IMAGE_CUBE_MAP_ARRAY:
      case GL_IMAGE_BUFFER:
      case GL_INT_IMAGE_1D:
      case GL_INT_IMAGE_CUBE_MAP_ARRAY:
      case GL_INT_IMAGE_BUFFER:
      case GL_UNSIGNED_INT_IMAGE_1D:
      case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
      case GL_UNSIGNED_INT_IMAGE_BUFFER:
#endif
        // This won't really change at runtime, so we might as well bind once
        // and then forget about it.
        _glgsg->_glUniform1i(p, _glsl_img_inputs.size());
        {
          ImageInput input;
          input._name = InternalName::make(param_name);
          input._writable = false;
          input._gtc = nullptr;
          _glsl_img_inputs.push_back(input);
        }
        return;
      default:
        GLCAT.warning() << "Ignoring unrecognized GLSL parameter type!\n";
    }
  } else {
    // A uniform array.
    switch (param_type) {
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT2x3:
    case GL_FLOAT_MAT2x4:
    case GL_FLOAT_MAT3x2:
    case GL_FLOAT_MAT3x4:
    case GL_FLOAT_MAT4x2:
    case GL_FLOAT_MAT4x3:
      GLCAT.warning() << "GLSL shader requested an unrecognized matrix array type\n";
      return;
    case GL_BOOL:
    case GL_BOOL_VEC2:
    case GL_BOOL_VEC3:
    case GL_BOOL_VEC4:
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_VEC2:
    case GL_UNSIGNED_INT_VEC3:
    case GL_UNSIGNED_INT_VEC4:
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
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_INT_VEC2:
      case GL_UNSIGNED_INT_VEC3:
      case GL_UNSIGNED_INT_VEC4:
        bind._type = Shader::SPT_uint;
        break;
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
      bind._dep[0] = Shader::SSD_general | Shader::SSD_shaderinputs | Shader::SSD_frame;
      bind._dep[1] = Shader::SSD_NONE;
      _shader->_ptr_spec.push_back(bind);
      return;
    }
    default:
      GLCAT.warning() << "Ignoring unrecognized GLSL parameter array type!\n";
    }
  }
}

/**
 * Returns the texture type required for the given GL sampler type.  Returns
 * false if unsupported.
 */
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

  case GL_INT_SAMPLER_1D_ARRAY:
  case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
  case GL_SAMPLER_1D_ARRAY:
    out = Texture::TT_1d_texture_array;
    return true;
#endif

  case GL_INT_SAMPLER_2D:
  case GL_UNSIGNED_INT_SAMPLER_2D:
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

  case GL_INT_SAMPLER_3D:
  case GL_UNSIGNED_INT_SAMPLER_3D:
  case GL_SAMPLER_3D:
    out = Texture::TT_3d_texture;
    if (_glgsg->_supports_3d_texture) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses 3D texture, which is unsupported by the driver.\n";
      return false;
    }

  case GL_SAMPLER_CUBE_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_CUBE:
  case GL_UNSIGNED_INT_SAMPLER_CUBE:
  case GL_SAMPLER_CUBE:
    out = Texture::TT_cube_map;
    if (!_glgsg->_supports_cube_map) {
      GLCAT.error()
        << "GLSL shader uses cube map, which is unsupported by the driver.\n";
      return false;
    }
    return true;

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

#ifndef OPENGLES
  case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
  case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
  case GL_SAMPLER_CUBE_MAP_ARRAY:
    out = Texture::TT_cube_map_array;
    if (_glgsg->_supports_cube_map_array) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses cube map array, which is unsupported by the driver.\n";
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
#endif  // !OPENGLES

  default:
    GLCAT.error()
      << "GLSL shader uses unsupported sampler type for texture input.\n";
    return false;
  }
}

/**
 * xyz
 */
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  // Don't call release_resources; we may not have an active context.
}

/**
 * Should deallocate all system resources (such as vertex program handles or
 * Cg contexts).
 */
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

/**
 * Returns true if the shader is "valid", ie, if the compilation was
 * successful.  The compilation could fail if there is a syntax error in the
 * shader, or if the current video card isn't shader-capable, or if no shader
 * languages are compiled into panda.
 */
bool CLP(ShaderContext)::
valid() {
  if (_shader->get_error_flag()) {
    return false;
  }
  return (_glsl_program != 0);
}

/**
 * This function is to be called to enable a new shader.  It also initializes
 * all of the shader's input parameters.
 */
void CLP(ShaderContext)::
bind() {
  if (!_validated) {
    _glgsg->_glValidateProgram(_glsl_program);
    glsl_report_program_errors(_glsl_program, false);
    _validated = true;
  }

  if (!_shader->get_error_flag()) {
    _glgsg->_glUseProgram(_glsl_program);
  }

  if (GLCAT.is_spam()) {
    GLCAT.spam() << "glUseProgram(" << _glsl_program << "): "
                 << _shader->get_filename() << "\n";
  }

  _glgsg->report_my_gl_errors();
}

/**
 * This function disables a currently-bound shader.
 */
void CLP(ShaderContext)::
unbind() {
  if (GLCAT.is_spam()) {
    GLCAT.spam() << "glUseProgram(0)\n";
  }

  _glgsg->_glUseProgram(0);
  _glgsg->report_my_gl_errors();
}

/**
 * This function gets called whenever the RenderState or TransformState has
 * changed, but the Shader itself has not changed.  It loads new values into
 * the shader's parameters.
 */
void CLP(ShaderContext)::
set_state_and_transform(const RenderState *target_rs,
                        const TransformState *modelview_transform,
                        const TransformState *camera_transform,
                        const TransformState *projection_transform) {

  // Find out which state properties have changed.
  int altered = 0;

  if (_modelview_transform != modelview_transform) {
    _modelview_transform = modelview_transform;
    altered |= (Shader::SSD_transform & ~Shader::SSD_view_transform);
  }
  if (_camera_transform != camera_transform) {
    _camera_transform = camera_transform;
    altered |= Shader::SSD_transform;
  }
  if (_projection_transform != projection_transform) {
    _projection_transform = projection_transform;
    altered |= Shader::SSD_projection;
  }

  CPT(RenderState) state_rs = _state_rs.lock();
  if (state_rs == nullptr) {
    // Reset all of the state.
    altered |= Shader::SSD_general;
    _state_rs = target_rs;
    target_rs->get_attrib_def(_color_attrib);

  } else if (state_rs != target_rs) {
    // The state has changed since last time.
    if (state_rs->get_attrib(ColorAttrib::get_class_slot()) !=
        target_rs->get_attrib(ColorAttrib::get_class_slot())) {
      altered |= Shader::SSD_color;
      target_rs->get_attrib_def(_color_attrib);
    }
    if (state_rs->get_attrib(ColorScaleAttrib::get_class_slot()) !=
        target_rs->get_attrib(ColorScaleAttrib::get_class_slot())) {
      altered |= Shader::SSD_colorscale;
    }
    if (state_rs->get_attrib(MaterialAttrib::get_class_slot()) !=
        target_rs->get_attrib(MaterialAttrib::get_class_slot())) {
      altered |= Shader::SSD_material;
    }
    if (state_rs->get_attrib(FogAttrib::get_class_slot()) !=
        target_rs->get_attrib(FogAttrib::get_class_slot())) {
      altered |= Shader::SSD_fog;
    }
    if (state_rs->get_attrib(LightAttrib::get_class_slot()) !=
        target_rs->get_attrib(LightAttrib::get_class_slot())) {
      altered |= Shader::SSD_light;
    }
    if (state_rs->get_attrib(ClipPlaneAttrib::get_class_slot()) !=
        target_rs->get_attrib(ClipPlaneAttrib::get_class_slot())) {
      altered |= Shader::SSD_clip_planes;
    }
    if (state_rs->get_attrib(TexMatrixAttrib::get_class_slot()) !=
        target_rs->get_attrib(TexMatrixAttrib::get_class_slot())) {
      altered |= Shader::SSD_tex_matrix;
    }
    if (state_rs->get_attrib(TextureAttrib::get_class_slot()) !=
        target_rs->get_attrib(TextureAttrib::get_class_slot())) {
      altered |= Shader::SSD_texture;
    }
    _state_rs = target_rs;
  }

  if (_shader_attrib.get_orig() != _glgsg->_target_shader || _shader_attrib.was_deleted()) {
    altered |= Shader::SSD_shaderinputs;
    _shader_attrib = _glgsg->_target_shader;
  }

  // Is this the first time this shader is used this frame?
  int frame_number = ClockObject::get_global_clock()->get_frame_count();
  if (frame_number != _frame_number) {
     altered |= Shader::SSD_frame;
    _frame_number = frame_number;
  }

  if (altered != 0) {
    issue_parameters(altered);
  }
}

/**
 * This function gets called whenever the RenderState or TransformState has
 * changed, but the Shader itself has not changed.  It loads new values into
 * the shader's parameters.
 */
void CLP(ShaderContext)::
issue_parameters(int altered) {
  PStatGPUTimer timer(_glgsg, _glgsg->_draw_set_state_shader_parameters_pcollector);

  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << "Setting uniforms for " << _shader->get_filename()
      << " (altered 0x" << hex << altered << dec << ")\n";
  }

  // We have no way to track modifications to PTAs, so we assume that they are
  // modified every frame and when we switch ShaderAttribs.
  if (altered & (Shader::SSD_shaderinputs | Shader::SSD_frame)) {

    // If we have an osg_FrameNumber input, set it now.
    if ((altered & Shader::SSD_frame) != 0 && _frame_number_loc >= 0) {
      _glgsg->_glUniform1i(_frame_number_loc, _frame_number);
    }

    // Iterate through _ptr parameters
    for (int i = 0; i < (int)_shader->_ptr_spec.size(); ++i) {
      Shader::ShaderPtrSpec &spec = _shader->_ptr_spec[i];

      Shader::ShaderPtrData ptr_data;
      if (!_glgsg->fetch_ptr_parameter(spec, ptr_data)) { //the input is not contained in ShaderPtrData
        release_resources();
        return;
      }

      nassertd(spec._dim[1] > 0) continue;

      GLint p = spec._id._seqno;
      int array_size = min(spec._dim[0], (int)ptr_data._size / spec._dim[1]);
      switch (spec._type) {
      case Shader::SPT_float:
        {
          float *data = nullptr;

          switch (ptr_data._type) {
          case Shader::SPT_int:
            // Convert int data to float data.
            data = (float*) alloca(sizeof(float) * array_size * spec._dim[1]);
            for (int i = 0; i < (array_size * spec._dim[1]); ++i) {
              data[i] = (float)(((int*)ptr_data._ptr)[i]);
            }
            break;

          case Shader::SPT_uint:
            // Convert unsigned int data to float data.
            data = (float*) alloca(sizeof(float) * array_size * spec._dim[1]);
            for (int i = 0; i < (array_size * spec._dim[1]); ++i) {
              data[i] = (float)(((unsigned int*)ptr_data._ptr)[i]);
            }
            break;

          case Shader::SPT_double:
            // Downgrade double data to float data.
            data = (float*) alloca(sizeof(float) * array_size * spec._dim[1]);
            for (int i = 0; i < (array_size * spec._dim[1]); ++i) {
              data[i] = (float)(((double*)ptr_data._ptr)[i]);
            }
            break;

          case Shader::SPT_float:
            data = (float*)ptr_data._ptr;
            break;

          default:
            nassertd(false) continue;
          }

          switch (spec._dim[1]) {
          case 1: _glgsg->_glUniform1fv(p, array_size, (float*)data); continue;
          case 2: _glgsg->_glUniform2fv(p, array_size, (float*)data); continue;
          case 3: _glgsg->_glUniform3fv(p, array_size, (float*)data); continue;
          case 4: _glgsg->_glUniform4fv(p, array_size, (float*)data); continue;
          case 9: _glgsg->_glUniformMatrix3fv(p, array_size, GL_FALSE, (float*)data); continue;
          case 16: _glgsg->_glUniformMatrix4fv(p, array_size, GL_FALSE, (float*)data); continue;
          }
          nassertd(false) continue;
        }
        break;

      case Shader::SPT_int:
        if (ptr_data._type != Shader::SPT_int &&
            ptr_data._type != Shader::SPT_uint) {
          GLCAT.error()
            << "Cannot pass floating-point data to integer shader input '" << spec._id._name << "'\n";

          // Deactivate it to make sure the user doesn't get flooded with this
          // error.
          spec._dep[0] = 0;
          spec._dep[1] = 0;

        } else {
          switch (spec._dim[1]) {
          case 1: _glgsg->_glUniform1iv(p, array_size, (int*)ptr_data._ptr); continue;
          case 2: _glgsg->_glUniform2iv(p, array_size, (int*)ptr_data._ptr); continue;
          case 3: _glgsg->_glUniform3iv(p, array_size, (int*)ptr_data._ptr); continue;
          case 4: _glgsg->_glUniform4iv(p, array_size, (int*)ptr_data._ptr); continue;
          }
          nassertd(false) continue;
        }
        break;

      case Shader::SPT_uint:
        if (ptr_data._type != Shader::SPT_uint &&
            ptr_data._type != Shader::SPT_int) {
          GLCAT.error()
            << "Cannot pass floating-point data to integer shader input '" << spec._id._name << "'\n";

          // Deactivate it to make sure the user doesn't get flooded with this
          // error.
          spec._dep[0] = 0;
          spec._dep[1] = 0;

        } else {
          switch (spec._dim[1]) {
          case 1: _glgsg->_glUniform1uiv(p, array_size, (GLuint *)ptr_data._ptr); continue;
          case 2: _glgsg->_glUniform2uiv(p, array_size, (GLuint *)ptr_data._ptr); continue;
          case 3: _glgsg->_glUniform3uiv(p, array_size, (GLuint *)ptr_data._ptr); continue;
          case 4: _glgsg->_glUniform4uiv(p, array_size, (GLuint *)ptr_data._ptr); continue;
          }
          nassertd(false) continue;
        }
        break;

      case Shader::SPT_double:
        GLCAT.error() << "Passing double-precision shader inputs to GLSL shaders is not currently supported\n";

        // Deactivate it to make sure the user doesn't get flooded with this
        // error.
        spec._dep[0] = 0;
        spec._dep[1] = 0;

      default:
        continue;
      }
    }
  }

  if (altered & _shader->_mat_deps) {
    for (int i = 0; i < (int)_shader->_mat_spec.size(); ++i) {
      Shader::ShaderMatSpec &spec = _shader->_mat_spec[i];

      if ((altered & (spec._dep[0] | spec._dep[1])) == 0) {
        continue;
      }

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

      GLint p = spec._id._seqno;
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
      case Shader::SMP_cell14:
        _glgsg->_glUniform1fv(p, 1, data+14);
        continue;
      case Shader::SMP_cell13:
        _glgsg->_glUniform1fv(p, 1, data+13);
        continue;
      }
    }
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Changes the active transform table, used for hardware skinning.
 */
void CLP(ShaderContext)::
update_transform_table(const TransformTable *table) {
  LMatrix4f *matrices = (LMatrix4f *)alloca(_transform_table_size * 64);

  size_t i = 0;
  if (table != nullptr) {
    size_t num_transforms = min((size_t)_transform_table_size, table->get_num_transforms());
    for (; i < num_transforms; ++i) {
#ifdef STDFLOAT_DOUBLE
      LMatrix4 matrix;
      table->get_transform(i)->get_matrix(matrix);
      matrices[i] = LCAST(float, matrix);
#else
      table->get_transform(i)->get_matrix(matrices[i]);
#endif
    }
  }
  for (; i < (size_t)_transform_table_size; ++i) {
    matrices[i] = LMatrix4f::ident_mat();
  }

  _glgsg->_glUniformMatrix4fv(_transform_table_index, _transform_table_size,
                              GL_FALSE, (float *)matrices);
}

/**
 * Changes the active slider table, used for hardware skinning.
 */
void CLP(ShaderContext)::
update_slider_table(const SliderTable *table) {
  float *sliders = (float *)alloca(_slider_table_size * 4);
  memset(sliders, 0, _slider_table_size * 4);

  if (table != nullptr) {
    size_t num_sliders = min((size_t)_slider_table_size, table->get_num_sliders());
    for (size_t i = 0; i < num_sliders; ++i) {
      sliders[i] = table->get_slider(i)->get_slider();
    }
  }

  _glgsg->_glUniform1fv(_slider_table_index, _slider_table_size, sliders);
}

/**
 * Disable all the vertex arrays used by this shader.
 */
void CLP(ShaderContext)::
disable_shader_vertex_arrays() {
  if (_glsl_program == 0) {
    return;
  }

  for (size_t i = 0; i < _shader->_var_spec.size(); ++i) {
    const Shader::ShaderVarSpec &bind = _shader->_var_spec[i];
    GLint p = bind._id._seqno;

    for (int i = 0; i < bind._elements; ++i) {
      _glgsg->disable_vertex_attrib_array(p + i);
    }
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Disables all vertex arrays used by the previous shader, then enables all
 * the vertex arrays needed by this shader.  Extracts the relevant vertex
 * array data from the gsg.
 */
bool CLP(ShaderContext)::
update_shader_vertex_arrays(ShaderContext *prev, bool force) {
  if (_glsl_program == 0) {
    return true;
  }

  // Get the active ColorAttrib.  We'll need it to determine how to apply
  // vertex colors.
  const ColorAttrib *color_attrib = _color_attrib.p();

  const GeomVertexArrayDataHandle *array_reader;

  if (_glgsg->_use_vertex_attrib_binding) {
    // Use experimental new separated formatbinding state.
    const GeomVertexDataPipelineReader *data_reader = _glgsg->_data_reader;

    for (size_t ai = 0; ai < data_reader->get_num_arrays(); ++ai) {
      array_reader = data_reader->get_array_reader(ai);

      // Make sure the vertex buffer is up-to-date.
      CLP(VertexBufferContext) *gvbc = DCAST(CLP(VertexBufferContext),
        array_reader->prepare_now(_glgsg->get_prepared_objects(), _glgsg));
      nassertr(gvbc != (CLP(VertexBufferContext) *)nullptr, false);

      if (!_glgsg->update_vertex_buffer(gvbc, array_reader, force)) {
        return false;
      }

      GLintptr stride = array_reader->get_array_format()->get_stride();

      // Bind the vertex buffer to the binding index.
      if (ai >= _glgsg->_current_vertex_buffers.size()) {
        GLuint zero = 0;
        _glgsg->_current_vertex_buffers.resize(ai + 1, zero);
      }
      if (_glgsg->_current_vertex_buffers[ai] != gvbc->_index) {
        _glgsg->_glBindVertexBuffer(ai, gvbc->_index, 0, stride);
        _glgsg->_current_vertex_buffers[ai] = gvbc->_index;
      }
    }

    // Figure out which attributes to enable or disable.
    BitMask32 enabled_attribs = _enabled_attribs;
    if (_color_attrib_index != -1 &&
        color_attrib->get_color_type() != ColorAttrib::T_vertex) {
      // Vertex colours are disabled.
      enabled_attribs.clear_bit(_color_attrib_index);

#ifdef STDFLOAT_DOUBLE
      _glgsg->_glVertexAttrib4dv(_color_attrib_index, color_attrib->get_color().get_data());
#else
      _glgsg->_glVertexAttrib4fv(_color_attrib_index, color_attrib->get_color().get_data());
#endif
    }

    BitMask32 changed_attribs = enabled_attribs ^ _glgsg->_enabled_vertex_attrib_arrays;

    for (int i = 0; i < 32; ++i) {
      if (changed_attribs.get_bit(i)) {
        if (enabled_attribs.get_bit(i)) {
          _glgsg->_glEnableVertexAttribArray(i);
        } else {
          _glgsg->_glDisableVertexAttribArray(i);
        }
      }
    }
    _glgsg->_enabled_vertex_attrib_arrays = enabled_attribs;

  } else {
    Geom::NumericType numeric_type;
    int start, stride, num_values;
    size_t nvarying = _shader->_var_spec.size();

    GLint max_p = 0;

    for (size_t i = 0; i < nvarying; ++i) {
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

      GLint p = bind._id._seqno;
      max_p = max(max_p, p + bind._elements);

      // Don't apply vertex colors if they are disabled with a ColorAttrib.
      int num_elements, element_stride, divisor;
      bool normalized;
      if ((p != _color_attrib_index || color_attrib->get_color_type() == ColorAttrib::T_vertex) &&
          _glgsg->_data_reader->get_array_info(name, array_reader,
                                               num_values, numeric_type,
                                               normalized, start, stride, divisor,
                                               num_elements, element_stride)) {
        const unsigned char *client_pointer;
        if (!_glgsg->setup_array_data(client_pointer, array_reader, force)) {
          return false;
        }
        client_pointer += start;

        GLenum type = _glgsg->get_numeric_type(numeric_type);
        for (int i = 0; i < num_elements; ++i) {
          _glgsg->enable_vertex_attrib_array(p);

          if (numeric_type == GeomEnums::NT_packed_dabc) {
            // GL_BGRA is a special accepted value available since OpenGL 3.2.
            // It requires us to pass GL_TRUE for normalized.
            _glgsg->_glVertexAttribPointer(p, GL_BGRA, GL_UNSIGNED_BYTE,
                                           GL_TRUE, stride, client_pointer);
          } else if (bind._numeric_type == Shader::SPT_float ||
                     numeric_type == GeomEnums::NT_float32) {
            _glgsg->_glVertexAttribPointer(p, num_values, type,
                                           normalized, stride, client_pointer);
          } else if (bind._numeric_type == Shader::SPT_double) {
            _glgsg->_glVertexAttribLPointer(p, num_values, type,
                                            stride, client_pointer);
          } else {
            _glgsg->_glVertexAttribIPointer(p, num_values, type,
                                            stride, client_pointer);
          }

          if (divisor > 0) {
            _glgsg->set_vertex_attrib_divisor(p, divisor);
          }

          ++p;
          client_pointer += element_stride;
        }
      } else {
        for (int i = 0; i < bind._elements; ++i) {
          _glgsg->disable_vertex_attrib_array(p + i);
        }
        if (p == _color_attrib_index) {
          // Vertex colors are disabled or not present.  Apply flat color.
#ifdef STDFLOAT_DOUBLE
          _glgsg->_glVertexAttrib4dv(p, _glgsg->_scene_graph_color.get_data());
#else
          _glgsg->_glVertexAttrib4fv(p, _glgsg->_scene_graph_color.get_data());
#endif
        }
      }
    }

    // Disable attribute arrays we don't use.
    GLint highest_p = _glgsg->_enabled_vertex_attrib_arrays.get_highest_on_bit() + 1;
    for (GLint p = max_p; p < highest_p; ++p) {
      _glgsg->disable_vertex_attrib_array(p);
    }
  }

  if (_transform_table_index >= 0) {
    const TransformTable *table = _glgsg->_data_reader->get_transform_table();
    update_transform_table(table);
  }

  if (_slider_table_index >= 0) {
    const SliderTable *table = _glgsg->_data_reader->get_slider_table();
    update_slider_table(table);
  }

  _glgsg->report_my_gl_errors();

  return true;
}

/**
 * Disable all the texture bindings used by this shader.
 */
void CLP(ShaderContext)::
disable_shader_texture_bindings() {
  if (_glsl_program == 0) {
    return;
  }

  DO_PSTATS_STUFF(_glgsg->_texture_state_pcollector.add_level(1));

  for (size_t i = 0; i < _shader->_tex_spec.size(); ++i) {
#ifndef OPENGLES
    // Check if bindless was used, if so, there's nothing to unbind.
    if (_glgsg->_supports_bindless_texture) {
      GLint p = _shader->_tex_spec[i]._id._seqno;

      if (_glsl_uniform_handles.count(p) > 0) {
        continue;
      }
    }

    if (_glgsg->_supports_multi_bind) {
      // There are non-bindless textures to unbind, and we're lazy, so let's
      // go and unbind everything after this point using one multi-bind call,
      // and then break out of the loop.
      _glgsg->_glBindTextures(i, _shader->_tex_spec.size() - i, nullptr);
      break;
    }
#endif

    _glgsg->set_active_texture_stage(i);

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
      glBindTexture(GL_TEXTURE_3D, 0);
      break;

    case Texture::TT_2d_texture_array:
      glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
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

  // Now unbind all the image units.  Not sure if we *have* to do this.
  int num_image_units = min(_glsl_img_inputs.size(), (size_t)_glgsg->_max_image_units);

  if (num_image_units > 0) {
#ifndef OPENGLES
    if (_glgsg->_supports_multi_bind) {
      _glgsg->_glBindImageTextures(0, num_image_units, nullptr);
    } else
#endif
    {
      for (int i = 0; i < num_image_units; ++i) {
        _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
      }
    }

    if (gl_enable_memory_barriers) {
      for (int i = 0; i < num_image_units; ++i) {
        ImageInput &input = _glsl_img_inputs[i];

        if (input._gtc != nullptr) {
          input._gtc->mark_incoherent(input._writable);
          input._gtc = nullptr;
        }
      }
    }
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Disables all texture bindings used by the previous shader, then enables all
 * the texture bindings needed by this shader.  Extracts the relevant vertex
 * array data from the gsg.  The current implementation is inefficient,
 * because it may unnecessarily disable textures then immediately reenable
 * them.  We may optimize this someday.
 */
void CLP(ShaderContext)::
update_shader_texture_bindings(ShaderContext *prev) {
  // if (prev) { prev->disable_shader_texture_bindings(); }

  if (_glsl_program == 0) {
    return;
  }

  GLbitfield barriers = 0;

  // First bind all the 'image units'; a bit of an esoteric OpenGL feature
  // right now.
  int num_image_units = min(_glsl_img_inputs.size(), (size_t)_glgsg->_max_image_units);

  if (num_image_units > 0) {
    for (int i = 0; i < num_image_units; ++i) {
      ImageInput &input = _glsl_img_inputs[i];
      const ParamTextureImage *param = nullptr;
      Texture *tex;

      const ShaderInput &sinp = _glgsg->_target_shader->get_shader_input(input._name);
      switch (sinp.get_value_type()) {
      case ShaderInput::M_texture_image:
        param = (const ParamTextureImage *)sinp.get_param();
        tex = param->get_texture();
        break;

      case ShaderInput::M_texture:
        // People find it convenient to be able to pass a texture without
        // further ado.
        tex = sinp.get_texture();
        break;

      case ShaderInput::M_invalid:
        GLCAT.error()
          << "Missing texture image binding input " << *input._name << "\n";
        continue;

      default:
        GLCAT.error()
          << "Mismatching type for parameter " << *input._name << ", expected texture image binding\n";
        continue;
      }

      GLuint gl_tex = 0;
      CLP(TextureContext) *gtc;

      if (tex != nullptr) {
        int view = _glgsg->get_current_tex_view_offset();

        gtc = DCAST(CLP(TextureContext), tex->prepare_now(view, _glgsg->_prepared_objects, _glgsg));
        if (gtc != nullptr) {
          input._gtc = gtc;

          _glgsg->update_texture(gtc, true);
          gl_tex = gtc->_index;

          if (gtc->needs_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)) {
            barriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
          }
        }
      }
      input._writable = false;

      if (gl_tex == 0) {
        _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);

      } else {
        // TODO: automatically convert to sized type instead of plain GL_RGBA
        // If a base type is used, it will crash.
        GLenum internal_format = gtc->_internal_format;
        if (internal_format == GL_RGBA || internal_format == GL_RGB) {
          GLCAT.error()
            << "Texture " << tex->get_name() << " has an unsized format.  Textures bound "
            << "to a shader as an image need a sized format.\n";

          // This may not actually be right, but may still prevent a crash.
          internal_format = _glgsg->get_internal_image_format(tex, true);
        }

        GLenum access = GL_READ_WRITE;
        GLint bind_level = 0;
        GLint bind_layer = 0;
        GLboolean layered = GL_TRUE;

        if (param != nullptr) {
          layered = param->get_bind_layered();
          bind_level = param->get_bind_level();
          bind_layer = param->get_bind_layer();

          bool has_read = param->has_read_access();
          bool has_write = param->has_write_access();
          input._writable = has_write;

          if (gl_force_image_bindings_writeonly) {
            access = GL_WRITE_ONLY;

          } else if (has_read && has_write) {
            access = GL_READ_WRITE;

          } else if (has_read) {
            access = GL_READ_ONLY;

          } else if (has_write) {
            access = GL_WRITE_ONLY;

          } else {
            access = GL_READ_ONLY;
            gl_tex = 0;
          }
        }
        _glgsg->_glBindImageTexture(i, gl_tex, bind_level, layered, bind_layer,
                                    access, gtc->_internal_format);
      }
    }
  }

  size_t num_textures = _shader->_tex_spec.size();
  GLuint *textures = nullptr;
  GLuint *samplers = nullptr;
#ifdef OPENGLES
  static const bool multi_bind = false;
#else
  bool multi_bind = false;
  if (num_textures > 1 &&
      _glgsg->_supports_multi_bind && _glgsg->_supports_sampler_objects) {
    // Prepare to multi-bind the textures and samplers.
    multi_bind = true;
    textures = (GLuint *)alloca(sizeof(GLuint) * num_textures);
    samplers = (GLuint *)alloca(sizeof(GLuint) * num_textures);
  }
#endif

  for (size_t i = 0; i < num_textures; ++i) {
    Shader::ShaderTexSpec &spec = _shader->_tex_spec[i];
    const InternalName *id = spec._name;

    int view = _glgsg->get_current_tex_view_offset();
    SamplerState sampler;

    PT(Texture) tex = _glgsg->fetch_specified_texture(spec, sampler, view);
    if (tex.is_null()) {
      // Apply a white texture in order to make it easier to use a shader that
      // takes a texture on a model that doesn't have a texture applied.
      if (multi_bind) {
        textures[i] = _glgsg->get_white_texture();
        samplers[i] = 0;
      } else {
        _glgsg->apply_white_texture(i);
      }
      continue;
    }

    if (tex->get_texture_type() != spec._desired_type) {
      switch (spec._part) {
      case Shader::STO_named_input:
        GLCAT.error()
          << "Sampler type of GLSL shader input '" << *id << "' does not "
             "match type of texture " << *tex << ".\n";
        break;

      case Shader::STO_stage_i:
        GLCAT.error()
          << "Sampler type of GLSL shader input p3d_Texture" << spec._stage
          << " does not match type of texture " << *tex << ".\n";
        break;

      case Shader::STO_light_i_shadow_map:
        GLCAT.error()
          << "Sampler type of GLSL shader input p3d_LightSource[" << spec._stage
          << "].shadowMap does not match type of texture " << *tex << ".\n";
        break;
      }
      // TODO: also check whether shadow sampler textures have shadow filter
      // enabled.
    }

    CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tex->prepare_now(view, _glgsg->_prepared_objects, _glgsg));
    if (gtc == nullptr) {
      if (multi_bind) {
        textures[i] = 0;
        samplers[i] = 0;
      }
      continue;
    }

#ifndef OPENGLES
    GLint p = spec._id._seqno;

    // If it was recently written to, we will have to issue a memory barrier
    // soon.
    if (gtc->needs_barrier(GL_TEXTURE_FETCH_BARRIER_BIT)) {
      barriers |= GL_TEXTURE_FETCH_BARRIER_BIT;
    }

    // Try bindless texturing first, if supported.
    if (gl_use_bindless_texture && _glgsg->_supports_bindless_texture) {
      // We demand the real texture, since we won't be able to change the
      // texture properties after this point.
      if (multi_bind) {
        textures[i] = 0;
        samplers[i] = 0;
      }
      if (!_glgsg->update_texture(gtc, true)) {
        continue;
      }

      GLuint64 handle = gtc->get_handle();
      if (handle != 0) {
        gtc->make_handle_resident();
        gtc->set_active(true);

        // Check if we have already specified this texture handle.  If so, no
        // need to call glUniformHandle again.
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

    // Bindless texturing wasn't supported or didn't work, so let's just bind
    // the texture normally.
#ifndef OPENGLES
    if (multi_bind) {
      // Multi-bind case.
      if (!_glgsg->update_texture(gtc, false)) {
        textures[i] = 0;
      } else {
        gtc->set_active(true);
        textures[i] = gtc->_index;
      }

      SamplerContext *sc = sampler.prepare_now(_glgsg->get_prepared_objects(), _glgsg);
      if (sc == nullptr) {
        samplers[i] = 0;
      } else {
        CLP(SamplerContext) *gsc = DCAST(CLP(SamplerContext), sc);
        gsc->enqueue_lru(&_glgsg->_prepared_objects->_sampler_object_lru);
        samplers[i] = gsc->_index;
      }
    } else
#endif  // !OPENGLES
    {
      // Non-multibind case.
      _glgsg->set_active_texture_stage(i);
      if (!_glgsg->update_texture(gtc, false)) {
        continue;
      }
      _glgsg->apply_texture(gtc);
      _glgsg->apply_sampler(i, sampler, gtc);
    }
  }

#ifndef OPENGLES
  if (multi_bind && num_textures > 0) {
    _glgsg->_glBindTextures(0, num_textures, textures);
    _glgsg->_glBindSamplers(0, num_textures, samplers);
  }

  if (barriers != 0) {
    // Issue a memory barrier prior to this shader's execution.
    _glgsg->issue_memory_barrier(barriers);
  }
#endif

  _glgsg->report_my_gl_errors();
}

/**
 * Updates the shader buffer bindings for this shader.
 */
void CLP(ShaderContext)::
update_shader_buffer_bindings(ShaderContext *prev) {
#ifndef OPENGLES
  // Update the shader storage buffer bindings.
  const ShaderAttrib *attrib = _glgsg->_target_shader;

  for (size_t i = 0; i < _storage_blocks.size(); ++i) {
    StorageBlock &block = _storage_blocks[i];

    ShaderBuffer *buffer = attrib->get_shader_input_buffer(block._name);
#ifndef NDEBUG
    if (buffer->get_data_size_bytes() < block._min_size) {
      GLCAT.error()
        << "cannot bind " << *buffer << " to shader because it is too small"
           " (expected at least " << block._min_size << " bytes)\n";
    }
#endif
    _glgsg->apply_shader_buffer(block._binding_index, buffer);
  }
#endif
}

/**
 * This subroutine prints the infolog for a shader.
 */
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

  // Parse the errors so that we can substitute in actual file locations
  // instead of source indices.
  std::istringstream log(info_log);
  string line;
  while (std::getline(log, line)) {
    int fileno, lineno, colno;
    int prefixlen = 0;

    // This first format is used by the majority of compilers.
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

      // This is the format NVIDIA uses.
      Filename fn = _shader->get_filename_from_index(fileno, type);
      GLCAT.error(false)
        << fn << "(" << lineno << ") : " << (line.c_str() + prefixlen) << "\n";

    } else if (sscanf(line.c_str(), "%d:%d(%d): %n", &fileno, &lineno, &colno, &prefixlen) == 3
               && prefixlen > 0) {

      // This is the format for Mesa's OpenGL ES 2 implementation.
      Filename fn = _shader->get_filename_from_index(fileno, type);
      GLCAT.error(false)
        << fn << ":" << lineno << "(" << colno << "): " << (line.c_str() + prefixlen) << "\n";

    } else if (!fatal) {
      GLCAT.warning(false) << line << "\n";

    } else {
      GLCAT.error(false) << line << "\n";
    }
  }
}

/**
 * This subroutine prints the infolog for a program.
 */
void CLP(ShaderContext)::
glsl_report_program_errors(GLuint program, bool fatal) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

  if (length > 1) {
    info_log = (char *) alloca(length);
    _glgsg->_glGetProgramInfoLog(program, length, &num_chars, info_log);

    if (strcmp(info_log, "Success.\n") != 0 &&
        strcmp(info_log, "No errors.\n") != 0 &&
        strcmp(info_log, "Validation successful.\n") != 0) {
      if (!fatal) {
        GLCAT.warning()
          << "Shader " << _shader->get_filename() << " produced the "
          << "following warnings:\n" << info_log << "\n";
      } else {
        GLCAT.error(false) << info_log << "\n";
      }
    }
  }
}

/**
 *
 */
bool CLP(ShaderContext)::
glsl_compile_shader(Shader::ShaderType type) {
  static const char *types[] = {"", "vertex ", "fragment ", "geometry ",
      "tessellation control ", "tessellation evaluation ", "compute ", ""};

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Compiling GLSL " << types[type] << "shader "
      << _shader->get_filename(type) << "\n";
  }

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
#endif
    case Shader::ST_compute:
      if (_glgsg->get_supports_compute_shaders()) {
        handle = _glgsg->_glCreateShader(GL_COMPUTE_SHADER);
      }
      break;
    default:
      break;
  }
  if (!handle) {
    GLCAT.error()
      << "Could not create a GLSL " << types[type] << "shader.\n";
    _glgsg->report_my_gl_errors();
    return false;
  }

  if (_glgsg->_use_object_labels) {
    string name = _shader->get_filename(type);
    _glgsg->_glObjectLabel(GL_SHADER, handle, name.size(), name.data());
  }

  string text_str = _shader->get_text(type);
  const char* text = text_str.c_str();
  _glgsg->_glShaderSource(handle, 1, &text, nullptr);
  _glgsg->_glCompileShader(handle);
  GLint status;
  _glgsg->_glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    GLCAT.error()
      << "An error occurred while compiling GLSL " << types[type]
      << "shader " << _shader->get_filename(type) << ":\n";
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

/**
 * This subroutine compiles a GLSL shader.
 */
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

  // Do we have a compiled program?  Try to load that.
  unsigned int format;
  string binary;
  if (_shader->get_compiled(format, binary)) {
    _glgsg->_glProgramBinary(_glsl_program, format, binary.data(), binary.size());

    GLint status;
    _glgsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
    if (status == GL_TRUE) {
      // Hooray, the precompiled shader worked.
      if (GLCAT.is_debug()) {
        GLCAT.debug() << "Loaded precompiled binary for GLSL shader "
                      << _shader->get_filename() << "\n";
      }
      return true;
    }

    // Bummer, it didn't work..  Oh well, just recompile the shader.
    if (GLCAT.is_debug()) {
      GLCAT.debug() << "Failure loading precompiled binary for GLSL shader "
                    << _shader->get_filename() << "\n";
    }
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

    // XXX Actually, it turns out that this is unavailable in the core version
    // of geometry shaders.  Probably no need to bother with it.

    // nassertr(_glgsg->_glProgramParameteri != NULL, false); GLint
    // max_vertices; glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES,
    // &max_vertices); _glgsg->_glProgramParameteri(_glsl_program,
    // GL_GEOMETRY_VERTICES_OUT_ARB, max_vertices);
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

  if (!valid) {
    return false;
  }

  // There might be warnings, so report those.  GLSLShaders::const_iterator
  // it; for (it = _glsl_shaders.begin(); it != _glsl_shaders.end(); ++it) {
  // glsl_report_shader_errors(*it); }

  // Under OpenGL's compatibility profile, we have to make sure that we bind
  // something to attribute 0.  Make sure that this is the position array.
  _glgsg->_glBindAttribLocation(_glsl_program, 0, "p3d_Vertex");
  _glgsg->_glBindAttribLocation(_glsl_program, 0, "vertex");

  // While we're at it, let's also map these to fixed locations.  These
  // attributes were historically fixed to these locations, so it might help a
  // buggy driver.
  _glgsg->_glBindAttribLocation(_glsl_program, 2, "p3d_Normal");
  _glgsg->_glBindAttribLocation(_glsl_program, 3, "p3d_Color");

  if (gl_fixed_vertex_attrib_locations) {
    _glgsg->_glBindAttribLocation(_glsl_program, 1, "transform_weight");
    _glgsg->_glBindAttribLocation(_glsl_program, 2, "normal");
    _glgsg->_glBindAttribLocation(_glsl_program, 3, "color");
    _glgsg->_glBindAttribLocation(_glsl_program, 7, "transform_index");
    _glgsg->_glBindAttribLocation(_glsl_program, 8, "p3d_MultiTexCoord0");
    _glgsg->_glBindAttribLocation(_glsl_program, 8, "texcoord");
  }

  // Also bind the p3d_FragData array to the first index always.
  if (_glgsg->_glBindFragDataLocation != nullptr) {
    _glgsg->_glBindFragDataLocation(_glsl_program, 0, "p3d_FragData");
  }

  // If we requested to retrieve the shader, we should indicate that before
  // linking.
  bool retrieve_binary = false;
  if (_glgsg->_supports_get_program_binary) {
    retrieve_binary = _shader->get_cache_compiled_shader();

#ifndef NDEBUG
    if (gl_dump_compiled_shaders) {
      retrieve_binary = true;
    }
#endif

    _glgsg->_glProgramParameteri(_glsl_program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
  }

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Linking GLSL shader " << _shader->get_filename() << "\n";
  }

  _glgsg->_glLinkProgram(_glsl_program);

  GLint status;
  _glgsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    GLCAT.error() << "An error occurred while linking GLSL shader "
                  << _shader->get_filename() << "\n";
    glsl_report_program_errors(_glsl_program, true);
    return false;
  }

  // Report any warnings.
  glsl_report_program_errors(_glsl_program, false);

  if (retrieve_binary) {
    GLint length = 0;
    _glgsg->_glGetProgramiv(_glsl_program, GL_PROGRAM_BINARY_LENGTH, &length);
    length += 2;

    char *binary = (char *)alloca(length);
    GLenum format;
    GLsizei num_bytes = 0;
    _glgsg->_glGetProgramBinary(_glsl_program, length, &num_bytes, &format, (void*)binary);

    _shader->set_compiled(format, binary, num_bytes);

#ifndef NDEBUG
    // Dump the binary if requested.
    if (gl_dump_compiled_shaders) {
      char filename[64];
      static int gl_dump_count = 0;
      sprintf(filename, "glsl_program%d.dump", gl_dump_count++);

      pofstream s;
      s.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
      s.write(binary, num_bytes);
      s.close();

      GLCAT.info()
        << "Dumped " << num_bytes << " bytes of program binary with format 0x"
        << hex << format << dec << "  to " << filename << "\n";
    }
#endif  // NDEBUG
  }

  _glgsg->report_my_gl_errors();
  return valid;
}

#endif  // OPENGLES_1
