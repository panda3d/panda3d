// Filename: transformTransition.cxx
// Created by:  drose (24Mar00)
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

#include "transformTransition.h"
#include "transformAttribute.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamWriter.h>
#include <bamReader.h>

TypeHandle TransformTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransformTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *TransformTransition::
make_copy() const {
  return new TransformTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransformAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *TransformTransition::
make_attrib() const {
  return new TransformAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransformTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *TransformTransition::
make_initial() const {
  return new TransformTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TransformTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_transform(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformTransition::make_with_matrix
//       Access: Protected, Virtual
//  Description: Returns a new transition with the indicated matrix.
////////////////////////////////////////////////////////////////////
MatrixTransition<LMatrix4f> *TransformTransition::
make_with_matrix(const LMatrix4f &matrix) const {
  return new TransformTransition(matrix);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformTransition::make_TransformTransition
//       Access: Protected
//  Description: Factory method to generate a TransformTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* TransformTransition::
make_TransformTransition(const FactoryParams &params)
{
  TransformTransition *me = new TransformTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a TransformTransition object
////////////////////////////////////////////////////////////////////
void TransformTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_TransformTransition);
}
