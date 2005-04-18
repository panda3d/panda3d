// Filename: qpgeomVertexData.I
// Created by:  drose (06Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
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


////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_name
//       Access: Published
//  Description: Returns the name passed to the constructor, if any.
//               This name is reported on the PStats graph for vertex
//               computations.
////////////////////////////////////////////////////////////////////
INLINE const string &qpGeomVertexData::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::set_name
//       Access: Published
//  Description: Changes the name of the vertex data.  This name is
//               reported on the PStats graph for vertex computations.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexData::
set_name(const string &name) {
  _name = name;
  _app_char_pcollector = 
    PStatCollector(PStatCollector(_app_animation_pcollector, name), "Vertices");
  _cull_char_pcollector =
    PStatCollector(PStatCollector(_cull_animation_pcollector, name), "Vertices");
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_format
//       Access: Published
//  Description: Returns a pointer to the GeomVertexFormat structure
//               that defines this data.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexData::
get_format() const {
  return _format;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_usage_hint
//       Access: Published
//  Description: Returns the usage hint that was passed to the
//               constructor, and which will be passed to each array
//               data object created initially, and arrays created as
//               the result of a convert_to() operation.  See
//               geomEnums.h.
//
//               However, each individual array may be replaced with a
//               different array object with an independent usage hint
//               specified, so there is no guarantee that the
//               individual arrays all have the same usage_hint.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData::UsageHint qpGeomVertexData::
get_usage_hint() const {
  CDReader cdata(_cycler);
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::has_column
//       Access: Published
//  Description: Returns true if the data has the named column,
//               false otherwise.  This is really just a shortcut for
//               asking the same thing from the format.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::
has_column(const InternalName *name) const {
  return _format->has_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::set_num_rows
//       Access: Published
//  Description: Sets the length of the array to n rows in all of
//               the various arrays (presumably by adding rows).
//
//               The new vertex data is initialized to 0, except for
//               the "color" column, which is initialized to (1, 1, 1,
//               1).
//
//               The return value is true if the number of rows
//               was changed, false if the object already contained n
//               rows (or if there was some error).
//
//               Although this method is Published, application code
//               only very rarely has any need to call it.  Instead,
//               you should use the GeomVertexWriter to build up the
//               rows in a GeomVertexData object automatically,
//               without need to explicitly set the number of
//               rows.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::
set_num_rows(int n) {
  CDWriter cdata(_cycler);
  return do_set_num_rows(n, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_num_arrays
//       Access: Published
//  Description: Returns the number of individual arrays stored within
//               the data.  This must match
//               get_format()->get_num_arrays().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexData::
get_num_arrays() const {
  CDReader cdata(_cycler);
  return cdata->_arrays.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_array
//       Access: Published
//  Description: Returns a const pointer to the vertex data for the
//               indicated array, for application code to directly
//               examine (but not modify) the underlying vertex data.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexArrayData *qpGeomVertexData::
get_array(int i) const {
  CDReader cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_arrays.size(), NULL);
  return cdata->_arrays[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_transform_palette
//       Access: Published
//  Description: Returns a const pointer to the TransformPalette
//               assigned to this data.  Vertices within the table
//               will index into this palette to indicate their
//               dynamic skinning information; this table is used when
//               the vertex animation is to be performed by the
//               graphics hardware (but also see
//               get_transform_blend_palette()).
//
//               This will return NULL if the vertex data does not
//               have a TransformPalette assigned (which implies the
//               vertices will not be animated by the graphics
//               hardware).
////////////////////////////////////////////////////////////////////
INLINE const TransformPalette *qpGeomVertexData::
get_transform_palette() const {
  CDReader cdata(_cycler);
  return cdata->_transform_palette;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::clear_transform_palette
//       Access: Published
//  Description: Sets the TransformPalette pointer to NULL,
//               removing the palette from the vertex data.  This
//               disables hardware-driven vertex animation.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexData::
clear_transform_palette() {
  set_transform_palette(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_transform_blend_palette
//       Access: Published
//  Description: Returns a const pointer to the TransformBlendPalette
//               assigned to this data.  Vertices within the table
//               will index into this palette to indicate their
//               dynamic skinning information; this table is used when
//               the vertex animation is to be performed by the CPU
//               (but also see get_transform_palette()).
//
//               This will return NULL if the vertex data does not
//               have a TransformBlendPalette assigned (which implies
//               the vertices will not be animated by the CPU).
////////////////////////////////////////////////////////////////////
INLINE const TransformBlendPalette *qpGeomVertexData::
get_transform_blend_palette() const {
  CDReader cdata(_cycler);
  return cdata->_transform_blend_palette;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::clear_transform_blend_palette
//       Access: Published
//  Description: Sets the TransformBlendPalette pointer to NULL,
//               removing the palette from the vertex data.  This
//               disables CPU-driven vertex animation.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexData::
clear_transform_blend_palette() {
  set_transform_blend_palette(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_slider_table
//       Access: Published
//  Description: Returns a const pointer to the SliderTable
//               assigned to this data.  Vertices within the vertex
//               data will look up their morph offsets, if any, within
//               this table.
//
//               This will return NULL if the vertex data does not
//               have a SliderTable assigned.
////////////////////////////////////////////////////////////////////
INLINE const SliderTable *qpGeomVertexData::
get_slider_table() const {
  CDReader cdata(_cycler);
  return cdata->_slider_table;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::clear_slider_table
//       Access: Published
//  Description: Sets the SliderTable pointer to NULL,
//               removing the table from the vertex data.  This
//               disables morph (blend shape) animation.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexData::
clear_slider_table() {
  set_slider_table(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time the vertex data is
//               modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq qpGeomVertexData::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::animate_vertices
//       Access: Published
//  Description: Returns a GeomVertexData that represents the results
//               of computing the vertex animation on the CPU for this
//               GeomVertexData.
//
//               If there is no CPU-defined vertex animation on this
//               object, this just returns the original object.
//
//               If there is vertex animation, but the VertexTransform
//               values have not changed since last time, this may
//               return the same pointer it returned previously.  Even
//               if the VertexTransform values have changed, it may
//               still return the same pointer, but with its contents
//               modified (this is preferred, since it allows the
//               graphics backend to update vertex buffers optimally).
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexData) qpGeomVertexData::
animate_vertices() const {
  return do_animate_vertices(true);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::animate_vertices_cull
//       Access: Public
//  Description: Does exactly the same thing as animate_vertices(),
//               but when PStats is enabled, it records the time spent
//               as during the cull step instead of the app step.
//               This is intended to be called from the cull callback,
//               rather the called directly by the user.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexData) qpGeomVertexData::
animate_vertices_cull() const {
  return do_animate_vertices(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::has_vertex
//       Access: Public
//  Description: Returns true if the data has a "vertex" column, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::
has_vertex() const {
  return (_format->get_vertex_column() != (qpGeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::is_vertex_transformed
//       Access: Public
//  Description: Returns true if the data has a "vertex" column and it
//               is indicated as having been transformed into clip
//               coordinates, false if there is no vertex column or if
//               it contains ordinary 3-d pre-transformation points.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::
is_vertex_transformed() const {
  const qpGeomVertexColumn *column = _format->get_vertex_column();
  if (column != (qpGeomVertexColumn *)NULL) {
    return column->get_contents() == C_clip_point;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::has_normal
//       Access: Public
//  Description: Returns true if the data has a "normal" column, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::
has_normal() const {
  return (_format->get_normal_column() != (qpGeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::has_color
//       Access: Public
//  Description: Returns true if the data has a "color" column, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::
has_color() const {
  return (_format->get_color_column() != (qpGeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::pack_abcd
//       Access: Public, Static
//  Description: Packs four values in a DirectX-style NT_packed_abcd
//               value.
////////////////////////////////////////////////////////////////////
INLINE PN_uint32 qpGeomVertexData::
pack_abcd(unsigned int a, unsigned int b,
          unsigned int c, unsigned int d) {
  return (((a & 0xff) << 24) |
          ((b & 0xff) << 16) |
          ((c & 0xff) << 8) |
          (d & 0xff));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::unpack_abcd_a
//       Access: Public, Static
//  Description: Returns the first packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int qpGeomVertexData::
unpack_abcd_a(PN_uint32 data) {
  return (data >> 24) & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::unpack_abcd_b
//       Access: Public, Static
//  Description: Returns the second packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int qpGeomVertexData::
unpack_abcd_b(PN_uint32 data) {
  return (data >> 16) & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::unpack_abcd_c
//       Access: Public, Static
//  Description: Returns the third packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int qpGeomVertexData::
unpack_abcd_c(PN_uint32 data) {
  return (data >> 8) & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::unpack_abcd_d
//       Access: Public, Static
//  Description: Returns the fourth packed value from a DirectX-style
//               NT_packed_abcd.
////////////////////////////////////////////////////////////////////
INLINE unsigned int qpGeomVertexData::
unpack_abcd_d(PN_uint32 data) {
  return data & 0xff;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::add_transform
//       Access: Private, Static
//  Description: Adds the indicated transform to the palette, if it is
//               not already there, and returns its index number.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexData::
add_transform(TransformPalette *palette, const VertexTransform *transform,
              TransformMap &already_added) {
  pair<TransformMap::iterator, bool> result = already_added.insert(TransformMap::value_type(transform, palette->get_num_transforms()));
  
  if (result.second) {
    palette->add_transform(transform);
  }

  return (*(result.first)).second;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CacheEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData::CacheEntry::
CacheEntry(const qpGeomVertexFormat *modifier) :
  _source(NULL),
  _modifier(modifier),
  _result(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CacheEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData::CacheEntry::
CacheEntry(qpGeomVertexData *source,
           const qpGeomVertexFormat *modifier,
           const qpGeomVertexData *result) :
  _source(source),
  _modifier(modifier),
  _result(result)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CacheEntry::operator <
//       Access: Public
//  Description: Provides a unique ordering within the set.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexData::CacheEntry::
operator < (const CacheEntry &other) const {
  return _modifier < other._modifier;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData::CData::
CData() :
  _usage_hint(UH_unspecified)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexData::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData::CData::
CData(const qpGeomVertexData::CData &copy) :
  _usage_hint(copy._usage_hint),
  _arrays(copy._arrays),
  _transform_palette(copy._transform_palette),
  _transform_blend_palette(copy._transform_blend_palette),
  _slider_table(copy._slider_table),
  _animated_vertices(copy._animated_vertices),
  _animated_vertices_modified(copy._animated_vertices_modified),
  _modified(copy._modified)
{
}

INLINE ostream &
operator << (ostream &out, const qpGeomVertexData &obj) {
  obj.output(out);
  return out;
}
