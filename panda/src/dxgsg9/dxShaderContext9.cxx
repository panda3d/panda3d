// Filename: dxShaderContext9.cxx
// Created by: jyelon (01Sep05), conversion aignacio (Jan-Mar06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "dxGraphicsStateGuardian9.h"
#include "dxShaderContext9.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

TypeHandle CLP(ShaderContext)::_type_handle;

static char *vertex_shader_function_name = "vshader";
static char *pixel_shader_function_name = "fshader";

#ifdef HAVE_CGDX9
////////////////////////////////////////////////////////////////////
//     Function: cg_type_to_panda_type
//       Access: Public, Static
//  Description: convert a cg shader-arg type to a panda shader-arg type.
////////////////////////////////////////////////////////////////////
static ShaderContext::ShaderArgType
cg_type_to_panda_type(CGtype n) {
  switch (n) {
  case CG_FLOAT1:      return ShaderContext::SAT_float1;
  case CG_FLOAT2:      return ShaderContext::SAT_float2;
  case CG_FLOAT3:      return ShaderContext::SAT_float3;
  case CG_FLOAT4:      return ShaderContext::SAT_float4;
  case CG_FLOAT4x4:    return ShaderContext::SAT_float4x4;
  case CG_SAMPLER1D:   return ShaderContext::SAT_sampler1d;
  case CG_SAMPLER2D:   return ShaderContext::SAT_sampler2d;
  case CG_SAMPLER3D:   return ShaderContext::SAT_sampler3d;
  case CG_SAMPLERCUBE: return ShaderContext::SAT_samplercube;
  default:           return ShaderContext::SAT_unknown;
  }
}
#endif  // HAVE_CGDX9

#ifdef HAVE_CGDX9
////////////////////////////////////////////////////////////////////
//     Function: cg_dir_to_panda_dir
//       Access: Public, Static
//  Description: convert a cg shader-arg type to a panda shader-arg type.
////////////////////////////////////////////////////////////////////
static ShaderContext::ShaderArgDir
cg_dir_to_panda_dir(CGenum n) {
  switch (n) {
  case CG_IN:    return ShaderContext::SAD_in;
  case CG_OUT:   return ShaderContext::SAD_out;
  case CG_INOUT: return ShaderContext::SAD_inout;
  default:       return ShaderContext::SAD_unknown;
  }
}
#endif  // HAVE_CGDX9

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(ShaderExpansion *s, GSG *gsg) : ShaderContext(s) {
  string header;
  s->parse_init();
  s->parse_line(header, true, true);

  _state = false;

#ifdef HAVE_CGDX9

  DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: Create ShaderContext \n"; DBG_E

  _cg_shader = false;

  _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
  _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
  _cg_program[SHADER_type_vert] = (CGprogram)0;
  _cg_program[SHADER_type_frag] = (CGprogram)0;

  _vertex_size = 0;
  _vertex_element_array = 0;

  _total_dx_parameters = 0;
  _dx_parameter_array = 0;

  _transpose_matrix = false;

  _name = s->get_name ( );

  if (header == "//Cg") {

    // CGcontext is created once during Reset ( )
    if (gsg -> _cg_context == 0) {
      release_resources();
      dxgsg9_cat.error() << "Cg not supported by this video card.\n";
      return;
    }

    // Parse any directives in the source.
    // IGNORE SPECIFIC PROFILES IN DX
    if (false) {
      string directive;
      while (!s->parse_eof()) {
        s->parse_line(directive, true, true);
        vector_string pieces;
        tokenize(directive, pieces, " \t");
        if ((pieces.size()==4)&&(pieces[0]=="//Cg")&&(pieces[1]=="profile")) {
          suggest_cg_profile(pieces[2], pieces[3]);
        }
      }
    }

    // Select a profile if no preferred profile specified in the source.
    if (_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN) {
      _cg_profile[SHADER_type_vert] = cgD3D9GetLatestVertexProfile( );
    }
    if (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN) {
      _cg_profile[SHADER_type_frag] = cgD3D9GetLatestPixelProfile( );
    }

    // If we still haven't chosen a profile, give up.
    if ((_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN)||
        (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN)) {
      release_resources();
      dxgsg9_cat.error() << "Cg not supported by this video card.\n";
      return;
    }

    // Compile the program.
    _cg_shader = true;
    try_cg_compile(s, gsg);
    return;
  }
#endif

  dxgsg9_cat.error() << s->get_name() << ": unrecognized shader language " << header << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  release_resources();

  if (_vertex_element_array) {
    delete _vertex_element_array;
    _vertex_element_array = 0;
  }
}

#ifdef HAVE_CGDX9
////////////////////////////////////////////////////////////////////
//     Function: Shader::report_cg_compile_errors
//       Access: Public, Static
//  Description: Used only after a Cg compile command, to print
//               out any error messages that may have occurred
//               during the Cg shader compilation.  The 'file'
//               is the name of the file containing the Cg code.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
report_cg_compile_errors(const string &file, CGcontext ctx)
{
  CGerror err = cgGetError();
  if (err != CG_NO_ERROR) {
    if (err == CG_COMPILER_ERROR) {
      string listing = cgGetLastListing(ctx);
      vector_string errlines;
      tokenize(listing, errlines, "\n");
      for (int i=0; i<(int)errlines.size(); i++) {
        string line = trim(errlines[i]);
        if (line != "") {
          dxgsg9_cat.error() << file << " " << errlines[i] << "\n";
        }
      }
    } else {
      dxgsg9_cat.error() << file << ": " << cgGetErrorString(err) << "\n";
    }
  }
}
#endif  // HAVE_CGDX9

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::suggest_cg_profile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGDX9
void CLP(ShaderContext)::
suggest_cg_profile(const string &vpro, const string &fpro)
{
  // If a good profile has already been suggested, ignore suggestion.
  if ((_cg_profile[SHADER_type_vert] != CG_PROFILE_UNKNOWN)||
      (_cg_profile[SHADER_type_frag] != CG_PROFILE_UNKNOWN)) {
    return;
  }

  // Parse the suggestion. If not parseable, print error and ignore.
  _cg_profile[SHADER_type_vert] = parse_cg_profile(vpro, true);
  _cg_profile[SHADER_type_frag] = parse_cg_profile(fpro, false);
  if ((_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN)||
      (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN)) {
    dxgsg9_cat.error() << "Cg: unrecognized profile name: " << vpro << " " << fpro << "\n";
    _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
    _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
    return;
  }

// NO EQUIVALENT FUNCTIONALITY IN DX
/*
  // If the suggestion is parseable, but not supported, ignore silently.
  if ((!cgGLIsProfileSupported(_cg_profile[SHADER_type_vert]))||
      (!cgGLIsProfileSupported(_cg_profile[SHADER_type_frag]))) {
    _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
    _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
    return;
  }
*/

}
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::parse_cg_profile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGDX9
CGprofile CLP(ShaderContext)::
parse_cg_profile(const string &id, bool vertex)
{
  int nvprofiles = 4;
  int nfprofiles = 4;
  CGprofile vprofiles[] = { CG_PROFILE_ARBVP1, CG_PROFILE_VP20, CG_PROFILE_VP30, CG_PROFILE_VP40 };
  CGprofile fprofiles[] = { CG_PROFILE_ARBFP1, CG_PROFILE_FP20, CG_PROFILE_FP30, CG_PROFILE_FP40 };
  if (vertex) {
    for (int i=0; i<nvprofiles; i++) {
      if (id == cgGetProfileString(vprofiles[i])) {
        return vprofiles[i];
      }
    }
  } else {
    for (int i=0; i<nfprofiles; i++) {
      if (id == cgGetProfileString(fprofiles[i])) {
        return fprofiles[i];
      }
    }
  }
  return CG_PROFILE_UNKNOWN;
}
#endif  // HAVE_CGDX9

int save_file (int size, void *data, char *file_path)
{
  int state;
  int file_handle;

  state = false;
  file_handle = _open (file_path, _O_CREAT | _O_RDWR | _O_TRUNC, _S_IREAD | _S_IWRITE);
  if (file_handle != -1) {
    if (_write (file_handle, data, size) == size) {
      state = true;
    }
    _close (file_handle);
  }

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::try_cg_compile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGDX9
bool CLP(ShaderContext)::
try_cg_compile(ShaderExpansion *s, GSG *gsg)
{
  cgGetError();
  _cg_program[0] =
    cgCreateProgram(gsg -> _cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[0], vertex_shader_function_name, (const char**)NULL);
  report_cg_compile_errors(s->get_name(), gsg -> _cg_context);

  cgGetError();
  _cg_program[1] =
    cgCreateProgram(gsg -> _cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[1], pixel_shader_function_name, (const char**)NULL);
  report_cg_compile_errors(s->get_name(), gsg -> _cg_context);

  if ((_cg_program[SHADER_type_vert]==0)||(_cg_program[SHADER_type_frag]==0)) {
    release_resources();
    return false;
  }

  const char *vertex_program;
  const char *pixel_program;
  vertex_program = cgGetProgramString (_cg_program[0], CG_COMPILED_PROGRAM);
  pixel_program = cgGetProgramString (_cg_program[1], CG_COMPILED_PROGRAM);

  DBG_SH3
    // DEBUG: output the generated program
    dxgsg9_cat.debug ( ) << vertex_program << "\n";
    dxgsg9_cat.debug ( ) << pixel_program << "\n";
  DBG_E

  DBG_SH1
    // DEBUG: save the generated program to a file
    int size;
    char file_path [512];

    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    _splitpath (_name.c_str ( ), drive, dir, fname, ext);

    size = strlen (vertex_program);
    sprintf (file_path, "%s.vasm", fname);
    save_file (size, (void *) vertex_program, file_path);

    size = strlen (pixel_program);
    sprintf (file_path, "%s.pasm", fname);
    save_file (size, (void *) pixel_program, file_path);

  DBG_E

  // The following code is present to work around a bug in the Cg compiler.
  // It does not generate correct code for shadow map lookups when using arbfp1.
  // This is a particularly onerous limitation, given that arbfp1 is the only
  // Cg target that works on radeons.  I suspect this is an intentional
  // omission on nvidia's part.  The following code fetches the output listing,
  // detects the error, repairs the code, and resumbits the repaired code to Cg.
  if ((_cg_profile[1] == CG_PROFILE_ARBFP1) && (gsg->_supports_shadow_filter)) {
    bool shadowunit[32];
    bool anyshadow = false;
    memset(shadowunit, 0, sizeof(shadowunit));
    vector_string lines;
    tokenize(cgGetProgramString(_cg_program[1], CG_COMPILED_PROGRAM), lines, "\n");
    // figure out which texture units contain shadow maps.
    for (int lineno=0; lineno<(int)lines.size(); lineno++) {
      if (lines[lineno].compare(0,21,"#var sampler2DSHADOW ")) {
        continue;
      }
      vector_string fields;
      tokenize(lines[lineno], fields, ":");
      if (fields.size()!=5) {
        continue;
      }
      vector_string words;
      tokenize(trim(fields[2]), words, " ");
      if (words.size()!=2) {
        continue;
      }
      int unit = atoi(words[1].c_str());
      if ((unit < 0)||(unit >= 32)) {
        continue;
      }
      anyshadow = true;
      shadowunit[unit] = true;
    }
    // modify all TEX statements that use the relevant texture units.
    if (anyshadow) {
      for (int lineno=0; lineno<(int)lines.size(); lineno++) {
        if (lines[lineno].compare(0,4,"TEX ")) {
          continue;
        }
        vector_string fields;
        tokenize(lines[lineno], fields, ",");
        if ((fields.size()!=4)||(trim(fields[3]) != "2D;")) {
          continue;
        }
        vector_string texunitf;
        tokenize(trim(fields[2]), texunitf, "[]");
        if ((texunitf.size()!=3)||(texunitf[0] != "texture")||(texunitf[2]!="")) {
          continue;
        }
        int unit = atoi(texunitf[1].c_str());
        if ((unit < 0) || (unit >= 32) || (shadowunit[unit]==false)) {
          continue;
        }
        lines[lineno] = fields[0]+","+fields[1]+","+fields[2]+", SHADOW2D;";
      }
      string result = "!!ARBfp1.0\nOPTION ARB_fragment_program_shadow;\n";
      for (int lineno=1; lineno<(int)lines.size(); lineno++) {
        result += (lines[lineno] + "\n");
      }
      cgDestroyProgram(_cg_program[1]);
      _cg_program[1] =
        cgCreateProgram(gsg -> _cg_context, CG_OBJECT, result.c_str(),
                        _cg_profile[1], "fshader", (const char**)NULL);
      report_cg_compile_errors(s->get_name(), gsg -> _cg_context);
      if (_cg_program[SHADER_type_frag]==0) {
        release_resources();
        return false;
      }
    }
  }

  bool success = true;
  CGparameter parameter;
  for (int progindex=0; progindex<2; progindex++) {
    int nvtx = _var_spec.size();
    for (parameter = cgGetFirstLeafParameter(_cg_program[progindex],CG_PROGRAM);
         parameter != 0;
         parameter = cgGetNextLeafParameter(parameter)) {
      CGenum vbl = cgGetParameterVariability(parameter);
      if ((vbl==CG_VARYING)||(vbl==CG_UNIFORM)) {
        success &= compile_parameter(parameter,
                                     cgGetParameterName(parameter),
                                     cg_type_to_panda_type(cgGetParameterType(parameter)),
                                     cg_dir_to_panda_dir(cgGetParameterDirection(parameter)),
                                     (vbl == CG_VARYING),
                                     dxgsg9_cat.get_safe_ptr());
      }
    }
    if ((progindex == SHADER_type_frag) && (nvtx != _var_spec.size())) {
      dxgsg9_cat.error() << "Cannot use vtx parameters in an fshader\n";
      success = false;
    }
  }
  if (!success) {
    release_resources();
    return false;
  }

  BOOL paramater_shadowing;
  DWORD assembly_flags;

  paramater_shadowing = FALSE;
  assembly_flags = 0;

  cgD3D9LoadProgram(_cg_program[SHADER_type_vert], paramater_shadowing, assembly_flags);
  cgD3D9LoadProgram(_cg_program[SHADER_type_frag], paramater_shadowing, assembly_flags);

  DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: end try_cg_compile \n"; DBG_E

  _state = true;

  return true;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::release_resources
//       Access: Public
//  Description: Should deallocate all system resources (such as
//               vertex program handles or Cg contexts).
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
release_resources() {
#ifdef HAVE_CGDX9
  {
    _cg_profile[SHADER_type_vert] = (CGprofile)0;
    _cg_profile[SHADER_type_frag] = (CGprofile)0;
    _cg_program[SHADER_type_vert] = (CGprogram)0;
    _cg_program[SHADER_type_frag] = (CGprogram)0;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::bind
//       Access: Public
//  Description: This function is to be called to enable a new
//               shader.  It also initializes all of the shader's
//               input parameters.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind(GSG *gsg) {

#ifdef HAVE_CGDX9
  if (_state) {
    if (gsg -> _cg_context != 0) {
      DBG_SH5  dxgsg9_cat.debug ( ) << "SHADER: bind \n";  DBG_E

      // clear the last cached FVF to make sure the next SetFVF call goes through
      gsg -> _last_fvf = 0;

      // Pass in k-parameters and transform-parameters
      issue_parameters(gsg, true);

      HRESULT hr;

      if (_cg_shader) {
        // Bind the shaders.
        hr = cgD3D9BindProgram(_cg_program[SHADER_type_vert]);
        if (FAILED (hr)) {
          dxgsg9_cat.error() << "cgD3D9BindProgram vertex shader failed\n";
        }
        hr = cgD3D9BindProgram(_cg_program[SHADER_type_frag]);
        if (FAILED (hr)) {
          dxgsg9_cat.error() << "cgD3D9BindProgram pixel shader failed\n";

          CGerror error = cgGetError();
          if (error != CG_NO_ERROR) {
            dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
          }
        }
      }
    }
  }
#endif

}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind(GSG *gsg) {

#ifdef HAVE_CGDX9
  if (_state) {

    DBG_SH5  dxgsg9_cat.debug ( ) << "SHADER: unbind \n"; DBG_E

    HRESULT hr;

    hr = gsg -> _d3d_device -> SetVertexShader (NULL);
    if (FAILED (hr)) {
      dxgsg9_cat.error()
        << "SetVertexShader (NULL) failed " << D3DERRORSTRING(hr);
    }
    hr = gsg -> _d3d_device -> SetPixelShader (NULL);
    if (FAILED (hr)) {
      dxgsg9_cat.error()
        << "SetPixelShader (NULL) failed " << D3DERRORSTRING(hr);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::issue_parameters
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               or TransformState has changed, but the ShaderExpansion
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
issue_parameters(GSG *gsg, bool altered)
{
#ifdef HAVE_CGDX9
  if (gsg -> _cg_context != 0) {
    for (int i=0; i<(int)_mat_spec.size(); i++) {
      if (altered || _mat_spec[i]._trans_dependent) {
        CGparameter p = (CGparameter)(_mat_spec[i]._parameter);
        const LMatrix4f *val = gsg->fetch_specified_value(_mat_spec[i], altered);
        if (val) {
          HRESULT hr;
          float v [4];
          LMatrix4f temp_matrix;

          hr = D3D_OK;

          const float *data;

          data = val -> get_data ( );

          switch (_mat_spec[i]._piece) {
          case SMP_whole:
            // TRANSPOSE REQUIRED
            temp_matrix.transpose_from (*val);
            data = temp_matrix.get_data();
            hr = cgD3D9SetUniform (p, data);

            DBG_SH2
              dxgsg9_cat.debug ( ) << "SMP_whole MATRIX \n" <<
                data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
                data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
                data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
                data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
            DBG_E

            break;

          case SMP_transpose:
            // NO TRANSPOSE REQUIRED
            hr = cgD3D9SetUniform (p, data);
            break;

          case SMP_row0:
            hr = cgD3D9SetUniform (p, data + 0);
            break;
          case SMP_row1:
            hr = cgD3D9SetUniform (p, data + 4);
            break;
          case SMP_row2:
            hr = cgD3D9SetUniform (p, data + 8);
            break;
          case SMP_row3:
            hr = cgD3D9SetUniform (p, data + 12);
            break;

          case SMP_col0:
            v[0] = data[0]; v[1] = data[4]; v[2] = data[8]; v[3] = data[12];
            hr = cgD3D9SetUniform (p, v);
            break;
          case SMP_col1:
            v[0] = data[1]; v[1] = data[5]; v[2] = data[9]; v[3] = data[13];
            hr = cgD3D9SetUniform (p, v);
            break;
          case SMP_col2:
            v[0] = data[2]; v[1] = data[6]; v[2] = data[10]; v[3] = data[14];
            hr = cgD3D9SetUniform (p, v);
            break;
          case SMP_col3:
            v[0] = data[3]; v[1] = data[7]; v[2] = data[11]; v[3] = data[15];
            hr = cgD3D9SetUniform (p, v);
            break;

          default:
            dxgsg9_cat.error()
              << "issue_parameters ( ) SMP parameter type not implemented " << _mat_spec[i]._piece << "\n";
            break;
          }

          if (FAILED (hr)) {
            dxgsg9_cat.error()
              << "MAT TYPE  "
              << _mat_spec[i]._piece
              << " cgD3D9SetUniform failed "
              << D3DERRORSTRING(hr);

            CGerror error = cgGetError ();
            if (error != CG_NO_ERROR) {
              dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
            }
          }
        }
      }
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays(GSG *gsg)
{
#ifdef HAVE_CGDX9
  if (gsg -> _cg_context) {
    for (int i=0; i<(int)_var_spec.size(); i++) {
      CGparameter p = (CGparameter)(_var_spec[i]._parameter);

      // DO NOTHING, CURRENTLY USING ONLY ONE STREAM SOURCE

    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::update_shader_vertex_arrays
//       Access: Public
//  Description: Disables all vertex arrays used by the previous
//               shader, then enables all the vertex arrays needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable arrays then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_vertex_arrays(gsg);
#ifdef HAVE_CGDX9
  if (gsg -> _cg_context) {

  #ifdef SUPPORT_IMMEDIATE_MODE
/*
    if (gsg->_use_sender) {
      dxgsg9_cat.error() << "immediate mode shaders not implemented yet\n";
    } else
*/
  #endif // SUPPORT_IMMEDIATE_MODE
    {
      if (_vertex_element_array == 0) {
        bool error;
        const GeomVertexArrayData *array_data;
        Geom::NumericType numeric_type;
        int start, stride, num_values;
        int nvarying = _var_spec.size();

        int stream_index;
        VertexElementArray *vertex_element_array;

        error = false;
        // SHADER ISSUE: STREAM INDEX ALWAYS 0 FOR VERTEX BUFFER?
        stream_index = 0;
        vertex_element_array = new VertexElementArray (nvarying + 2);

        for (int i=0; i<nvarying; i++) {
          CGparameter p = (CGparameter)(_var_spec[i]._parameter);
          InternalName *name = _var_spec[i]._name;
          int texslot = _var_spec[i]._append_uv;
          if (texslot >= 0) {
            const Geom::ActiveTextureStages &active_stages =
              gsg->_state._texture->get_on_stages();
            if (texslot < (int)active_stages.size()) {
              TextureStage *stage = active_stages[texslot];
              InternalName *texname = stage->get_texcoord_name();
              if (name == InternalName::get_texcoord()) {
                name = texname;
              } else if (texname != InternalName::get_texcoord()) {
                name = name->append(texname->get_basename());
              }
            }
          }
          if (gsg->_vertex_data->get_array_info(name, array_data, num_values, numeric_type, start, stride)) {

            if (false) {

            } else if (name == InternalName::get_vertex ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values) {
                  case 3:
                    vertex_element_array -> add_position_xyz_vertex_element (stream_index);
                    break;
                  case 4:
                    vertex_element_array -> add_position_xyzw_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of vertex coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid vertex type " << numeric_type << "\n";
              }

            } else if (name == InternalName::get_texcoord ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 1:
                    vertex_element_array -> add_u_vertex_element (stream_index);
                    break;
                  case 2:
                    vertex_element_array -> add_uv_vertex_element (stream_index);
                    break;
                  case 3:
                    vertex_element_array -> add_uvw_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of vertex texture coordinate elements " << num_values <<  "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid texture coordinate type " << numeric_type << "\n";
              }

            } else if (name == InternalName::get_normal ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 3:
                    vertex_element_array -> add_normal_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of normal coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid normal type " << numeric_type << "\n";
              }

            } else if (name == InternalName::get_binormal ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 3:
                    vertex_element_array -> add_binormal_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of binormal coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid binormal type " << numeric_type << "\n";
              }

            } else if (name == InternalName::get_tangent ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 3:
                    vertex_element_array -> add_tangent_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of tangent coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid tangent type " << numeric_type << "\n";
              }

            } else if (name == InternalName::get_color ( )) {

              if (numeric_type == Geom::NT_packed_dcba ||
                  numeric_type == Geom::NT_packed_dabc ||
                  numeric_type == Geom::NT_uint8) {
                switch (num_values)
                {
                  case 4:
                    vertex_element_array -> add_diffuse_color_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid color coordinates " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid color type " << numeric_type << "\n";
              }

            } else {
              dxgsg9_cat.error ( ) << "VE ERROR: unsupported vertex element " << name -> get_name ( ) << "\n";
            }

            DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: update_shader_vertex_arrays " << i <<  "\n"; DBG_E
            DBG_SH2  dxgsg9_cat.debug ( )
              << "\n  name " << name -> get_name ( )
              << "  num_values " << num_values
              << "  numeric_type " << numeric_type
              << "  start " << start
              << "  stride " << stride
              << "\n";
            DBG_E

          } else {
            dxgsg9_cat.error ( )
              << "get_array_info ( ) failed for shader "
              << _name
              << "\n"
              << "  vertex element name = "
              << name -> get_name ( )
              << "\n";
            error = true;
          }
        }

        if (error) {
          delete vertex_element_array;
        }
        else {
          int state;

          state = vertex_element_array -> add_end_vertex_element ( );
          if (state) {
            if (_cg_shader) {
              if (cgD3D9ValidateVertexDeclaration (_cg_program [SHADER_type_vert],
                    vertex_element_array -> vertex_element_array) == CG_TRUE) {
                dxgsg9_cat.debug() << "|||||cgD3D9ValidateVertexDeclaration succeeded\n";
              }
              else {
                dxgsg9_cat.error() << "********************************************\n";
                dxgsg9_cat.error() << "***cgD3D9ValidateVertexDeclaration failed***\n";
                dxgsg9_cat.error() << "********************************************\n";
              }
            }
            else {

            }

            _vertex_size = vertex_element_array -> offset;

            DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: vertex size " << _vertex_size <<  "\n"; DBG_E

            _vertex_element_array = vertex_element_array;
          }
          else {
            dxgsg9_cat.error ( ) << "VertexElementArray creation failed\n";
            delete vertex_element_array;
          }
        }
      }
    }
  }
#endif // HAVE_CGDX9
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::disable_shader_texture_bindings
//       Access: Public
//  Description: Disable all the texture bindings used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_texture_bindings(GSG *gsg)
{
#ifdef HAVE_CGDX9
  if (gsg -> _cg_context) {
    for (int i=0; i<(int)_tex_spec.size(); i++) {
      CGparameter p = (CGparameter)(_tex_spec[i]._parameter);
      int texunit = cgGetParameterResourceIndex(p);

      HRESULT hr;

      hr = gsg -> _d3d_device -> SetTexture (texunit, NULL);
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "SetTexture ("
          << texunit
          << ", NULL) failed "
          << D3DERRORSTRING(hr);
      }
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::update_shader_texture_bindings
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
update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_texture_bindings(gsg);

#ifdef HAVE_CGDX9
  if (gsg -> _cg_context) {
    for (int i=0; i<(int)_tex_spec.size(); i++) {
      CGparameter p = (CGparameter)(_tex_spec[i]._parameter);
      Texture *tex = 0;
      InternalName *id = _tex_spec[i]._name;
      if (id != 0) {
        const ShaderInput *input = gsg->_target._shader->get_shader_input(id);
        tex = input->get_texture();
      } else {
        if (_tex_spec[i]._stage >= gsg->_target._texture->get_num_on_stages()) {
          continue;
        }
        TextureStage *stage = gsg->_target._texture->get_on_stage(_tex_spec[i]._stage);
        tex = gsg->_target._texture->get_on_texture(stage);
      }
      if (_tex_spec[i]._suffix != 0) {
        // The suffix feature is inefficient. It is a temporary hack.
        if (tex == 0) {
          continue;
        }
        tex = tex->load_related(_tex_spec[i]._suffix);
      }
      if ((tex == 0) || (tex->get_texture_type() != _tex_spec[i]._desired_type)) {
        continue;
      }
      TextureContext *tc = tex->prepare_now(gsg->_prepared_objects, gsg);
      if (tc == (TextureContext*)NULL) {
        continue;
      }

      int texunit = cgGetParameterResourceIndex(p);

      gsg->apply_texture(texunit, tc);
    }
  }
#endif
}

