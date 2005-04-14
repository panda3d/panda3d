// Filename: qpgeom.I
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
//     Function: qpGeom::get_primitive_type
//       Access: Published
//  Description: Returns the primitive type that is common to all
//               GeomPrimitives added within the Geom.  All nested
//               primitives within a particular Geom must be the same
//               type.
////////////////////////////////////////////////////////////////////
INLINE qpGeom::PrimitiveType qpGeom::
get_primitive_type() const {
  CDReader cdata(_cycler);
  return cdata->_primitive_type;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_usage_hint
//       Access: Published
//  Description: Returns the minimum (i.e. most dynamic) usage_hint
//               among all of the individual GeomPrimitives that have
//               been added to the geom.
////////////////////////////////////////////////////////////////////
INLINE qpGeom::UsageHint qpGeom::
get_usage_hint() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_usage_hint) {
    CDWriter cdataw(((qpGeom *)this)->_cycler, cdata);
    ((qpGeom *)this)->reset_usage_hint(cdataw);
  }
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_point_rendering
//       Access: Published
//  Description: Returns the set of PointRendering bits that represent
//               the rendering properties required to properly render
//               the points (if any) represented within this Geom.
////////////////////////////////////////////////////////////////////
INLINE int qpGeom::
get_point_rendering() const {
  CDReader cdata(_cycler);
  return cdata->_point_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_vertex_data
//       Access: Published
//  Description: Returns a const pointer to the GeomVertexData,
//               for application code to directly examine (but not
//               modify) the geom's underlying data.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexData) qpGeom::
get_vertex_data() const {
  CDReader cdata(_cycler);
  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_num_primitives
//       Access: Published
//  Description: Returns the number of GeomPrimitive objects stored
//               within the Geom, each of which represents a number of
//               primitives of a particular type.
////////////////////////////////////////////////////////////////////
INLINE int qpGeom::
get_num_primitives() const {
  CDReader cdata(_cycler);
  return cdata->_primitives.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_primitive
//       Access: Published
//  Description: Returns a const pointer to the ith GeomPrimitive
//               object stored within the Geom.  Use this call only to
//               inspect the ith object; use modify_primitive() or
//               set_primitive() if you want to modify it.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomPrimitive *qpGeom::
get_primitive(int i) const {
  CDReader cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_primitives.size(), NULL);
  return cdata->_primitives[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::modify_primitive
//       Access: Published
//  Description: Returns a modifiable pointer to the ith GeomPrimitive
//               object stored within the Geom, so application code
//               can directly manipulate the properties of this
//               primitive.
////////////////////////////////////////////////////////////////////
INLINE qpGeomPrimitive *qpGeom::
modify_primitive(int i) {
  clear_cache();
  CDWriter cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_primitives.size(), NULL);
  cdata->_got_usage_hint = false;
  cdata->_modified = qpGeom::get_next_modified();
  if (cdata->_primitives[i]->get_ref_count() > 1) {
    cdata->_primitives[i] = cdata->_primitives[i]->make_copy();
  }
  return cdata->_primitives[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time any of the primitives in
//               the Geom is modified, or the set of primitives is
//               modified.  However, this does not include
//               modifications to the vertex data, which should be
//               tested separately.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq qpGeom::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CacheEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeom::CacheEntry::
CacheEntry(const qpGeomVertexData *source_data, const qpGeomMunger *modifier) :
  _source_data(source_data),
  _modifier(modifier)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CacheEntry::operator <
//       Access: Public
//  Description: Provides a unique ordering within the set.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeom::CacheEntry::
operator < (const CacheEntry &other) const {
  if (_modifier != other._modifier) {
    int compare = _modifier->geom_compare_to(*other._modifier);
    if (compare != 0) {
      return (compare < 0);
    }
  }
  if (_source_data != other._source_data) {
    return (_source_data < other._source_data);
  }
  return 0;
}


////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeom::CData::
CData() :
  _primitive_type(PT_none),
  _point_rendering(0),
  _usage_hint(UH_unspecified),
  _got_usage_hint(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeom::CData::
CData(const qpGeom::CData &copy) :
  _data(copy._data),
  _primitives(copy._primitives),
  _primitive_type(copy._primitive_type),
  _point_rendering(copy._point_rendering),
  _usage_hint(copy._usage_hint),
  _got_usage_hint(copy._got_usage_hint),
  _modified(copy._modified)
{
}

INLINE ostream &
operator << (ostream &out, const qpGeom &obj) {
  obj.output(out);
  return out;
}
