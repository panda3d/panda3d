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
//     Function: qpGeom::get_usage_hint
//       Access: Published
//  Description: Returns the minimum (i.e. most dynamic) usage_hint
//               among all of the individual GeomPrimitives that have
//               been added to the geom.
////////////////////////////////////////////////////////////////////
INLINE qpGeomUsageHint::UsageHint qpGeom::
get_usage_hint() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_usage_hint) {
    CDWriter cdataw(((qpGeom *)this)->_cycler, cdata);
    ((qpGeom *)this)->reset_usage_hint(cdataw);
  }
  return cdata->_usage_hint;
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
  return cdata->_primitives[i];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::set_primitive
//       Access: Published
//  Description: Replaces the ith GeomPrimitive object stored within
//               the Geom with the new object.
////////////////////////////////////////////////////////////////////
INLINE void qpGeom::
set_primitive(int i, const qpGeomPrimitive *primitive) {
  clear_cache();
  CDWriter cdata(_cycler);
  nassertv(i >= 0 && i < (int)cdata->_primitives.size());
  if (cdata->_got_usage_hint &&
      cdata->_primitives[i]->get_usage_hint() != primitive->get_usage_hint()) {
    if (cdata->_primitives[i]->get_usage_hint() < primitive->get_usage_hint()) {
      // If we're reducing the usage hint, we might also be reducing
      // the minimum usage hit.
      cdata->_usage_hint = min(cdata->_usage_hint, primitive->get_usage_hint());
    } else { // (cdata->_primitives[i]->get_usage_hint() > primitive->get_usage_hint())
      // If we're increasing it, we might have to rederive the minimum.
      if (cdata->_usage_hint == cdata->_primitives[i]->get_usage_hint()) {
        cdata->_got_usage_hint = false;
      }
    }
  }
  cdata->_primitives[i] = (qpGeomPrimitive *)primitive;
  cdata->_modified = qpGeom::get_next_modified();
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
//     Function: qpGeom::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpGeom::CData::
CData() :
  _usage_hint(qpGeomUsageHint::UH_static),
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
