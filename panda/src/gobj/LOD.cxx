// Filename: LOD.cxx
// Created by:  mike (09Jan97)
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
#include "LOD.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "indent.h"
#include "config_gobj.h"

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE LODSwitch
#define NAME LODSwitchVector

#include "vector_src.cxx"

float LOD::_stress_factor = lod_stress_factor;

////////////////////////////////////////////////////////////////////
//     Function: LOD::constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LOD::
LOD(void) {
  _center.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LOD::
LOD(const LOD &copy) :
  _center(copy._center),
  _switch_vector(copy._switch_vector)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LOD::
~LOD(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::xform
//       Access: Public
//  Description: Transforms the LOD specification by the indicated
//               matrix.
////////////////////////////////////////////////////////////////////
void LOD::
xform(const LMatrix4f &mat) {
  _center = _center * mat;

  // We'll take just the length of the y axis as the matrix's scale.
  LVector3f y;
  mat.get_row3(y,1);
  float factor_squared = y.length_squared();

  LODSwitchVector::iterator si;
  for (si = _switch_vector.begin(); si != _switch_vector.end(); ++si) {
    (*si).rescale(factor_squared);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::compute_child
//       Access: Public
//  Description: Computes the distance between two points and returns
//               the index for the child of the LOD by testing against
//               the corresponding list of switching distances.
////////////////////////////////////////////////////////////////////
int LOD::
compute_child(const LPoint3f &cam_pos, const LPoint3f &center) const {

  LVector3f v = cam_pos - center;
  float dist = dot(v, v) * _stress_factor;
  LODSwitchVector::const_iterator i;
  int child = 0;
  for (i = _switch_vector.begin(), child = 0;
       i != _switch_vector.end(); ++i, ++child) {
    if ((*i).in_range(dist))
      break;
  }

  if(debug_LOD_mode) {
      //not ifndef NDEBUG'ing this out since need it at Opt4 for perf measurements
      if(select_LOD_number>=0) {
          return select_LOD_number;
      }

      // since lowest level LOD is lev 0, must invert the meaning of
      // so minimum_LOD_number 0 will screen out no LODs, and increasing it
      // will screen out successively higher levels
      int max_allowed_LOD_number = _switch_vector.size() - minimum_LOD_number;
      if(child > max_allowed_LOD_number) {
          child = max_allowed_LOD_number;
      }
  }

  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::write_datagram
//       Access: Public
//  Description: Writes the contents of the LOD out to the datagram,
//               presumably in preparation to writing to a Bam file.
////////////////////////////////////////////////////////////////////
void LOD::
write_datagram(Datagram &destination) const {
  _center.write_datagram(destination);

  destination.add_uint16(_switch_vector.size());

  LODSwitchVector::const_iterator si;
  for (si = _switch_vector.begin();
       si != _switch_vector.end();
       ++si) {
    (*si).write_datagram(destination);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::read_datagram
//       Access: Public
//  Description: Reads the contents of the LOD from the datagram,
//               presumably in response to reading a Bam file.
////////////////////////////////////////////////////////////////////
void LOD::
read_datagram(DatagramIterator &source) {
  _center.read_datagram(source);

  _switch_vector.clear();

  int num_switches = source.get_uint16();
  _switch_vector.reserve(num_switches);
  for (int i = 0; i < num_switches; i++) {
    _switch_vector.push_back(LODSwitch(0, 0));
    _switch_vector.back().read_datagram(source);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LOD::
output(ostream &out) const {
  if (_switch_vector.empty()) {
    out << "no switches.";
  } else {
    LODSwitchVector::const_iterator si;
    si = _switch_vector.begin();
    out << "(" << (*si).get_in() << "/" << (*si).get_out() << ")";
    ++si;
    while (si != _switch_vector.end()) {
      out << " (" << (*si).get_in() << "/" << (*si).get_out() << ")";
      ++si;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LOD::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "LOD, " << _switch_vector.size() << " switches:\n";
  LODSwitchVector::const_iterator si;
  int i = 0;
  for (si = _switch_vector.begin();
       si != _switch_vector.end();
       ++si) {
    indent(out, indent_level + 2)
      << i << ". in at " << (*si).get_in()
      << ", out at " << (*si).get_out() << "\n";
    i++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::set_stress_factor
//       Access: Published, Static
//  Description: Sets the factor that globally scales all LOD's.  This
//               factor is applied to the square of the LOD distance,
//               so the larger the number, the lower the detail that
//               is presented.  The normal value is 1.0.
////////////////////////////////////////////////////////////////////
void LOD::
set_stress_factor(float stress_factor) {
  _stress_factor = stress_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: LOD::get_stress_factor
//       Access: Published, Static
//  Description: Returns the factor that globally scales all LOD's.
//               See get_stress_factor().
////////////////////////////////////////////////////////////////////
float LOD::
get_stress_factor() {
  return _stress_factor;
}
