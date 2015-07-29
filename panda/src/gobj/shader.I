// Filename: shader.I
// Heavily Modified:  jyelon (Sep05)
// Updated by: fperazzi, PandaSE(06Apr10)
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


////////////////////////////////////////////////////////////////////
//     Function: Shader::get_filename
//       Access: Published
//  Description: Return the Shader's filename for the given shader
//               type.
////////////////////////////////////////////////////////////////////
INLINE Filename Shader::
get_filename(const ShaderType &type) const {
  if (_filename._separate && type != ST_none) {
    switch (type) {
      case ST_vertex:
        return _filename._vertex;
        break;
      case ST_fragment:
        return _filename._fragment;
        break;
      case ST_geometry:
        return _filename._geometry;
        break;
      case ST_tess_control:
        return _filename._tess_control;
        break;
      case ST_tess_evaluation:
        return _filename._tess_evaluation;
        break;
      case ST_compute:
        return _filename._compute;
        break;
      default:
        return _filename._shared;
    }
  } else if (!_filename._shared.empty()) {
    return _filename._shared;

  } else {
    // Um, better than nothing?
    return _filename._fragment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::get_text
//       Access: Published
//  Description: Return the Shader's text for the given shader type.
////////////////////////////////////////////////////////////////////
INLINE const string &Shader::
get_text(const ShaderType &type) const {
  if (_text._separate) {
    nassertr(type != ST_none || !_text._shared.empty(), _text._shared);
    switch (type) {
      case ST_vertex:
        return _text._vertex;
        break;
      case ST_fragment:
        return _text._fragment;
        break;
      case ST_geometry:
        return _text._geometry;
        break;
      case ST_tess_control:
        return _text._tess_control;
        break;
      case ST_tess_evaluation:
        return _text._tess_evaluation;
        break;
      case ST_compute:
        return _text._compute;
        break;
      default:
        return _text._shared;
    }
  } else {
    return _text._shared;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::get_error_flag
//       Access: Public
//  Description: Returns true if the shader contains a compile-time
//               error.  This doesn't tell you whether or not the
//               shader is supported on the current video card.
////////////////////////////////////////////////////////////////////
INLINE bool Shader::
get_error_flag() const {
  return _error_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::set_shader_utilization
//       Access: Published, Static
//  Description: Set this flag to SUT_none, SUT_basic, or
//               SUT_advanced to limit panda's automatic shader
//               generation facilities.
////////////////////////////////////////////////////////////////////
INLINE void Shader::
set_shader_utilization(ShaderUtilization sut) {
  _shader_utilization = sut;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::get_shader_utilization
//       Access: Published, Static
//  Description: This flag returns SUT_none, SUT_basic, or
//               SUT_advanced and controls the automatic generation
//               of shaders.  It is initialized from the config
//               variable of the same name, but it can be
//               subsequently adjusted.
////////////////////////////////////////////////////////////////////
INLINE ShaderUtilization Shader::
get_shader_utilization() {
  if (_shader_utilization == SUT_unspecified) {
    return shader_utilization;
  } else {
    return _shader_utilization;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::have_shader_utilization
//       Access: Published, Static
//  Description: If true, then get_shader_utilization has been
//               set using set_shader_utilization.
//               If false, then get_shader_utilization simply
//               returns the config variable of the same name.
////////////////////////////////////////////////////////////////////
INLINE bool Shader::
have_shader_utilization() {
  return (_shader_utilization != SUT_unspecified);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::get_language
//       Access: Published
//  Description: Returns the shader language in which this shader
//               was written.
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderLanguage Shader::
get_language() const {
  return _language;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderCapabilities Constructor
//  Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderCaps::
ShaderCaps() {
  clear();
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderCapabilities::operator ==
//  Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Shader::ShaderCaps::
operator == (const ShaderCaps &other) const {
#ifdef HAVE_CG
  if ((_active_vprofile != other._active_vprofile) ||
      (_active_fprofile != other._active_fprofile) ||
      (_active_gprofile != other._active_gprofile) ||
      (_ultimate_vprofile != other._ultimate_vprofile) ||
      (_ultimate_fprofile != other._ultimate_fprofile) ||
      (_ultimate_gprofile != other._ultimate_gprofile)) {
     return false;
  }
#endif
  return true;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData() :
  _ptr(NULL),
  _type(SPT_unknown),
  _updated(true),
  _size(0)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_float &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_float),
  _updated(true),
  _size(ptr.size())
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix4f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_float),
  _updated(true),
  _size(ptr.size() * 16)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix3f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_float),
  _updated(true),
  _size(ptr.size() * 9)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase4f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_float),
  _updated(true),
  _size(ptr.size() * 4)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase3f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_float),
  _updated(true),
  _size(ptr.size() * 3)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase2f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_float),
  _updated(true),
  _size(ptr.size() * 2)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase4f &vec) :
  _type(SPT_float),
  _updated(true),
  _size(4)
{
  PTA_float pta = PTA_float::empty_array(4);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase3f &vec) :
  _type(SPT_float),
  _updated(true),
  _size(3)
{
  PTA_float pta = PTA_float::empty_array(3);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase2f &vec) :
  _type(SPT_float),
  _updated(true),
  _size(2)
{
  PTA_float pta = PTA_float::empty_array(2);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LMatrix4f &mat) :
  _type(SPT_float),
  _updated(true),
  _size(16)
{
  PTA_float pta = PTA_float::empty_array(16);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(mat(0, 0)) * mat.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, mat.get_data(), sizeof(mat(0, 0)) * mat.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LMatrix3f &mat) :
  _type(SPT_float),
  _updated(true),
  _size(9)
{
  PTA_float pta = PTA_float::empty_array(9);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(mat(0, 0)) * mat.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, mat.get_data(), sizeof(mat(0, 0)) * mat.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_double &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_double),
  _updated(true),
  _size(ptr.size())
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix4d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_double),
  _updated(true),
  _size(ptr.size() * 16)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix3d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_double),
  _updated(true),
  _size(ptr.size() * 9)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase4d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_double),
  _updated(true),
  _size(ptr.size() * 4)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase3d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_double),
  _updated(true),
  _size(ptr.size() * 3)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase2d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_double),
  _updated(true),
  _size(ptr.size() * 2)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase4d &vec) :
  _type(SPT_double),
  _updated(true),
  _size(4)
{
  PTA_double pta = PTA_double::empty_array(4);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase3d &vec) :
  _type(SPT_double),
  _updated(true),
  _size(3)
{
  PTA_double pta = PTA_double::empty_array(3);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase2d &vec) :
  _type(SPT_double),
  _updated(true),
  _size(2)
{
  PTA_double pta = PTA_double::empty_array(2);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LMatrix4d &mat) :
  _type(SPT_double),
  _updated(true),
  _size(16)
{
  PTA_double pta = PTA_double::empty_array(16);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(mat(0, 0)) * mat.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, mat.get_data(), sizeof(mat(0, 0)) * mat.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LMatrix3d &mat) :
  _type(SPT_double),
  _updated(true),
  _size(9)
{
  PTA_double pta = PTA_double::empty_array(9);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(mat(0, 0)) * mat.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, mat.get_data(), sizeof(mat(0, 0)) * mat.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_int &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_int),
  _updated(true),
  _size(ptr.size())
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase4i &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_int),
  _updated(true),
  _size(ptr.size() * 4)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase3i &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_int),
  _updated(true),
  _size(ptr.size() * 3)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase2i &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(SPT_int),
  _updated(true),
  _size(ptr.size() * 2)
{
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase4i &vec) :
  _type(SPT_int),
  _updated(true),
  _size(4)
{
  PTA_int pta = PTA_int::empty_array(4);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase3i &vec) :
  _type(SPT_int),
  _updated(true),
  _size(3)
{
  PTA_int pta = PTA_int::empty_array(3);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderPtrData Constructor
//  Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase2i &vec) :
  _type(SPT_int),
  _updated(true),
  _size(2)
{
  PTA_int pta = PTA_int::empty_array(2);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderPtrData::write_datagram
//       Access: Public
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
INLINE void Shader::ShaderPtrData::
write_datagram(Datagram &dg) const {
  dg.add_uint8(_type);
  dg.add_uint32(_size);

  if (_type == SPT_double) {
    const double *data = (const double *) _ptr;
    for (size_t i = 0; i < _size; ++i) {
      dg.add_float64(data[i]);
    }

  } else if (_type == SPT_float) {
    const float *data = (const float *) _ptr;
    for (size_t i = 0; i < _size; ++i) {
      dg.add_float32(data[i]);
    }
  } else if (_type == SPT_int) {
    const int *data = (const int *) _ptr;
    for (size_t i = 0; i < _size; ++i) {
      dg.add_int32(data[i]);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderPtrData::write_datagram
//       Access: Public
//  Description: Reads the object from a Datagram.
////////////////////////////////////////////////////////////////////
INLINE void Shader::ShaderPtrData::
read_datagram(DatagramIterator &scan) {
  _type = (ShaderPtrType) scan.get_uint8();
  _size = scan.get_uint32();

  if (_type == SPT_double) {
    PTA_double pta = PTA_double::empty_array(_size);
    for (size_t i = 0; i < _size; ++i) {
      pta[i] = scan.get_float64();
    }
    _pta = pta.v0();
    _ptr = pta.p();

  } else if (_type == SPT_float) {
    PTA_float pta = PTA_float::empty_array(_size);
    for (size_t i = 0; i < _size; ++i) {
      pta[i] = scan.get_float32();
    }
    _pta = pta.v0();
    _ptr = pta.p();

  } else if (_type == SPT_int) {
    PTA_int pta = PTA_int::empty_array(_size);
    for (size_t i = 0; i < _size; ++i) {
      pta[i] = scan.get_int32();
    }
    _pta = pta.v0();
    _ptr = pta.p();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderFile::
ShaderFile(const string &shared) :
  _separate(false),
  _shared(shared)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderFile::
ShaderFile(const string &vertex,
           const string &fragment,
           const string &geometry,
           const string &tess_control,
           const string &tess_evaluation) :
  _separate(true),
  _vertex(vertex),
  _fragment(fragment),
  _geometry(geometry),
  _tess_control(tess_control),
  _tess_evaluation(tess_evaluation)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderFile::write_datagram
//       Access: Public
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
INLINE void Shader::ShaderFile::
write_datagram(Datagram &dg) const {
  if (_separate) {
    dg.add_uint8(6);
    dg.add_string(_vertex);
    dg.add_string(_fragment);
    dg.add_string(_geometry);
    dg.add_string(_tess_control);
    dg.add_string(_tess_evaluation);
    dg.add_string(_compute);
  } else {
    dg.add_uint8(0);
    dg.add_string(_shared);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderFile::write_datagram
//       Access: Public
//  Description: Reads the object from a Datagram.
////////////////////////////////////////////////////////////////////
INLINE void Shader::ShaderFile::
read_datagram(DatagramIterator &scan) {
  short count = scan.get_uint8();
  if (count > 0) {
    if (count-- > 0) _vertex = scan.get_string();
    if (count-- > 0) _fragment = scan.get_string();
    if (count-- > 0) _geometry = scan.get_string();
    if (count-- > 0) _tess_control = scan.get_string();
    if (count-- > 0) _tess_evaluation = scan.get_string();
    if (count-- > 0) _compute = scan.get_string();
    while (count-- > 0) {
      scan.get_string();
    }
  } else {
    _shared = scan.get_string();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderFile::operator <
//       Access: Public
//  Description: Ordering operator
////////////////////////////////////////////////////////////////////
INLINE bool Shader::ShaderFile::
operator < (const Shader::ShaderFile &other) const {
  if (_separate != other._separate) {
    return (!_separate && other._separate);
  }
  if (_shared != other._shared) {
    return (_shared < other._shared);
  }
  if (_vertex != other._vertex) {
    return (_vertex < other._vertex);
  }
  if (_fragment != other._fragment) {
    return (_fragment < other._fragment);
  }
  if (_geometry != other._geometry) {
    return (_geometry < other._geometry);
  }
  if (_tess_control != other._tess_control) {
    return (_tess_control < other._tess_control);
  }
  if (_tess_evaluation != other._tess_evaluation) {
    return (_tess_evaluation < other._tess_evaluation);
  }
  if (_compute != other._compute) {
    return (_compute < other._compute);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::get_filename_from_index
//       Access: Public
//  Description: Returns the filename of the included shader with
//               the given source file index (as recorded in the
//               #line statement in r_preprocess_source).  We use
//               this to associate error messages with included files.
////////////////////////////////////////////////////////////////////
INLINE Filename Shader::
get_filename_from_index(int index, ShaderType type) const {
  if (index == 0) {
    Filename fn = get_filename(type);
    if (!fn.empty()) {
      return fn;
    }
  } else if (glsl_preprocess && index >= 2048 &&
             (index - 2048) < (int)_included_files.size()) {
    return _included_files[index - 2048];
  }
  // Must be a mistake.  Quietly put back the integer.
  char str[32];
  sprintf(str, "%d", index);
  return Filename(str);
}
