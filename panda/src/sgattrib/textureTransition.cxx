// Filename: textureTransition.cxx
// Created by:  drose (06Feb99)
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

#include "config_sgattrib.h"

#include "textureTransition.h"
#include "textureAttribute.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle TextureTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *TextureTransition::
make_copy() const {
  return new TextureTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *TextureTransition::
make_attrib() const {
  return new TextureAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *TextureTransition::
make_initial() const {
  return new TextureTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TextureTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_texture(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another TextureTransition.
////////////////////////////////////////////////////////////////////
void TextureTransition::
set_value_from(const OnOffTransition *other) {
  const TextureTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  nassertv(_value != (Texture *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int TextureTransition::
compare_values(const OnOffTransition *other) const {
  const TextureTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return (int)(_value - ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void TextureTransition::
output_value(ostream &out) const {
  nassertv(_value != (Texture *)NULL);
  out << *_value;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void TextureTransition::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (Texture *)NULL);
  indent(out, indent_level) << *_value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a TextureTransition object
////////////////////////////////////////////////////////////////////
void TextureTransition::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_TextureTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void TextureTransition::
write_datagram(BamWriter *manager, Datagram &me) {
  OnOffTransition::write_datagram(manager, me);
  manager->write_pointer(me, _value);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int TextureTransition::
complete_pointers(vector_typedWritable &p_list, BamReader *) {
  if (p_list[0] == TypedWritable::Null) {
    if (sgattrib_cat.is_debug()) {
      sgattrib_cat->debug()
    << get_type().get_name() << " received null Texture,"
    << " turning off" << endl;
    }
    _value = (Texture *)NULL;
    set_off();

  } else {
    _value = DCAST(Texture, p_list[0]);
  }

  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::make_TextureTransition
//       Access: Protected
//  Description: Factory method to generate a TextureTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* TextureTransition::
make_TextureTransition(const FactoryParams &params) {
  TextureTransition *me = new TextureTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void TextureTransition::
fillin(DatagramIterator& scan, BamReader* manager) {
  OnOffTransition::fillin(scan, manager);
  manager->read_pointer(scan, this);
}
