// Filename: dxShaderContext9.cxx
// Created by: aignacio (Jan06)
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


#define DISASSEMBLE_SHADER true
#define dx_verify(result) __dx_verify ((result), __FILE__, __LINE__)

static char *vertex_shader_function_name = "vshader";
static char *pixel_shader_function_name = "fshader";

void print_string (char *string) {
  DBG_HLSL
    dxgsg9_cat.debug() << string;
  DBG_E
}
void print_return (void) {
  DBG_HLSL
    dxgsg9_cat.debug() << "\n";
  DBG_E
}

int __dx_verify (HRESULT result, char *file, int line) {
  int state;

  state = TRUE;
  if (result != D3D_OK) {
    char *string;

    string = (char *) DXGetErrorString9 (result);
    print_string (string);

    state = FALSE;
  }

  return state;
}

void disassemble_shader (LPD3DXBUFFER shader)
{
  if (shader) {
    LPD3DXBUFFER disassembly;

    if (dx_verify (D3DXDisassembleShader
    (
      (DWORD *) shader -> GetBufferPointer ( ),
      FALSE,
      0,
      &disassembly
    )))
    {
      int size;
      char *message;

      size = disassembly -> GetBufferSize ( );

      message = (char *) (disassembly -> GetBufferPointer ( ));
      if (message) {
        print_string (message);
        print_return ( );
      }

      disassembly -> Release ( );
    }
  }
}

void free_vertex_shader (DIRECT_3D_VERTEX_SHADER direct_3d_vertex_shader)
{
  if (direct_3d_vertex_shader) {
    direct_3d_vertex_shader -> Release ( );
  }
}

void free_pixel_shader (DIRECT_3D_PIXEL_SHADER direct_3d_pixel_shader)
{
  if (direct_3d_pixel_shader) {
    direct_3d_pixel_shader -> Release ( );
  }
}

bool find_character_in_c_string (char character, const char *c_string)
{
  bool find;
  int index;
  int length;

  find = false;
  length = strlen (c_string);
  for (index = 0; index < length; index++) {
    if (c_string [index] == character) {
      find = true;
      break;
    }
  }

  return find;
}

DIRECT_3D_SHADER compile_shader (int hlsl, int vertex_shader, char *shader_profile, char *function_name, char *file_path, char *program, DIRECT_3D_DEVICE direct_3d_device)
{
  int shader_state;
  DIRECT_3D_SHADER direct_3d_shader;
  DIRECT_3D_VERTEX_SHADER direct_3d_vertex_shader;
  DIRECT_3D_PIXEL_SHADER direct_3d_pixel_shader;

  int flags;
  D3DXMACRO *defines;
  LPD3DXINCLUDE include;
  LPD3DXBUFFER shader;
  LPD3DXBUFFER error_messages;
  LPD3DXCONSTANTTABLE constant_table;

  memset (&direct_3d_shader, 0, sizeof (DIRECT_3D_SHADER));

  flags = D3DXSHADER_DEBUG;
  defines = 0;
  include = 0;
  shader = 0;
  error_messages = 0;

  direct_3d_vertex_shader = 0;
  direct_3d_pixel_shader = 0;
  constant_table = 0;

  shader_state = FALSE;
  if (hlsl) {
    if (program) {
      if (dx_verify (
        D3DXCompileShader
        (
          program,
          strlen (program),
          defines,
          include,
          function_name,
          shader_profile,
          flags,
          &shader,
          &error_messages,
          &constant_table
        )))
      {
        shader_state = TRUE;
      }
    }
    else {
      if (dx_verify (
        D3DXCompileShaderFromFile
        (
          file_path,
          defines,
          include,
          function_name,
          shader_profile,
          flags,
          &shader,
          &error_messages,
          &constant_table
        )))
      {
        shader_state = TRUE;
      }
    }

    if (DISASSEMBLE_SHADER && shader) {
      disassemble_shader (shader);
    }
  }
  else {
    if (dx_verify (D3DXAssembleShaderFromFile (file_path, defines, include, flags, &shader, &error_messages))) {
      shader_state = TRUE;
    }
  }

  if (shader_state) {
    shader_state = FALSE;
    if (vertex_shader) {
      DWORD *buffer;
      buffer = (DWORD *) shader -> GetBufferPointer ( );

      if (dx_verify (direct_3d_device -> CreateVertexShader (buffer, &direct_3d_vertex_shader))) {
        UINT count;
        D3DXSEMANTIC *semantic_array;

        count = 0;
        semantic_array = 0;
        if (dx_verify (D3DXGetShaderInputSemantics (buffer, NULL, &count))) {
          semantic_array = new D3DXSEMANTIC [count];
          if (dx_verify (D3DXGetShaderInputSemantics (buffer, semantic_array, &count))) {
            direct_3d_shader.total_semantics = count;
            direct_3d_shader.semantic_array = semantic_array;
            shader_state = TRUE;
          }
          else {
            delete semantic_array;
          }
        }
      }
    }
    else {
      if (dx_verify (direct_3d_device -> CreatePixelShader ((DWORD *) shader -> GetBufferPointer ( ), &direct_3d_pixel_shader))) {
        shader_state = TRUE;
      }
    }
  }
  else
  {
    if (error_messages) {
      char *error_message;

      error_message = (char *) (error_messages -> GetBufferPointer ( ));
      if (error_message) {
        print_string (error_message);
        print_return ( );
      }
    }
  }

  if (shader) {
    shader -> Release ( );
  }
  if (error_messages) {
    error_messages -> Release ( );
  }

  direct_3d_shader.state = shader_state;
  if (vertex_shader) {
    direct_3d_shader.direct_3d_vertex_shader = direct_3d_vertex_shader;
  }
  else {
    direct_3d_shader.direct_3d_pixel_shader = direct_3d_pixel_shader;
  }
  direct_3d_shader.constant_table = constant_table;

  if (constant_table) {
    if (dx_verify (constant_table -> GetDesc (&direct_3d_shader.constant_table_description))) {

    }
  }

  if (vertex_shader) {

// parse vertex shader inputs
    int index;
    int total_words;
    int function_name_length;
    bool function_name_found;
    string program_string;
    vector_string program_vector_string;

    program_string = program;
    function_name_length = strlen (function_name);

    function_name_found = false;
    total_words = extract_words (program_string, program_vector_string);

    // look for the vertex shader's function name
    for (index = 0; index < total_words; index++) {
      dxgsg9_cat.debug ( ) << program_vector_string [index] << "\n";

      if (program_vector_string [index].compare (0, function_name_length, function_name) == 0) {
        dxgsg9_cat.debug ( ) << "FUNCTION NAME FOUND\n";
        function_name_found = true;
        break;
      }
    }

    // look for the left parenthesis
    if (function_name_found) {
      int start_index;

      start_index = index;
      for (index = start_index; index < total_words; index++) {

        dxgsg9_cat.debug ( ) << program_vector_string [index] << "\n";

        const char *c_string;

        c_string = program_vector_string [index].c_str ( );
        if (find_character_in_c_string ('(', c_string)) {
          dxgsg9_cat.debug ( ) << "LEFT PARENTHESIS FOUND\n";
          break;
        }
      }
      start_index = index;

      vector_string first_words;

      tokenize (program_vector_string [index], first_words, "(");

      if (first_words.size ( ) > 1) {

// type
first_words [1];

// identifier

      }

      for (index = start_index + 1; index < total_words; index++) {


      }
    }
    else {
      dxgsg9_cat.error ( ) << "vertex shader function name not found\n";
    }
  }

  return direct_3d_shader;
}

void set_dx_shader_parameter_float (DX_PARAMETER *dx_parameter, const float *data, DIRECT_3D_DEVICE direct_3d_device)
{
  if (dx_parameter) {
    int index;

    for (index = 0; index < dx_parameter -> total_constant_descriptions; index++) {
      D3DXCONSTANT_DESC *constant_description;

      constant_description = &dx_parameter -> constant_description_array [index];
      if (dx_parameter -> vertex_shader) {
        if (constant_description -> Columns == 4) {
          direct_3d_device -> SetVertexShaderConstantF (constant_description -> RegisterIndex, data, constant_description -> Rows);
        }
        else {
          int offset;
          UINT row;

          offset = 0;
          for (row = 0; row < constant_description -> Rows; row++) {
            direct_3d_device -> SetVertexShaderConstantF (constant_description -> RegisterIndex + row, data + offset, 1);
            offset += constant_description -> Columns;
          }
        }
      }
      else {
        if (constant_description -> Columns == 4) {
          direct_3d_device -> SetPixelShaderConstantF (constant_description -> RegisterIndex, data, constant_description -> Rows);
        }
        else {
          int offset;
          UINT row;

          offset = 0;
          for (row = 0; row < constant_description -> Rows; row++) {
            direct_3d_device -> SetPixelShaderConstantF (constant_description -> RegisterIndex + row, data + offset, 1);
            offset += constant_description -> Columns;
          }
        }
      }
    }
  }
}

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

  memset (&_direct_3d_vertex_shader, 0, sizeof (DIRECT_3D_SHADER));
  memset (&_direct_3d_pixel_shader, 0, sizeof (DIRECT_3D_SHADER));

  _total_dx_parameters = 0;
  _dx_parameter_array = 0;

  _transpose_matrix = false;

  _name = s->get_name ( );

  if (header == "//Cg") {

    _transpose_matrix = true;

    // CGcontext is created once during Reset ( )
    if (gsg -> _cg_context == 0) {
      release_resources();
      cerr << "Cg not supported by this video card.\n";
      return;
    }

    // IGNORE THIS FOR NOW, SEEMS TO BE LOADING IN GL SPECIFIC PROFILES
    if (false) {
      // Parse any directives in the source.
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
      cerr << "Cg not supported by this video card.\n";
      return;
    }

    // Compile the program.
    _cg_shader = true;
    try_cg_compile(s, gsg);
    cerr << _cg_errors;
    return;
  }

  int hlsl;
  bool direct_x_program;

  direct_x_program = false;
  if (header == "//hlsl") {
    direct_x_program = true;
    hlsl = true;
  }
  if (header == "//asm") {
    direct_x_program = true;
    hlsl = false;
  }
  if (direct_x_program) {

    #define MAXIMUM_CONSTANT_DESCRIPTIONS 16
    static char *parameter_type_names_array [ ] =
    {
      "D3DXPT_VOID",
      "D3DXPT_BOOL",
      "D3DXPT_INT",
      "D3DXPT_FLOAT",
      "D3DXPT_STRING",
      "D3DXPT_TEXTURE",
      "D3DXPT_TEXTURE1D",
      "D3DXPT_TEXTURE2D",
      "D3DXPT_TEXTURE3D",
      "D3DXPT_TEXTURECUBE",
      "D3DXPT_SAMPLER",
      "D3DXPT_SAMPLER1D",
      "D3DXPT_SAMPLER2D",
      "D3DXPT_SAMPLER3D",
      "D3DXPT_SAMPLERCUBE",
      "D3DXPT_PIXELSHADER",
      "D3DXPT_VERTEXSHADER",
      "D3DXPT_PIXELFRAGMENT",
      "D3DXPT_VERTEXFRAGMENT",
    };
    char *function_name;

    function_name = vertex_shader_function_name;
    _direct_3d_vertex_shader = compile_shader (hlsl, TRUE, gsg -> _vertex_shader_profile, function_name, (char *) _name.c_str ( ), (char *) s -> _text.c_str ( ), gsg -> _d3d_device);

    function_name = pixel_shader_function_name;
    _direct_3d_pixel_shader = compile_shader (hlsl, FALSE, gsg -> _pixel_shader_profile, function_name, (char *) _name.c_str ( ), (char *) s -> _text.c_str ( ), gsg -> _d3d_device);

    if (_direct_3d_vertex_shader.state && _direct_3d_pixel_shader.state) {

      int dx_parameter_index;
      DX_PARAMETER *dx_parameter;

      dx_parameter_index = 0;
      _total_dx_parameters = _direct_3d_vertex_shader.constant_table_description.Constants + _direct_3d_pixel_shader.constant_table_description.Constants;
      _dx_parameter_array = new DX_PARAMETER [_total_dx_parameters];
      memset (_dx_parameter_array, 0, _total_dx_parameters * sizeof (DX_PARAMETER));

      if (_direct_3d_vertex_shader.constant_table) {
        UINT index;

        for (index = 0; index < _direct_3d_vertex_shader.constant_table_description.Constants; index++) {
          D3DXHANDLE handle;

          handle = _direct_3d_vertex_shader.constant_table -> GetConstant (NULL, index);
          if (handle) {
            UINT count;
            D3DXCONSTANT_DESC constant_description_array [MAXIMUM_CONSTANT_DESCRIPTIONS];

            count = MAXIMUM_CONSTANT_DESCRIPTIONS;
            if (dx_verify (_direct_3d_vertex_shader.constant_table -> GetConstantDesc (handle, constant_description_array, &count))) {
              UINT constant_description_index;
              D3DXCONSTANT_DESC *constant_description;

              dx_parameter = &_dx_parameter_array [dx_parameter_index];
              dx_parameter_index++;
              dx_parameter -> total_constant_descriptions = count;
              dx_parameter -> vertex_shader = TRUE;
              dx_parameter -> constant_description_array = new D3DXCONSTANT_DESC [count];

              for (constant_description_index = 0; constant_description_index < count; constant_description_index++) {
                constant_description = &constant_description_array [constant_description_index];
                dx_parameter -> constant_description_array [constant_description_index] = *constant_description;

                DBG_HLSL
                  char string [512];
                  sprintf (string, "  VS CONSTANT %d,%d  NAME %s  REG_SET %d  INDEX %d  TYPE: %s  ELEMENTS: %d  ROWS: %d  COLS: %d\n",
                    index,
                    constant_description_index,
                    constant_description -> Name,
                    constant_description -> RegisterSet,
                    constant_description -> RegisterIndex,
                    parameter_type_names_array [constant_description -> Type],
                    constant_description -> Elements,
                    constant_description -> Rows,
                    constant_description -> Columns);
                  print_string (string);
                DBG_E
              }

              compile_cg_parameter (0, dx_parameter);
            }
          }
        }
      }
      if (_direct_3d_pixel_shader.constant_table) {
        UINT index;

        for (index = 0; index < _direct_3d_pixel_shader.constant_table_description.Constants; index++) {
          D3DXHANDLE handle;

          handle = _direct_3d_pixel_shader.constant_table -> GetConstant (NULL, index);
          if (handle) {
            UINT count;
            D3DXCONSTANT_DESC constant_description_array [MAXIMUM_CONSTANT_DESCRIPTIONS];

            count = MAXIMUM_CONSTANT_DESCRIPTIONS;
            if (dx_verify (_direct_3d_pixel_shader.constant_table -> GetConstantDesc (handle, constant_description_array, &count))) {
              UINT constant_description_index;
              D3DXCONSTANT_DESC *constant_description;

              dx_parameter = &_dx_parameter_array [dx_parameter_index];
              dx_parameter_index++;
              dx_parameter -> total_constant_descriptions = count;
              dx_parameter -> vertex_shader = FALSE;
              dx_parameter -> constant_description_array = new D3DXCONSTANT_DESC [count];

              for (constant_description_index = 0; constant_description_index < count; constant_description_index++) {
                constant_description = &constant_description_array [constant_description_index];
                dx_parameter -> constant_description_array [constant_description_index] = *constant_description;

                DBG_HLSL
                  char string [512];
                  sprintf (string, "  PS CONSTANT %d,%d  NAME %s  REG_SET %d  INDEX %d  TYPE: %s  ELEMENTS: %d  ROWS: %d  COLS: %d\n",
                    index,
                    constant_description_index,
                    constant_description -> Name,
                    constant_description -> RegisterSet,
                    constant_description -> RegisterIndex,
                    parameter_type_names_array [constant_description -> Type],
                    constant_description -> Elements,
                    constant_description -> Rows,
                    constant_description -> Columns);
                  print_string (string);
                DBG_E

              }

              compile_cg_parameter (0, dx_parameter);
            }
          }
        }
      }

      if (_direct_3d_vertex_shader.semantic_array) {
        int index;

        DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: semantic_array = " << _direct_3d_vertex_shader.total_semantics <<  "\n"; DBG_E

        int stream_index;
        VertexElementArray *vertex_element_array;

// SHADER ISSUE: STREAM INDEX ALWAYS 0 FOR VERTEX BUFFER?
        stream_index = 0;
        vertex_element_array = new VertexElementArray (_direct_3d_vertex_shader.total_semantics + 2);

        for (index = 0; index < _direct_3d_vertex_shader.total_semantics; index++) {
          D3DXSEMANTIC *semantic;

          semantic = &_direct_3d_vertex_shader.semantic_array [index];
          switch (semantic -> Usage)
          {
            case D3DDECLUSAGE_POSITION:
              vertex_element_array -> add_position_xyz_vertex_element (stream_index);

// vertex_element_array -> add_position_xyzw_vertex_element (stream_index);

              break;
            case D3DDECLUSAGE_BLENDWEIGHT:
              break;
            case D3DDECLUSAGE_BLENDINDICES:
              break;
            case D3DDECLUSAGE_NORMAL:
              vertex_element_array -> add_normal_vertex_element (stream_index);
              break;
            case D3DDECLUSAGE_PSIZE:
              break;
            case D3DDECLUSAGE_TEXCOORD:

              vertex_element_array -> add_uv_vertex_element (stream_index);


//          vertex_element_array -> add_u_vertex_element (stream_index);
//          vertex_element_array -> add_uvw_vertex_element (stream_index);

              break;
            case D3DDECLUSAGE_TANGENT:
              vertex_element_array -> add_tangent_vertex_element (stream_index);
              break;
            case D3DDECLUSAGE_BINORMAL:
              vertex_element_array -> add_binormal_vertex_element (stream_index);
              break;
            case D3DDECLUSAGE_TESSFACTOR:
              break;
            case D3DDECLUSAGE_POSITIONT:
              break;
            case D3DDECLUSAGE_COLOR:
              if (semantic -> UsageIndex == 0) {
                vertex_element_array -> add_diffuse_color_vertex_element (stream_index);
              }
              else {
                vertex_element_array -> add_specular_color_vertex_element (stream_index);
              }
              break;
            case D3DDECLUSAGE_FOG:
              break;
            case D3DDECLUSAGE_DEPTH:
              break;
            case D3DDECLUSAGE_SAMPLE:
              break;
            default:
              break;
          }
        }

        int state;

        state = vertex_element_array -> add_end_vertex_element ( );
        if (state) {
          _vertex_size = vertex_element_array -> offset;

          DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: vertex size " << _vertex_size <<  "\n"; DBG_E

          _vertex_element_array = vertex_element_array;
        }
      }

      _state = true;
    }

    return;
  }

#endif
  cerr << s->get_name() << ": unrecognized shader language " << header << "\n";
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

  if (_direct_3d_vertex_shader.semantic_array) {
    delete _direct_3d_vertex_shader.semantic_array;
  }

  if (_direct_3d_vertex_shader.state) {
    free_vertex_shader (_direct_3d_vertex_shader.direct_3d_vertex_shader);
  }
  if (_direct_3d_pixel_shader.state) {
    free_pixel_shader (_direct_3d_pixel_shader.direct_3d_pixel_shader);
  }

  if (_dx_parameter_array) {
    int index;

    for (index = 0; index < _total_dx_parameters; index++) {
      DX_PARAMETER *dx_parameter;

      dx_parameter = &_dx_parameter_array [index];
      if (dx_parameter -> constant_description_array) {
        delete dx_parameter -> constant_description_array;
        dx_parameter -> constant_description_array = 0;
      }
    }

    delete _dx_parameter_array;
    _dx_parameter_array = 0;
  }
}

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
    cerr << "Cg: unrecognized profile name: " << vpro << " " << fpro << "\n";
    _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
    _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
    return;
  }

// cgD3D9IsProfileSupported DOES NOT EXIST
/*
  // If the suggestion is parseable, but not supported, ignore silently.
  if ((!cgD3D9IsProfileSupported(_cg_profile[SHADER_type_vert]))||
      (!cgD3D9IsProfileSupported(_cg_profile[SHADER_type_frag]))) {
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
  _cg_errors = "";

  cgGetError();
  _cg_program[0] =
    cgCreateProgram(gsg -> _cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[0], vertex_shader_function_name, (const char**)NULL);
  print_cg_compile_errors(s->get_name(), gsg -> _cg_context);

  cgGetError();
  _cg_program[1] =
    cgCreateProgram(gsg -> _cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[1], pixel_shader_function_name, (const char**)NULL);
  print_cg_compile_errors(s->get_name(), gsg -> _cg_context);

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
  if ((_cg_profile[1] == CG_PROFILE_ARBFP1) && (gsg->get_supports_shadow_filter ( ))) {

    DBG_SH1  dxgsg9_cat.debug ( ) << "SHADER COMPILE HACK\n"; DBG_E

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
                        _cg_profile[1], pixel_shader_function_name, (const char**)NULL);
      print_cg_compile_errors(s->get_name(), gsg -> _cg_context);
      if (_cg_program[SHADER_type_frag]==0) {
        release_resources();
        return false;
      }
    }
  }

  bool success = true;
  CGparameter parameter;
  for (int progindex=0; progindex<2; progindex++) {
    for (parameter = cgGetFirstLeafParameter(_cg_program[progindex],CG_PROGRAM);
         parameter != 0;
         parameter = cgGetNextLeafParameter(parameter)) {
      success &= compile_cg_parameter(parameter, 0);
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

  _cg_errors = s->get_name() + ": compiled to "
    + cgGetProfileString(_cg_profile[SHADER_type_vert]) + " "
    + cgGetProfileString(_cg_profile[SHADER_type_frag]) + "\n";

  DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: try_cg_compile \n"; DBG_E

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
  if (_state && gsg -> _cg_context != 0) {
    DBG_SH5  dxgsg9_cat.debug ( ) << "SHADER: bind \n";  DBG_E

    // clear the last cached FVF to make sure the next SetFVF call goes through
    gsg -> _last_fvf = 0;

    // Pass in k-parameters and transform-parameters
    issue_parameters(gsg);

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
      }
    }
    else {
      hr = gsg -> _d3d_device -> SetVertexShader (_direct_3d_vertex_shader.direct_3d_vertex_shader);
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "SetVertexShader ( ) failed "
          << D3DERRORSTRING(hr);
      }
      hr = gsg -> _d3d_device -> SetPixelShader (_direct_3d_pixel_shader.direct_3d_pixel_shader);
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "SetPixelShader ( ) failed "
          << D3DERRORSTRING(hr);
      }
    }

    // DEBUG
    DBG_SH2

      IDirect3DVertexShader9 *vertex_shader;
      IDirect3DPixelShader9 *pixel_shader;

      hr = gsg -> _d3d_device -> GetVertexShader (&vertex_shader);
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "GetVertexShader ( ) failed "
          << D3DERRORSTRING(hr);
      }
      hr = gsg -> _d3d_device -> GetPixelShader (&pixel_shader);
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "GetPixelShader ( ) failed "
          << D3DERRORSTRING(hr);
      }

      DBG_SH5  dxgsg9_cat.debug ( )
        << this -> _name
        << "\nSHADER: V "
        << vertex_shader
        << " P "
        << pixel_shader
        << " CG VS"
        << _cg_program[SHADER_type_vert]
        << " CG PS"
        << _cg_program[SHADER_type_frag]
        << "\n";
      DBG_E

    DBG_E
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind(GSG *gsg)
{
#ifdef HAVE_CGDX9
  if (_state && gsg -> _cg_context != 0) {

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
//     Function: DXShaderContext9::issue_cg_auto_bind
//       Access: Public
//  Description: Pass a single system parameter into the shader.
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGDX9
void CLP(ShaderContext)::
issue_cg_auto_bind(const ShaderAutoBind &bind, GSG *gsg)
{
  LVecBase4f t; float xhi,yhi; int px,py;
  LMatrix4f matrix;
  LMatrix4f temp_matrix;

  CGparameter p = bind.parameter;
  DX_PARAMETER *dx_parameter = bind.dx_parameter;

  HRESULT hr;

  DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: issue_cg_auto_bind " << bind.value << "\n"; DBG_E

  switch(bind.value) {
  case SIC_mat_modelview:
  case SIC_inv_modelview:
  case SIC_tps_modelview:
  case SIC_itp_modelview:
  case SIC_mat_projection:
  case SIC_inv_projection:
  case SIC_tps_projection:
  case SIC_itp_projection:
  case SIC_mat_texture:
  case SIC_inv_texture:
  case SIC_tps_texture:
  case SIC_itp_texture:
  case SIC_mat_modelproj:
  case SIC_inv_modelproj:
  case SIC_tps_modelproj:
  case SIC_itp_modelproj:

// ORIGINAL OpenGL CODE
// cgGLetStateMatrixParameter(p, (cgD3D9enum)((bind.value >> 2)+4), (cgD3D9enum)(bind.value & 3));

    const LMatrix4f *p_matrix;

    p_matrix = 0;

    // which matrix
    switch ((bind.value >> 2) + 4)
    {
      case CG_GL_MODELVIEW_MATRIX:

// SHADER ISSUE: which matrix ?????
//p_matrix = &(gsg -> _external_transform -> get_mat ( ));
        p_matrix = &(gsg -> _internal_transform -> get_mat ( ));

        if (_transpose_matrix) {
          temp_matrix.transpose_from (*p_matrix);
          p_matrix = &temp_matrix;
        }
        break;
      case CG_GL_PROJECTION_MATRIX:

        p_matrix = &gsg->_projection_mat->get_mat();

        if (_transpose_matrix) {
          temp_matrix.transpose_from (*p_matrix);
          p_matrix = &temp_matrix;
        }
        break;
      case CG_GL_TEXTURE_MATRIX:

        const TexMatrixAttrib *tex_matrix_attrib;

        tex_matrix_attrib = gsg->_state._tex_matrix;

// SHADER ISSUE: using default ????? GL texture matrix = which DX texture matrix ?????
        p_matrix = &(tex_matrix_attrib -> get_mat ( ));

        if (_transpose_matrix) {
          temp_matrix.transpose_from (*p_matrix);
          p_matrix = &temp_matrix;
        }
        break;
      case CG_GL_MODELVIEW_PROJECTION_MATRIX:

        const LMatrix4f *model_matrix;
        const LMatrix4f *projection_matrix;

        DBG_SH4  dxgsg9_cat.debug ( ) << "SHADER: issue_cg_auto_bind CG_GL_MODELVIEW_PROJECTION_MATRIX " << bind.value << "\n"; DBG_E

        projection_matrix = &gsg->_projection_mat->get_mat();

// which matrix ?????
// which multiply order ?????
        static int state = 3;
        switch (state & 0x03)
        {
          case 0:
            model_matrix = &(gsg -> _external_transform -> get_mat ( ));
            temp_matrix.multiply (*projection_matrix, *model_matrix);
            break;
          case 1:
            model_matrix = &(gsg -> _internal_transform -> get_mat ( ));
            temp_matrix.multiply (*projection_matrix, *model_matrix);
            break;
          case 2:
            model_matrix = &(gsg -> _external_transform -> get_mat ( ));
            temp_matrix.multiply (*model_matrix, *projection_matrix);
            break;
          case 3:
            model_matrix = &(gsg -> _internal_transform -> get_mat ( ));
            temp_matrix.multiply (*model_matrix, *projection_matrix);
            break;
        }

DBG_SH4
  const float *data;
  data = model_matrix -> get_data ( );
  dxgsg9_cat.debug ( ) << "MODELVIEW MATRIX \n" <<
    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";

  model_matrix = &(gsg -> _external_transform -> get_mat ( ));
  data = model_matrix -> get_data ( );
  dxgsg9_cat.debug ( ) << "EXTERNAL MODELVIEW MATRIX \n" <<
    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";

  data = projection_matrix -> get_data ( );
  dxgsg9_cat.debug ( ) << "PROJECTION MATRIX \n" <<
    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";

  D3DMATRIX d3d_matrix;

  data = &d3d_matrix._11;

  gsg -> _d3d_device -> GetTransform (D3DTS_WORLDMATRIX(0), &d3d_matrix);
  dxgsg9_cat.debug ( ) << "D3DTS_WORLDMATRIX(0) \n" <<
    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
DBG_E

/*
gsg -> _d3d_device -> GetTransform (D3DTS_WORLDMATRIX(1), &d3d_matrix);
    dxgsg9_cat.debug ( ) << "D3DTS_WORLDMATRIX(1) \n" <<
      data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
      data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
      data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
      data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
gsg -> _d3d_device -> GetTransform (D3DTS_WORLDMATRIX(2), &d3d_matrix);
    dxgsg9_cat.debug ( ) << "D3DTS_WORLDMATRIX(2) \n" <<
      data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
      data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
      data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
      data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
*/

        if (_transpose_matrix) {
          temp_matrix.transpose_in_place ( );
        }
        p_matrix = &temp_matrix;
        break;
    }

    // matrix operation
    switch (bind.value & 3)
    {
      case CG_GL_MATRIX_IDENTITY:
        break;
      case CG_GL_MATRIX_TRANSPOSE:
        matrix.transpose_from (*p_matrix);
        p_matrix = &matrix;
        break;
      case CG_GL_MATRIX_INVERSE:
        matrix.invert_from (*p_matrix);
        p_matrix = &matrix;
        break;
      case CG_GL_MATRIX_INVERSE_TRANSPOSE:
        matrix.invert_from (*p_matrix);
        matrix.transpose_in_place ( );
        p_matrix = &matrix;
        break;
    }

    if (_cg_shader) {
      hr = cgD3D9SetUniform (p, p_matrix -> get_data ( ));
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "cgD3D9SetUniform "
          << bind.value << " failed "
          << " size " << cgD3D9TypeToSize(cgGetParameterType(p))
          << D3DERRORSTRING(hr);

        switch (hr) {
          case CGD3D9ERR_INVALIDPARAM:
            dxgsg9_cat.error() << "CGD3D9ERR_INVALIDPARAM\n";
            break;
          case CGD3D9ERR_NODEVICE:
            dxgsg9_cat.error() << "CGD3D9ERR_NODEVICE\n";
            break;
          case CGD3D9ERR_NOTLOADED:
            dxgsg9_cat.error() << "CGD3D9ERR_NOTLOADED\n";
            break;
          case CGD3D9ERR_NOTUNIFORM:
            dxgsg9_cat.error() << "CGD3D9ERR_NOTUNIFORM\n";
            break;
          case CGD3D9ERR_NULLVALUE:
            dxgsg9_cat.error() << "CGD3D9ERR_NULLVALUE\n";
            break;
          case CGD3D9ERR_OUTOFRANGE:
            dxgsg9_cat.error() << "CGD3D9ERR_OUTOFRANGE\n";
            break;
        }
      }
    }
    else {
      set_dx_shader_parameter_float (dx_parameter, p_matrix -> get_data ( ), gsg -> _d3d_device);
    }
    return;
  case SIC_sys_windowsize:
    t[0] = gsg->get_current_display_region ( )->get_pixel_width();
    t[1] = gsg->get_current_display_region ( )->get_pixel_height();
    t[2] = 1;
    t[3] = 1;
    if (_cg_shader) {
      hr = cgD3D9SetUniform (p, t.get_data());
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "cgD3D9SetUniform " << bind.value << " failed "
          << D3DERRORSTRING(hr);
      }
    }
    else {
      set_dx_shader_parameter_float (dx_parameter, t.get_data ( ), gsg -> _d3d_device);
    }
    return;
  case SIC_sys_pixelsize:
    t[0] = 1.0 / gsg->get_current_display_region ( )->get_pixel_width();
    t[1] = 1.0 / gsg->get_current_display_region ( )->get_pixel_height();
    t[2] = 1;
    t[3] = 1;
    if (_cg_shader) {
      hr = cgD3D9SetUniform (p, t.get_data());
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "cgD3D9SetUniform " << bind.value << " failed "
          << D3DERRORSTRING(hr);
      }
    }
    else {
      set_dx_shader_parameter_float (dx_parameter, t.get_data ( ), gsg -> _d3d_device);
    }
    return;
  case SIC_sys_cardcenter:
    px = gsg->get_current_display_region ( )->get_pixel_width();
    py = gsg->get_current_display_region ( )->get_pixel_height();
    xhi = (px*1.0) / Texture::up_to_power_2(px);
    yhi = (py*1.0) / Texture::up_to_power_2(py);
    t[0] = xhi*0.5;
    t[1] = yhi*0.5;
    t[2] = 1;
    t[3] = 1;
    if (_cg_shader) {
      hr = cgD3D9SetUniform (p, t.get_data());
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "cgD3D9SetUniform " << bind.value << " failed "
          << D3DERRORSTRING(hr);
      }
    }
    else {
      set_dx_shader_parameter_float (dx_parameter, t.get_data ( ), gsg -> _d3d_device);
    }
    return;
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::issue_parameters
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               has changed, but the ShaderExpansion itself has not
//               changed.  It loads new parameters into the
//               already-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
issue_parameters(GSG *gsg)
{
#ifdef HAVE_CGDX9

  DBG_SH3  dxgsg9_cat.debug ( ) << "SHADER: issue_parameters\n"; DBG_E

  if (gsg -> _cg_context != 0) {
    // Pass in k-float parameters.
    for (int i=0; i<(int)_cg_fbind.size(); i++) {
      const float *data;
      InternalName *id = _cg_fbind[i].name;
      const ShaderInput *input = gsg->_target._shader->get_shader_input(id);

      DBG_SH3  dxgsg9_cat.debug ( ) << "SHADER: issue_parameters, _cg_fbind " << id -> get_name ( ) << "\n"; DBG_E

      data = input->get_vector().get_data();

      #define TOTAL_VECTORS 32
      int start_register;
      int total_vectors;
      float constant_buffer [TOTAL_VECTORS * 4];
      float constant_buffer_result [TOTAL_VECTORS * 4];

      start_register = 0;
      total_vectors = TOTAL_VECTORS;

      DBG_SH3
        char string [256];

        sprintf (string, "%f  %f  %f  %f \n", data [0], data [1], data [2], data [3]);
        dxgsg9_cat.debug ( ) << string;

        gsg -> _d3d_device -> GetPixelShaderConstantF (start_register, constant_buffer, total_vectors);

      DBG_E

      if (_cg_shader) {
        HRESULT hr;

        hr = cgD3D9SetUniform (_cg_fbind[i].parameter, data);
        if (FAILED (hr)) {
          dxgsg9_cat.error() << "cgD3D9SetUniform failed " << D3DERRORSTRING(hr);
        }
      }
      else {
        set_dx_shader_parameter_float (_cg_fbind[i].dx_parameter, data, gsg -> _d3d_device);
      }

      DBG_SH3

        gsg -> _d3d_device -> GetPixelShaderConstantF (start_register, constant_buffer_result, total_vectors);

        int index;
        for (index = 0; index < (total_vectors * 4); index++)
        {
          if (constant_buffer [index] != constant_buffer_result [index])
          {
            char string [256];

            sprintf (string, "reg %d [%d] %f  %f\n", index / 4, index & 0x03, constant_buffer [index], constant_buffer_result [index]);
            dxgsg9_cat.debug ( ) << string;
          }
        }

      DBG_E
    }

    // Pass in k-float4x4 parameters.
    for (int i=0; i<(int)_cg_npbind.size(); i++) {
      InternalName *id = _cg_npbind[i].name;
      const ShaderInput *input = gsg->_target._shader->get_shader_input(id);
      const float *dat;
      if (input->get_nodepath().is_empty()) {
        dat = LMatrix4f::ident_mat().get_data();

        DBG_SH3  dxgsg9_cat.debug ( ) << "SHADER: issue_parameters, _cg_npbind " << id -> get_name ( ) << "\n"; DBG_E

        if (_cg_shader) {
          cgD3D9SetUniform (_cg_npbind[i].parameter, dat);
        }
        else {
          set_dx_shader_parameter_float (_cg_npbind[i].dx_parameter, dat, gsg -> _d3d_device);
        }

      } else {
        dat = input->get_nodepath().node()->get_transform()->get_mat().get_data();

        float matrix [16];

        matrix [0] = dat[0];
        matrix [1] = dat[4];
        matrix [2] = dat[8];
        matrix [3] = dat[12];
        matrix [4] = dat[1];
        matrix [5] = dat[5];
        matrix [6] = dat[9];
        matrix [7] = dat[13];
        matrix [8] = dat[2];
        matrix [9] = dat[6];
        matrix [10] = dat[10];
        matrix [11] = dat[14];
        matrix [12] = dat[3];
        matrix [13] = dat[7];
        matrix [14] = dat[11];
        matrix [15] = dat[15];

        DBG_SH3  dxgsg9_cat.debug ( ) << "SHADER: issue_parameters, _cg_npbind 2 \n"; DBG_E

        if (_cg_shader) {
          cgD3D9SetUniform (_cg_npbind[i].parameter, matrix);
        }
        else {
          set_dx_shader_parameter_float (_cg_npbind[i].dx_parameter, matrix, gsg -> _d3d_device);
        }
      }
    }

    // Pass in system parameters
    for (int i=0; i<(int)_cg_auto_param.size(); i++) {
      issue_cg_auto_bind(_cg_auto_param[i], gsg);
    }

    // Pass in trans,tpose,row,col,xvec,yvec,zvec,pos parameters
    for (int i=0; i<(int)_cg_parameter_bind.size(); i++) {

      DBG_SH3  dxgsg9_cat.debug ( ) << "SHADER: issue_parameters, _cg_parameter_bind \n"; DBG_E

      bind_cg_transform(_cg_parameter_bind[i], gsg);
    }
  }
#endif
  issue_transform(gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::issue_transform
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               or the TransformState has changed, but the
//               ShaderExpansion itself has not changed.  It loads
//               new parameters into the already-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
issue_transform(GSG *gsg)
{
#ifdef HAVE_CGDX9
  if (gsg -> _cg_context != 0) {
    // Pass in modelview, projection, etc.
    for (int i=0; i<(int)_cg_auto_trans.size(); i++) {
      issue_cg_auto_bind(_cg_auto_trans[i], gsg);
    }
    // Pass in trans,tpose,row,col,xvec,yvec,zvec,pos parameters
    for (int i=0; i<(int)_cg_transform_bind.size(); i++) {
      bind_cg_transform(_cg_transform_bind[i], gsg);
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
    for (int i=0; i<(int)_cg_varying.size(); i++) {

// ORIGINAL GL CODE: NO EQUIVALENT IN DX
//    cgGLDisableClientState(_cg_varying[i].parameter);

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
      cerr << "immediate mode shaders not implemented yet\n";
      return;
    }
*/
#endif // SUPPORT_IMMEDIATE_MODE

    // Create and cache a VertexElementArray that does most of the
    // mapping from the vertex shader inputs to a VertexElementArray.
    // This can be done since a vertex shader has a well defined input.
    // Later when the vertex buffer is applied the offsets will
    // be properly mapped.
    if (_vertex_element_array == 0 && _cg_shader) {
      const GeomVertexArrayData *array_data;
      Geom::NumericType numeric_type;
      int start, stride, num_values;
      int nvarying = _cg_varying.size();

      DBG_SH2  dxgsg9_cat.debug ( ) << "SHADER: update_shader_vertex_arrays: nvarying = " << nvarying <<  "\n"; DBG_E

      int stream_index;
      VertexElementArray *vertex_element_array;

// SHADER ISSUE: STREAM INDEX ALWAYS 0 FOR VERTEX BUFFER?
      stream_index = 0;
      vertex_element_array = new VertexElementArray (nvarying + 2);

      for (int i=0; i<nvarying; i++) {
        InternalName *name = _cg_varying[i].name;
        int texslot = _cg_varying[i].append_uv;
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
/*
          // ORIGINAL OpenGL CODE
          const unsigned char *client_pointer = gsg->setup_array_data(array_data);
          cgGLSetParameterPointer(_cg_varying[i].parameter,
                                  num_values, gsg->get_numeric_type(numeric_type),
                                  stride, client_pointer + start);
*/

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
          dxgsg9_cat.error ( ) << "get_array_info ( ) failed\n";
        }
      }

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
    for (int i=0; i<(int)_cg_texbind.size(); i++) {
      int texunit;

      if (_cg_shader) {
        texunit = cgGetParameterResourceIndex(_cg_texbind[i].parameter);
      }
      else {
        texunit = _cg_texbind[i].dx_parameter -> constant_description_array [0].RegisterIndex;
      }

/*
      gsg->_glActiveTexture(GL_TEXTURE0 + texunit);
      GLP(Disable)(GL_TEXTURE_1D);
      GLP(Disable)(GL_TEXTURE_2D);
      if (gsg->get_supports_3d_texture ( )) {
        GLP(Disable)(GL_TEXTURE_3D);
      }
      if (gsg->get_supports_cube_map ( )) {
        GLP(Disable)(GL_TEXTURE_CUBE_MAP);
      }
      // This is probably faster - but maybe not as safe?
      // cgD3D9DisableTextureParameter(_cg_texbind[i].parameter);
*/

// NOT SURE IF THIS IS CORRECT FROM GL TO DX
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
    for (int i=0; i<(int)_cg_texbind.size(); i++) {
      Texture *tex = 0;
      InternalName *id = _cg_texbind[i].name;
      if (id != 0) {
        const ShaderInput *input = gsg->_target._shader->get_shader_input(id);
        tex = input->get_texture();
      } else {
        if (_cg_texbind[i].stage >= gsg->_target._texture->get_num_on_stages()) {
          continue;
        }
        TextureStage *stage = gsg->_target._texture->get_on_stage(_cg_texbind[i].stage);
        tex = gsg->_target._texture->get_on_texture(stage);
      }
      if (_cg_texbind[i].suffix != 0) {
        // The suffix feature is inefficient. It is a temporary hack.
        if (tex == 0) {
          continue;
        }
        tex = tex->load_related(_cg_texbind[i].suffix);
      }
      if ((tex == 0) || (tex->get_texture_type() != _cg_texbind[i].desiredtype)) {
        continue;
      }
      TextureContext *tc = tex->prepare_now(gsg->get_prepared_objects ( ), gsg);
      if (tc == (TextureContext*)NULL) {
        continue;
      }
      int texunit;
      if (_cg_shader) {
        texunit = cgGetParameterResourceIndex(_cg_texbind[i].parameter);
      }
      else {
        texunit = _cg_texbind[i].dx_parameter -> constant_description_array [0].RegisterIndex;
      }

// ?????
/*
      gsg->_glActiveTexture(GL_TEXTURE0 + texunit);

      GLenum target = gsg->get_texture_target(tex->get_texture_type());
      if (target == GL_NONE) {
        // Unsupported texture mode.
        continue;
      }
      GLP(Enable)(target);
*/
// NOT SURE IF THIS IS CORRECT FROM GL TO DX
      gsg->apply_texture(texunit, tc);
    }
  }
#endif
}

#ifdef HAVE_CGDX9
////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::bind_cg_transform
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind_cg_transform(const ShaderTransBind &stb, GSG *gsg)
{
  const float *data;
  CPT(TransformState) src;
  CPT(TransformState) rel;

  if (stb.src_name == InternalName::get_camera()) {
    src = TransformState::make_identity();
  } else if (stb.src_name == InternalName::get_view()) {
    src = gsg->get_inv_cs_transform ( );
  } else if (stb.src_name == InternalName::get_model()) {
    src = gsg->get_transform();
  } else if (stb.src_name == InternalName::get_world()) {
    src = gsg->get_scene()->get_world_transform();
  } else {
    const ShaderInput *input = gsg->_target._shader->get_shader_input(stb.src_name);
    if (input->get_nodepath().is_empty()) {
      src = gsg->get_scene()->get_world_transform();
    } else {
      src = gsg->get_scene()->get_world_transform()->
        compose(input->get_nodepath().get_net_transform());
    }
  }

  if (stb.rel_name == InternalName::get_camera()) {
    rel = TransformState::make_identity();
  } else if (stb.rel_name == InternalName::get_view()) {
    rel = gsg->get_inv_cs_transform ( );
  } else if (stb.rel_name == InternalName::get_model()) {
    rel = gsg->get_transform();
  } else if (stb.rel_name == InternalName::get_world()) {
    rel = gsg->get_scene()->get_world_transform();
  } else {
    const ShaderInput *input = gsg->_target._shader->get_shader_input(stb.rel_name);
    if (input->get_nodepath().is_empty()) {
      rel = gsg->get_scene()->get_world_transform();
    } else {
      rel = gsg->get_scene()->get_world_transform()->
        compose(input->get_nodepath().get_net_transform());
    }
  }

  CPT(TransformState) total = rel->invert_compose(src);

  //  data = src->get_mat().get_data();
  //  cerr << "Src for " << cgGetParameterName(stb.parameter) << " is\n" <<
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = rel->get_mat().get_data();
  //  cerr << "Rel for " << cgGetParameterName(stb.parameter) << " is\n" <<
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = total->get_mat().get_data();
  //  cerr << "Total for " << cgGetParameterName(stb.parameter) << " is\n" <<
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = gsg->_cs_transform->get_mat().get_data();
  //  cerr << "cs_transform is\n" <<
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = gsg->_internal_transform->get_mat().get_data();
  //  cerr << "internal_transform is\n" <<
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";


  DBG_SH4  dxgsg9_cat.debug ( ) << "bind_cg_transform " << stb.trans_piece << "\n"; DBG_E

  data = total->get_mat().get_data();
  switch (stb.trans_piece) {
/*
  case SMP_whole: cgD3D9SetMatrixParameterfc(stb.parameter, data); break;
  case SMP_transpose:  cgD3D9SetMatrixParameterfr(stb.parameter, data); break;
  case SMP_row0: cgD3D9SetParameter4fv(stb.parameter, data+ 0); break;
  case SMP_row1: cgD3D9SetParameter4fv(stb.parameter, data+ 4); break;
  case SMP_row2: cgD3D9SetParameter4fv(stb.parameter, data+ 8); break;
  case SMP_row3: cgD3D9SetParameter4fv(stb.parameter, data+12); break;
  case SMP_col0: cgD3D9SetParameter4f(stb.parameter, data[0], data[4], data[ 8], data[12]); break;
  case SMP_col1: cgD3D9SetParameter4f(stb.parameter, data[1], data[5], data[ 9], data[13]); break;
  case SMP_col2: cgD3D9SetParameter4f(stb.parameter, data[2], data[6], data[10], data[14]); break;
  case SMP_col3: cgD3D9SetParameter4f(stb.parameter, data[3], data[7], data[11], data[15]); break;
*/

    float vector [4];
    float matrix [16];

    case SMP_whole:
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, data);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, data, gsg -> _d3d_device);
      }
      break;
    case SMP_transpose:
      matrix [0] = data[0];
      matrix [1] = data[4];
      matrix [2] = data[8];
      matrix [3] = data[12];
      matrix [4] = data[1];
      matrix [5] = data[5];
      matrix [6] = data[9];
      matrix [7] = data[13];
      matrix [8] = data[2];
      matrix [9] = data[6];
      matrix [10] = data[10];
      matrix [11] = data[14];
      matrix [12] = data[3];
      matrix [13] = data[7];
      matrix [14] = data[11];
      matrix [15] = data[15];
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, matrix);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, matrix, gsg -> _d3d_device);
      }
      break;
    case SMP_row0:
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, data);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, data, gsg -> _d3d_device);
      }
      break;
    case SMP_row1:
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, data + 4);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, data + 4, gsg -> _d3d_device);
      }
      break;
    case SMP_row2:
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, data + 8);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, data + 8, gsg -> _d3d_device);
      }
      break;
    case SMP_row3:
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, data + 12);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, data + 12, gsg -> _d3d_device);
      }
      break;
    case SMP_col0:
      vector [0] = data[0];
      vector [1] = data[4];
      vector [2] = data[8];
      vector [3] = data[12];
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, vector);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, vector, gsg -> _d3d_device);
      }
      break;
    case SMP_col1:
      vector [0] = data[1];
      vector [1] = data[5];
      vector [2] = data[9];
      vector [3] = data[13];
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, vector);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, vector, gsg -> _d3d_device);
      }
      break;
    case SMP_col2:
      vector [0] = data[2];
      vector [1] = data[6];
      vector [2] = data[10];
      vector [3] = data[14];
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, vector);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, vector, gsg -> _d3d_device);
      }
      break;
    case SMP_col3:
      vector [0] = data[3];
      vector [1] = data[7];
      vector [2] = data[11];
      vector [3] = data[15];
      if (_cg_shader) {
        cgD3D9SetUniform (stb.parameter, vector);
      }
      else {
        set_dx_shader_parameter_float (stb.dx_parameter, vector, gsg -> _d3d_device);
      }
      break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_words
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter contains
//               the specified number of words.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_words(CGparameter p, int len)
{
  if (p) {
    vector_string words;

    tokenize(cgGetParameterName(p), words, "_");
    if (words.size() != len) {
      errchk_cg_output(p, "parameter name has wrong number of words");
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_direction
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct direction.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_direction(CGparameter p, CGenum dir)
{
  if (p) {
    if (cgGetParameterDirection(p) != dir) {
      if (dir == CG_IN)    errchk_cg_output(p, "parameter should be declared 'in'");
      if (dir == CG_OUT)   errchk_cg_output(p, "parameter should be declared 'out'");
      if (dir == CG_INOUT) errchk_cg_output(p, "parameter should be declared 'inout'");
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_variance
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_variance(CGparameter p, CGenum var)
{
  if (p) {
    if (cgGetParameterVariability(p) != var) {
      if (var == CG_UNIFORM)
        errchk_cg_output(p, "parameter should be declared 'uniform'");
      if (var == CG_VARYING)
        errchk_cg_output(p, "parameter should be declared 'varying'");
      if (var == CG_CONSTANT)
        errchk_cg_output(p, "parameter should be declared 'const'");
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_prog
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter is a part
//               of the correct program.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_prog(CGparameter p, CGprogram prog, const string &msg)
{
  if (p) {
    if (cgGetParameterProgram(p) != prog) {
      string fmsg = "parameter can only be used in a ";
      errchk_cg_output(p, fmsg+msg+" program");
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_type
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_type(CGparameter p, CGtype dt)
{
  if (p) {
    if (cgGetParameterType(p) != dt) {
      string msg = "parameter should be of type ";
      errchk_cg_output(p, msg + cgGetTypeString(dt));
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_float
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has
//               a floating point type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_float(CGparameter p)
{
  if (p) {
    CGtype t = cgGetParameterType(p);
    if ((t != CG_FLOAT1)&&(t != CG_FLOAT2)&&(t != CG_FLOAT3)&&(t != CG_FLOAT4)) {
      errchk_cg_output(p, "parameter should have a float type");
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_sampler
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has
//               a texture type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_sampler(CGparameter p)
{
  if (p) {
    CGtype t = cgGetParameterType(p);
    if ((t!=CG_SAMPLER1D)&&
        (t!=CG_SAMPLER2D)&&
        (t!=CG_SAMPLER3D)&&
        (t!=CG_SAMPLERCUBE)) {
      errchk_cg_output(p, "parameter should have a 'sampler' type");
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_output
//       Access: Public, Static
//  Description: Print an error message including a description
//               of the specified Cg parameter.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
errchk_cg_output(CGparameter p, const string &msg)
{
  if (p) {
    string vstr;
    CGenum v = cgGetParameterVariability(p);
    if (v == CG_UNIFORM)  vstr = "uniform ";
    if (v == CG_VARYING)  vstr = "varying ";
    if (v == CG_CONSTANT) vstr = "const ";

    string dstr;
    CGenum d = cgGetParameterDirection(p);
    if (d == CG_IN)    dstr = "in ";
    if (d == CG_OUT)   dstr = "out ";
    if (d == CG_INOUT) dstr = "inout ";

    const char *ts = cgGetTypeString(cgGetParameterType(p));

    string err;
    string fn = _shader_expansion->get_name();
    err = fn + ": " + msg + " (" + vstr + dstr + ts + " " + cgGetParameterName(p) + ")\n";
    _cg_errors = _cg_errors + err + "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::print_cg_compile_errors
//       Access: Public, Static
//  Description: Used only after a Cg compile command, to print
//               out any error messages that may have occurred
//               during the Cg shader compilation.  The 'file'
//               is the name of the file containing the Cg code.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
print_cg_compile_errors(const string &file, CGcontext ctx)
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
          _cg_errors += file + " " + errlines[i] + "\n";
        }
      }
    } else {
      _cg_errors += file + ": " + cgGetErrorString(err) + "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::compile_cg_parameter
//       Access: Public
//  Description: Analyzes a Cg parameter and decides how to
//               bind the parameter to some part of panda's
//               internal state.  Updates one of the cg bind
//               arrays to cause the binding to occur.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
compile_cg_parameter(CGparameter p, DX_PARAMETER *dx_parameter)
{
  string pname;

  pname = "";
  if (p) {
    pname = cgGetParameterName(p);
  }
  if (dx_parameter) {
    if (dx_parameter -> constant_description_array [0].Name[0] == '$') {
      pname = &dx_parameter -> constant_description_array [0].Name[1];
    }
    else {
      pname = dx_parameter -> constant_description_array [0].Name;
    }
  }

  if (pname.size() == 0) return true;
  if (pname[0] == '$') return true;
  vector_string pieces;
  tokenize(pname, pieces, "_");

  if (pieces.size() < 2) {
    errchk_cg_output(p,"invalid parameter name");
    return false;
  }

  if (pieces[0] == "vtx") {
    if ((!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_VARYING)) ||
        (!errchk_cg_parameter_float(p)) ||
        (!errchk_cg_parameter_prog(p, _cg_program[SHADER_type_vert], "vertex")))
      return false;
    ShaderVarying bind;
    bind.parameter = p;
    bind.dx_parameter = dx_parameter;
    if (pieces.size() == 2) {
      if (pieces[1]=="position") {
        bind.name = InternalName::get_vertex();
        bind.append_uv = -1;
        _cg_varying.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,8)=="texcoord") {
        bind.name = InternalName::get_texcoord();
        bind.append_uv = atoi(pieces[1].c_str()+8);
        _cg_varying.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,7)=="tangent") {
        bind.name = InternalName::get_tangent();
        bind.append_uv = atoi(pieces[1].c_str()+7);
        _cg_varying.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,8)=="binormal") {
        bind.name = InternalName::get_binormal();
        bind.append_uv = atoi(pieces[1].c_str()+8);
        _cg_varying.push_back(bind);
        return true;
      }
    }
    bind.name = InternalName::get_root();
    bind.append_uv = -1;
    for (int i=1; i<(int)(pieces.size()-0); i++)
      bind.name = bind.name->append(pieces[i]);
    _cg_varying.push_back(bind);
    return true;
  }

  if ((pieces[0] == "trans")||
      (pieces[0] == "tpose")||
      (pieces[0] == "row0")||
      (pieces[0] == "row1")||
      (pieces[0] == "row2")||
      (pieces[0] == "row3")||
      (pieces[0] == "col0")||
      (pieces[0] == "col1")||
      (pieces[0] == "col2")||
      (pieces[0] == "col3")) {
    if ((!errchk_cg_parameter_words(p,4)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)))
      return false;
    if ((pieces[2] != "rel")&&(pieces[2] != "to")) {
      errchk_cg_output(p, "syntax error");
      return false;
    }
    ShaderTransBind bind;
    bind.parameter = p;
    bind.dx_parameter = dx_parameter;

    if      (pieces[0]=="trans") bind.trans_piece = SMP_whole;
    else if (pieces[0]=="tpose") bind.trans_piece = SMP_transpose;
    else if (pieces[0]=="row0") bind.trans_piece = SMP_row0;
    else if (pieces[0]=="row1") bind.trans_piece = SMP_row1;
    else if (pieces[0]=="row2") bind.trans_piece = SMP_row2;
    else if (pieces[0]=="row3") bind.trans_piece = SMP_row3;
    else if (pieces[0]=="col0") bind.trans_piece = SMP_col0;
    else if (pieces[0]=="col1") bind.trans_piece = SMP_col1;
    else if (pieces[0]=="col2") bind.trans_piece = SMP_col2;
    else if (pieces[0]=="col3") bind.trans_piece = SMP_col3;

    bind.src_name = InternalName::make(pieces[1]);
    bind.rel_name = InternalName::make(pieces[3]);

    if ((bind.trans_piece == SMP_whole)||(bind.trans_piece == SMP_transpose)) {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4x4)) return false;
    } else {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4)) return false;
    }

    _cg_parameter_bind.push_back(bind);
    if ((bind.src_name == InternalName::get_model()) ||
        (bind.rel_name == InternalName::get_model())) {
      _cg_transform_bind.push_back(bind);
    }
    return true;
  }

  if ((pieces[0]=="mstrans")||
      (pieces[0]=="cstrans")||
      (pieces[0]=="wstrans")||
      (pieces[0]=="mspos")||
      (pieces[0]=="cspos")||
      (pieces[0]=="wspos")) {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)))
      return false;
    ShaderTransBind bind;
    bind.parameter = p;
    bind.dx_parameter = dx_parameter;

    if      (pieces[0]=="wstrans") { bind.rel_name = InternalName::get_world();  bind.trans_piece = SMP_whole; }
    else if (pieces[0]=="vstrans") { bind.rel_name = InternalName::get_view();   bind.trans_piece = SMP_whole; }
    else if (pieces[0]=="cstrans") { bind.rel_name = InternalName::get_camera(); bind.trans_piece = SMP_whole; }
    else if (pieces[0]=="mstrans") { bind.rel_name = InternalName::get_model();  bind.trans_piece = SMP_whole; }
    else if (pieces[0]=="wspos")   { bind.rel_name = InternalName::get_world();  bind.trans_piece = SMP_row3; }
    else if (pieces[0]=="vspos")   { bind.rel_name = InternalName::get_view();   bind.trans_piece = SMP_row3; }
    else if (pieces[0]=="cspos")   { bind.rel_name = InternalName::get_camera(); bind.trans_piece = SMP_row3; }
    else if (pieces[0]=="mspos")   { bind.rel_name = InternalName::get_model();  bind.trans_piece = SMP_row3; }

    bind.src_name = InternalName::make(pieces[1]);

    if ((bind.trans_piece == SMP_whole)||(bind.trans_piece == SMP_transpose)) {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4x4)) return false;
    } else {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4)) return false;
    }
    _cg_parameter_bind.push_back(bind);
    if ((bind.src_name == InternalName::get_model()) ||
        (bind.rel_name == InternalName::get_model())) {
      _cg_transform_bind.push_back(bind);
    }
    return true;
  }

  if ((pieces[0]=="mat")||
      (pieces[0]=="inv")||
      (pieces[0]=="tps")||
      (pieces[0]=="itp")) {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_type(p, CG_FLOAT4x4)))
      return false;
    ShaderAutoBind bind;
    bind.parameter = p;
    bind.dx_parameter = dx_parameter;
    bind.value = 0;
    if      (pieces[1] == "modelview")  bind.value += 0;
    else if (pieces[1] == "projection") bind.value += 4;
    else if (pieces[1] == "texmatrix")  bind.value += 8;
    else if (pieces[1] == "modelproj")  bind.value += 12;
    else {
      errchk_cg_output(p, "unrecognized matrix name");
      return false;
    }
    if      (pieces[0]=="mat") bind.value += 0;
    else if (pieces[0]=="inv") bind.value += 1;
    else if (pieces[0]=="tps") bind.value += 2;
    else if (pieces[0]=="itp") bind.value += 3;
    _cg_auto_trans.push_back(bind);
    return true;
  }

  if (pieces[0] == "sys") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM))) {
      return false;
    }
    ShaderAutoBind bind;
    bind.parameter = p;
    bind.dx_parameter = dx_parameter;
    if (pieces[1] == "pixelsize") {
      if (!errchk_cg_parameter_type(p, CG_FLOAT2)) {
        return false;
      }
      bind.value = SIC_sys_pixelsize;
      _cg_auto_param.push_back(bind);
      return true;
    }
    if (pieces[1] == "windowsize") {
      if (!errchk_cg_parameter_type(p, CG_FLOAT2)) {
        return false;
      }
      bind.value = SIC_sys_windowsize;
      _cg_auto_param.push_back(bind);
      return true;
    }
    if (pieces[1] == "cardcenter") {
      if (!errchk_cg_parameter_type(p, CG_FLOAT2)) {
        return false;
      }
      bind.value = SIC_sys_cardcenter;
      _cg_auto_param.push_back(bind);
      return true;
    }
    errchk_cg_output(p,"unknown system parameter");
    return false;
  }

  if (pieces[0] == "tex") {
    if ((!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_sampler(p)))
      return false;
    if ((pieces.size() != 2)&&(pieces.size() != 3)) {
      errchk_cg_output(p, "Invalid parameter name");
      return false;
    }
    ShaderTexBind bind;
    bind.parameter = p;
    bind.dx_parameter = dx_parameter;
    bind.name = 0;
    bind.stage = atoi(pieces[1].c_str());
    if (p) {
      switch (cgGetParameterType(p)) {
      case CG_SAMPLER1D:   bind.desiredtype = Texture::TT_1d_texture; break;
      case CG_SAMPLER2D:   bind.desiredtype = Texture::TT_2d_texture; break;
      case CG_SAMPLER3D:   bind.desiredtype = Texture::TT_3d_texture; break;
      case CG_SAMPLERCUBE: bind.desiredtype = Texture::TT_cube_map; break;
      default:
        errchk_cg_output(p, "Invalid type for a tex-parameter");
        return false;
      }
    }
    else {
      switch (dx_parameter -> constant_description_array [0].Type) {
      case D3DXPT_SAMPLER1D:   bind.desiredtype = Texture::TT_1d_texture; break;
      case D3DXPT_SAMPLER2D:   bind.desiredtype = Texture::TT_2d_texture; break;
      case D3DXPT_SAMPLER3D:   bind.desiredtype = Texture::TT_3d_texture; break;
      case D3DXPT_SAMPLERCUBE: bind.desiredtype = Texture::TT_cube_map; break;
      default:
        return false;
      }
    }
    if (pieces.size()==3) {
      bind.suffix = InternalName::make(((string)"-") + pieces[2]);
    }
    _cg_texbind.push_back(bind);
    return true;
  }

  if (pieces[0] == "k") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)))
      return false;
    if (p) {
      switch (cgGetParameterType(p)) {
        case CG_FLOAT4: {
          ShaderArgBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          _cg_fbind.push_back(bind);
          break;
        }
        case CG_FLOAT4x4: {
          ShaderArgBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          _cg_npbind.push_back(bind);
          break;
        }
        case CG_SAMPLER1D: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype=Texture::TT_1d_texture;
          _cg_texbind.push_back(bind);
          break;
        }
        case CG_SAMPLER2D: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype=Texture::TT_2d_texture;
          _cg_texbind.push_back(bind);
          break;
        }
        case CG_SAMPLER3D: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype=Texture::TT_3d_texture;
          _cg_texbind.push_back(bind);
          break;
        }
        case CG_SAMPLERCUBE: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype = Texture::TT_cube_map;
          _cg_texbind.push_back(bind);
          break;
        }
        default:
          errchk_cg_output(p, "Invalid type for a k-parameter");
          return false;
      }
    }
    else {
      switch (dx_parameter -> constant_description_array [0].Type) {
        case D3DXPT_FLOAT:
          // CG_FLOAT4
          if (dx_parameter -> constant_description_array [0].Rows == 1)
          {
            ShaderArgBind bind;
            bind.parameter = p;
            bind.dx_parameter = dx_parameter;
            bind.name = InternalName::make(pieces[1]);
            _cg_fbind.push_back(bind);
          }
          // CG_FLOAT4x4:
          if (dx_parameter -> constant_description_array [0].Rows == 4)
          {
            ShaderArgBind bind;
            bind.parameter = p;
            bind.dx_parameter = dx_parameter;
            bind.name = InternalName::make(pieces[1]);
            _cg_npbind.push_back(bind);
          }
          break;

        case D3DXPT_SAMPLER1D: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype=Texture::TT_1d_texture;
          _cg_texbind.push_back(bind);
          break;
        }
        case D3DXPT_SAMPLER2D: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype=Texture::TT_2d_texture;
          _cg_texbind.push_back(bind);
          break;
        }
        case D3DXPT_SAMPLER3D: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype=Texture::TT_3d_texture;
          _cg_texbind.push_back(bind);
          break;
        }
        case D3DXPT_SAMPLERCUBE: {
          ShaderTexBind bind;
          bind.parameter = p;
          bind.dx_parameter = dx_parameter;
          bind.name = InternalName::make(pieces[1]);
          bind.desiredtype = Texture::TT_cube_map;
          _cg_texbind.push_back(bind);
          break;
        }
      }
    }
    return true;
  }

  if (pieces[0] == "l") {
    // IMPLEMENT THE ERROR CHECKING
    return true; // Cg handles this automatically.
  }

  if (pieces[0] == "o") {
    // IMPLEMENT THE ERROR CHECKING
    return true; // Cg handles this automatically.
  }

  errchk_cg_output(p, "unrecognized parameter name");
  return false;
}
#endif

