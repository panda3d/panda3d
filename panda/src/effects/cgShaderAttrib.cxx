// Filename: cgShaderAttrib.cxx
// Created by:  sshodhan (10Jul04)
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

#ifdef HAVE_CG

#include "cgShaderAttrib.h"
#include "config_effects.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"


TypeHandle CgShaderAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::make
//       Access: Published, Static
//  Description: Constructs a new CgShaderAttrib object suitable for
//               process the indicated geometry with shaders
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) CgShaderAttrib::
make(CgShader *shader) {
  CgShaderAttrib *attrib = new CgShaderAttrib;
  attrib->_cg_shader = shader;
  attrib->_always_reissue = true; // This makes the CgShader node issue every frame
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new CgShaderAttrib object suitable for
//               rendering geometry with no shader interference
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) CgShaderAttrib::
make_off() {
  CgShaderAttrib *attrib = new CgShaderAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void CgShaderAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_cg_shader_bind(this);
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived CgShaderAttrib
//               types to specify what the default property for a
//               TexGenAttrib of this type should be.
//
//               This should return a newly-allocated CgShaderAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of CgShaderAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *CgShaderAttrib::
make_default_impl() const {
  return new CgShaderAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived CgShaderAttrib
//               types to return a unique number indicating whether
//               this CgShaderAttrib is equivalent to the other one.
//
//               This should return 0 if the two CgShaderAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two CgShaderAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int CgShaderAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const CgShaderAttrib *cgsa;
  DCAST_INTO_R(cgsa, other, 0);
  
  // Comparing pointers by subtraction is problematic.  Instead of
  // doing this, we'll just depend on the built-in != and < operators
  // for comparing pointers.
  if (_cg_shader != cgsa->_cg_shader) {
    return _cg_shader < cgsa->_cg_shader ? -1 : 1;
  }
  return 0;
}



  /*
////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CgShaderAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    out << _cg_shader->get_name();
  }
}



////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CgShaderAttrib.
////////////////////////////////////////////////////////////////////
void CgShaderAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CgShaderAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _cg_shader);
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CgShaderAttrib is encountered
//               in the Bam file.  It should create the CgShaderAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CgShaderAttrib::
make_from_bam(const FactoryParams &params) {
  CgShaderAttrib *attrib = new CgShaderAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CgShaderAttrib.
////////////////////////////////////////////////////////////////////
void CgShaderAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // Read the _texture pointer.
  manager->read_pointer(scan);
}
*/

#endif  // HAVE_CG
