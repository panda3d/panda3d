// Filename: texCoordName.cxx
// Created by: masad (15Jul04)
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

#include "pandabase.h"
#include "texCoordName.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "preparedGraphicsObjects.h"

TexCoordName::TexCoordsByName TexCoordName::_texcoords_by_name;
CPT(TexCoordName) TexCoordName::_default_name;

TypeHandle TexCoordName::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::Constructor
//       Access: Private
//  Description: Use make() to make a new TexCoordName instance.
////////////////////////////////////////////////////////////////////
TexCoordName::
TexCoordName(const string &name) {
  _name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TexCoordName::
~TexCoordName() {
  TexCoordsByName::iterator ni = _texcoords_by_name.find(_name);
  nassertv(ni != _texcoords_by_name.end());
  _texcoords_by_name.erase(ni);
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::make
//       Access: Published
//  Description: The public interface for constructing a TexCoordName
//               pointer.  This will return a new TexCoordName
//               associated with the indicated name, if this is the
//               first time the particular name has been requested; if
//               the name is already in use, it will return the
//               existing pointer.
////////////////////////////////////////////////////////////////////
const TexCoordName *TexCoordName::
make(const string &name) {
  TexCoordsByName::iterator ni = _texcoords_by_name.find(name);
  if (ni != _texcoords_by_name.end()) {
    return (*ni).second;
  }

  TexCoordName *texcoord = new TexCoordName(name);
  _texcoords_by_name[name] = texcoord;
  return texcoord;
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::output
//       Access: Published
//  Description: output the TexCoordName and its member data
////////////////////////////////////////////////////////////////////
void TexCoordName::
output(ostream &out) const {
  out << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a TexCoordName object
////////////////////////////////////////////////////////////////////
void TexCoordName::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::finalize
//       Access: Public, Virtual
//  Description: Method to ensure that any necessary clean up tasks
//               that have to be performed by this object are performed
////////////////////////////////////////////////////////////////////
void TexCoordName::
finalize() {
  // Unref the pointer that we explicitly reffed in make_from_bam().
  unref();

  // We should never get back to zero after unreffing our own count,
  // because we expect to have been stored in a pointer somewhere.  If
  // we do get to zero, it's a memory leak; the way to avoid this is
  // to call unref_delete() above instead of unref(), but this is
  // dangerous to do from within a virtual function.
  nassertv(get_ref_count() != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TexCoordName is encountered
//               in the Bam file.  It should create the TexCoordName
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TexCoordName::
make_from_bam(const FactoryParams &params) {
  // The process of making a TexCoordName is slightly
  // different than making other Writable objects.
  // That is because all creation of TexCoordNames should
  // be done through the make() constructor.
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  // The name is the only thing written to the data stream.
  string name = scan.get_string();

  // Make a new TexCoordName with that name (or get the previous one
  // if there is one already).
  PT(TexCoordName) me = (TexCoordName *)make(name);

  // But now we have a problem, since we have to hold the reference
  // count and there's no way to return a TypedWritable while still
  // holding the reference count!  We work around this by explicitly
  // upping the count, and also setting a finalize() callback to down
  // it later.
  me->ref();
  manager->register_finalize(me);
  
  return me.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void TexCoordName::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(_name);
}

