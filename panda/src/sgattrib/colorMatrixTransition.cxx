// Filename: colorMatrixTransition.cxx
// Created by:  jason (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "colorMatrixTransition.h"
#include "colorMatrixAttribute.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamWriter.h>
#include <bamReader.h>

TypeHandle ColorMatrixTransition::_type_handle;

////////////////////////////////////////////////////////////////////s
//     Function: ColorMatrixTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorMatrixTransition::
make_copy() const {
  return new ColorMatrixTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMatrixTransition::
make_attrib() const {
  return new ColorMatrixAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::make_with_matrix
//       Access: Protected, Virtual
//  Description: Returns a new transition with the indicated matrix.
////////////////////////////////////////////////////////////////////
MatrixTransition<LMatrix4f> *ColorMatrixTransition::
make_with_matrix(const LMatrix4f &matrix) const {
  return new ColorMatrixTransition(matrix);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::make_ColorMatrixTransition
//       Access: Protected
//  Description: Factory method to generate a ColorMatrixTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* ColorMatrixTransition::
make_ColorMatrixTransition(const FactoryParams &params)
{
  ColorMatrixTransition *me = new ColorMatrixTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a ColorMatrixTransition object
////////////////////////////////////////////////////////////////////
void ColorMatrixTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_ColorMatrixTransition);
}
