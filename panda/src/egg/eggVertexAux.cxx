// Filename: eggVertexAux.cxx
// Created by:  jenes (15Nov11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "eggVertexAux.h"
#include "eggParameters.h"

#include "indent.h"

TypeHandle EggVertexAux::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexAux::
EggVertexAux(const string &name, const LVecBase4d &aux) :
  EggNamedObject(name),
  _aux(aux)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexAux::
EggVertexAux(const EggVertexAux &copy) :
  EggNamedObject(copy),
  _aux(copy._aux)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexAux &EggVertexAux::
operator = (const EggVertexAux &copy) {
  EggNamedObject::operator = (copy);
  _aux = copy._aux;

  return (*this);
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexAux::
~EggVertexAux() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::make_average
//       Access: Published, Static
//  Description: Creates a new EggVertexAux that contains the
//               averaged values of the two given objects.  It is
//               an error if they don't have the same name.
///////////////////////////////////////////////////////////////////
PT(EggVertexAux) EggVertexAux::
make_average(const EggVertexAux *first, const EggVertexAux *second) {
  nassertr(first->get_name() == second->get_name(), NULL);

  LVecBase4d aux = (first->_aux + second->_aux) / 2;
  return new EggVertexAux(first->get_name(), aux);
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggVertexAux::
write(ostream &out, int indent_level) const {
  string inline_name = get_name();
  if (!inline_name.empty()) {
    inline_name += ' ';
  }
  indent(out, indent_level)
    << "<Aux> " << inline_name << "{ " << get_aux() << " }\n";
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::compare_to
//       Access: Public
//  Description: An ordering operator to compare two vertices for
//               sorting order.  This imposes an arbitrary ordering
//               useful to identify unique vertices.
////////////////////////////////////////////////////////////////////
int EggVertexAux::
compare_to(const EggVertexAux &other) const {
  int compare;
  compare = _aux.compare_to(other._aux, egg_parameters->_pos_threshold);
  if (compare != 0) {
    return compare;
  }

  return 0;
}
