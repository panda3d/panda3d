// Filename: renderAttrib.h
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef RENDERATTRIB_H
#define RENDERATTRIB_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "indirectCompareTo.h"
#include "pointerTo.h"
#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : RenderAttrib
// Description : This is the base class for a number of render
//               attributes (other than transform) that may be set on
//               scene graph nodes to control the appearance of
//               geometry.  This includes TextureAttrib, ColorAttrib,
//               etc.
//
//               You should not attempt to create or modify a
//               RenderAttrib directly; instead, use the make() method
//               of the appropriate kind of attrib you want.  This
//               will allocate and return a new RenderAttrib of the
//               appropriate type, and it may share pointers if
//               possible.  Do not modify the new RenderAttrib if you
//               wish to change its properties; instead, create a new
//               one.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderAttrib : public TypedWritableReferenceCount {
protected:
  RenderAttrib();
private:
  RenderAttrib(const RenderAttrib &copy);
  void operator = (const RenderAttrib &copy);

public:
  virtual ~RenderAttrib();

  INLINE CPT(RenderAttrib) compose(const RenderAttrib *other) const;
  INLINE CPT(RenderAttrib) make_default() const;
  INLINE int compare_to(const RenderAttrib &other) const;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

protected:
  static CPT(RenderAttrib) return_new(RenderAttrib *attrib);

  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const=0;

private:
  typedef pset<const RenderAttrib *, IndirectCompareTo<RenderAttrib> > Attribs;
  static Attribs _attribs;

  Attribs::iterator _saved_entry;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual void finalize();

protected:
  static TypedWritable *new_from_bam(RenderAttrib *attrib, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "RenderAttrib",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const RenderAttrib &attrib) {
  attrib.output(out);
  return out;
}

#include "renderAttrib.I"

#endif

