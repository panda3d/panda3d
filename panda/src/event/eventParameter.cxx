/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventParameter.cxx
 * @author drose
 * @date 1999-02-08
 */

#include "eventParameter.h"
#include "dcast.h"

template class ParamValue<int>;
template class ParamValue<double>;

/**
 *
 */
void EventParameter::
output(std::ostream &out) const {
  if (_ptr == nullptr) {
    out << "(empty)";

  } else if (_ptr->is_of_type(ParamValueBase::get_class_type())) {
    const ParamValueBase *sv_ptr;
    DCAST_INTO_V(sv_ptr, _ptr);
    sv_ptr->output(out);

  } else {
    out << _ptr->get_type();
  }
}
