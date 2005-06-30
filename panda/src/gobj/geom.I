// Filename: geom.I
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
//     Function: Geom::get_primitive_type
//       Access: Published
//  Description: Returns the fundamental primitive type that is common
//               to all GeomPrimitives added within the Geom.  All
//               nested primitives within a particular Geom must be
//               the same type (that is, you can mix triangles and
//               tristrips, because they are both the same fundamental
//               type PT_polygons, but you cannot mix triangles and
//               points withn the same Geom).
////////////////////////////////////////////////////////////////////
INLINE Geom::PrimitiveType Geom::
get_primitive_type() const {
  CDReader cdata(_cycler);
  return cdata->_primitive_type;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_shade_model
//       Access: Published
//  Description: Returns the shade model common to all of the
//               individual GeomPrimitives that have been added to the
//               geom.
////////////////////////////////////////////////////////////////////
INLINE Geom::ShadeModel Geom::
get_shade_model() const {
  CDReader cdata(_cycler);
  return cdata->_shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_geom_rendering
//       Access: Published
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this Geom.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_geom_rendering() const {
  CDReader cdata(_cycler);
  return cdata->_geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_usage_hint
//       Access: Published
//  Description: Returns the minimum (i.e. most dynamic) usage_hint
//               among all of the individual GeomPrimitives that have
//               been added to the geom.
////////////////////////////////////////////////////////////////////
INLINE Geom::UsageHint Geom::
get_usage_hint() const {
  CDReader cdata(_cycler);
  if (!cdata->_got_usage_hint) {
    CDWriter cdataw(((Geom *)this)->_cycler, cdata);
    ((Geom *)this)->reset_usage_hint(cdataw);
    return cdataw->_usage_hint;
  }
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_vertex_data
//       Access: Published
//  Description: Returns a const pointer to the GeomVertexData,
//               for application code to directly examine (but not
//               modify) the geom's underlying data.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexData) Geom::
get_vertex_data() const {
  CDReader cdata(_cycler);
  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_num_primitives
//       Access: Published
//  Description: Returns the number of GeomPrimitive objects stored
//               within the Geom, each of which represents a number of
//               primitives of a particular type.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_num_primitives() const {
  CDReader cdata(_cycler);
  return cdata->_primitives.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_primitive
//       Access: Published
//  Description: Returns a const pointer to the ith GeomPrimitive
//               object stored within the Geom.  Use this call only to
//               inspect the ith object; use modify_primitive() or
//               set_primitive() if you want to modify it.
////////////////////////////////////////////////////////////////////
INLINE const GeomPrimitive *Geom::
get_primitive(int i) const {
  CDReader cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_primitives.size(), NULL);
  return cdata->_primitives[i];
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::modify_primitive
//       Access: Published
//  Description: Returns a modifiable pointer to the ith GeomPrimitive
//               object stored within the Geom, so application code
//               can directly manipulate the properties of this
//               primitive.
////////////////////////////////////////////////////////////////////
INLINE GeomPrimitive *Geom::
modify_primitive(int i) {
  clear_cache();
  CDWriter cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_primitives.size(), NULL);
  cdata->_got_usage_hint = false;
  cdata->_modified = Geom::get_next_modified();
  if (cdata->_primitives[i]->get_ref_count() > 1) {
    cdata->_primitives[i] = cdata->_primitives[i]->make_copy();
  }
  return cdata->_primitives[i];
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::decompose
//       Access: Published
//  Description: Decomposes all of the primitives within this Geom,
//               returning the result.  See
//               GeomPrimitive::decompose().
////////////////////////////////////////////////////////////////////
INLINE CPT(Geom) Geom::
decompose() const {
  PT(Geom) new_geom = new Geom(*this);
  new_geom->decompose_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::rotate
//       Access: Published
//  Description: Rotates all of the primitives within this Geom,
//               returning the result.  See
//               GeomPrimitive::rotate().
////////////////////////////////////////////////////////////////////
INLINE CPT(Geom) Geom::
rotate() const {
  PT(Geom) new_geom = new Geom(*this);
  new_geom->rotate_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::unify
//       Access: Published
//  Description: Unifies all of the primitives contained within this
//               Geom into a single primitive object.  This may
//               require decomposing the primitives if, for instance,
//               the Geom contains both triangle strips and triangle
//               fans.
////////////////////////////////////////////////////////////////////
INLINE CPT(Geom) Geom::
unify() const {
  PT(Geom) new_geom = new Geom(*this);
  new_geom->unify_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time any of the primitives in
//               the Geom is modified, or the set of primitives is
//               modified.  However, this does not include
//               modifications to the vertex data, which should be
//               tested separately.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq Geom::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::calc_tight_bounds
//       Access: Public, Virtual
//  Description: Expands min_point and max_point to include all of the
//               vertices in the Geom, if any.  found_any is set true
//               if any points are found.  It is the caller's
//               responsibility to initialize min_point, max_point,
//               and found_any before calling this function.
//
//               This version of the method assumes the Geom will use
//               its own vertex data, and the results are computed in
//               the Geom's own coordinate space.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                  bool &found_any) const {
  calc_tight_bounds(min_point, max_point, found_any,
                    get_vertex_data(), false, LMatrix4f::ident_mat());
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheEntry::
CacheEntry(const GeomVertexData *source_data, const GeomMunger *modifier) :
  _source(NULL),
  _source_data(source_data),
  _modifier(modifier),
  _geom_result(NULL),
  _data_result(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheEntry::
CacheEntry(Geom *source, const GeomVertexData *source_data,
           const GeomMunger *modifier, const Geom *geom_result, 
           const GeomVertexData *data_result) :
  _source(source),
  _source_data(source_data),
  _modifier(modifier),
  _geom_result(geom_result),
  _data_result(data_result)
{
  if (_geom_result != _source) {
    _geom_result->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::operator <
//       Access: Public
//  Description: Provides a unique ordering within the set.
////////////////////////////////////////////////////////////////////
INLINE bool Geom::CacheEntry::
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
//     Function: Geom::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CData::
CData() :
  _primitive_type(PT_none),
  _shade_model(SM_uniform),
  _geom_rendering(0),
  _usage_hint(UH_unspecified),
  _got_usage_hint(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CData::
CData(const Geom::CData &copy) :
  _data(copy._data),
  _primitives(copy._primitives),
  _primitive_type(copy._primitive_type),
  _shade_model(copy._shade_model),
  _geom_rendering(copy._geom_rendering),
  _usage_hint(copy._usage_hint),
  _got_usage_hint(copy._got_usage_hint),
  _modified(copy._modified)
{
}

INLINE ostream &
operator << (ostream &out, const Geom &obj) {
  obj.output(out);
  return out;
}
