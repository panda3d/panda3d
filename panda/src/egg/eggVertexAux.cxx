/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexAux.cxx
 * @author jenes
 * @date 2011-11-15
 */

#include "eggVertexAux.h"
#include "eggParameters.h"

#include "indent.h"

TypeHandle EggVertexAux::_type_handle;

/**
 *
 */
EggVertexAux::
EggVertexAux(const std::string &name, const LVecBase4d &aux) :
  EggNamedObject(name),
  _aux(aux)
{
}

/**
 *
 */
EggVertexAux::
EggVertexAux(const EggVertexAux &copy) :
  EggNamedObject(copy),
  _aux(copy._aux)
{
}

/**
 *
 */
EggVertexAux &EggVertexAux::
operator = (const EggVertexAux &copy) {
  EggNamedObject::operator = (copy);
  _aux = copy._aux;

  return (*this);
}

/**
 *
 */
EggVertexAux::
~EggVertexAux() {
}

/**
 * Creates a new EggVertexAux that contains the averaged values of the two
 * given objects.  It is an error if they don't have the same name.
 */
PT(EggVertexAux) EggVertexAux::
make_average(const EggVertexAux *first, const EggVertexAux *second) {
  nassertr(first->get_name() == second->get_name(), nullptr);

  LVecBase4d aux = (first->_aux + second->_aux) / 2;
  return new EggVertexAux(first->get_name(), aux);
}

/**
 *
 */
void EggVertexAux::
write(std::ostream &out, int indent_level) const {
  std::string inline_name = get_name();
  if (!inline_name.empty()) {
    inline_name += ' ';
  }
  indent(out, indent_level)
    << "<Aux> " << inline_name << "{ " << get_aux() << " }\n";
}

/**
 * An ordering operator to compare two vertices for sorting order.  This
 * imposes an arbitrary ordering useful to identify unique vertices.
 */
int EggVertexAux::
compare_to(const EggVertexAux &other) const {
  int compare;
  compare = _aux.compare_to(other._aux, egg_parameters->_pos_threshold);
  if (compare != 0) {
    return compare;
  }

  return 0;
}
