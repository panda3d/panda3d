// Filename: texGenAttrib.cxx
// Created by:  masad (21Jun04)
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

#include "texGenAttrib.h"
#include "texturePool.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle TexGenAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make
//       Access: Published, Static
//  Description: Constructs a new TexGenAttrib object suitable for
//               rendering the indicated texture onto geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
make(Mode mode) {
  TexGenAttrib *attrib = new TexGenAttrib(mode);
#if 0
  attrib->_texture = TexturePool::load_texture("/usr/masad/player/pmockup/maps/moon-card.rgb", 1);
  if (attrib->_texture)
    pgraph_cat.debug() << *(attrib->_texture) << endl;
  else
    return (TexGenAttrib *)NULL;
#endif
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new TexGenAttrib object suitable for
//               rendering untextured geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexGenAttrib::
make_off() {
  TexGenAttrib *attrib = new TexGenAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_tex_gen(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    switch (get_mode()) {
    case M_spherical:
      out << "spherical";
      break;
    case M_cubic:
      out << "cubic";
      break;
    }      
      //out << _texture->get_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexGenAttrib
//               types to return a unique number indicating whether
//               this TexGenAttrib is equivalent to the other one.
//
//               This should return 0 if the two TexGenAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two TexGenAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int TexGenAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TexGenAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  
  // Comparing pointers by subtraction is problematic.  Instead of
  // doing this, we'll just depend on the built-in != and < operators
  // for comparing pointers.
  return (int)_mode - (int)ta->_mode;
  /*
  if (_texture != ta->_texture) {
    return _texture < ta->_texture ? -1 : 1;
  }
  return 0;
  */
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexGenAttrib
//               types to specify what the default property for a
//               TexGenAttrib of this type should be.
//
//               This should return a newly-allocated TexGenAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of TexGenAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *TexGenAttrib::
make_default_impl() const {
  return new TexGenAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TexGenAttrib.
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
  //manager->write_pointer(dg, _texture);
}

/*
////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TexGenAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  TypedWritable *texture = p_list[pi++];
  if (texture != (TypedWritable *)NULL) {
    _texture = DCAST(Texture, texture);
  }

  return pi;
}
*/
////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TexGenAttrib is encountered
//               in the Bam file.  It should create the TexGenAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TexGenAttrib::
make_from_bam(const FactoryParams &params) {
  TexGenAttrib *attrib = new TexGenAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TexGenAttrib.
////////////////////////////////////////////////////////////////////
void TexGenAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
  // Read the _texture pointer.
  //manager->read_pointer(scan);
}
