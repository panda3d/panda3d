// Filename: textureAttrib.cxx
// Created by:  drose (21Feb02)
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

#include "textureAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle TextureAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make
//       Access: Published, Static
//  Description: Constructs a new TextureAttrib object suitable for
//               rendering the indicated texture onto geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
make(Texture *texture) {
  TextureAttrib *attrib = new TextureAttrib;
  attrib->_texture = texture;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new TextureAttrib object suitable for
//               rendering untextured geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
make_off() {
  TextureAttrib *attrib = new TextureAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void TextureAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_texture(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TextureAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    out << _texture->get_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TextureAttrib
//               types to return a unique number indicating whether
//               this TextureAttrib is equivalent to the other one.
//
//               This should return 0 if the two TextureAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two TextureAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int TextureAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TextureAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  
  // Comparing pointers by subtraction is problematic.  Instead of
  // doing this, we'll just depend on the built-in != and < operators
  // for comparing pointers.
  if (_texture != ta->_texture) {
    return _texture < ta->_texture ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TextureAttrib
//               types to specify what the default property for a
//               TextureAttrib of this type should be.
//
//               This should return a newly-allocated TextureAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of TextureAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *TextureAttrib::
make_default_impl() const {
  return new TextureAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TextureAttrib.
////////////////////////////////////////////////////////////////////
void TextureAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TextureAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TextureAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  TypedWritable *texture = p_list[pi++];
  if (texture != (TypedWritable *)NULL) {
    _texture = DCAST(Texture, texture);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TextureAttrib is encountered
//               in the Bam file.  It should create the TextureAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TextureAttrib::
make_from_bam(const FactoryParams &params) {
  TextureAttrib *attrib = new TextureAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TextureAttrib.
////////////////////////////////////////////////////////////////////
void TextureAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // Read the _texture pointer.
  manager->read_pointer(scan);
}
