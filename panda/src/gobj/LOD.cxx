// Filename: LOD.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "LOD.h"

#include <datagram.h>
#include <datagramIterator.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle LOD::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LOD::constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LOD::
LOD(void) {
  _center.set(0, 0, 0);
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
  LVector3f y = mat.get_row3(1);
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
//		 the corresponding list of switching distances. 
////////////////////////////////////////////////////////////////////
int LOD::
compute_child(const LPoint3f &cam_pos, const LPoint3f &center) const {
  LVector3f v = cam_pos - center;
  float dist = dot(v, v);
  LODSwitchVector::const_iterator i;
  int child = 0;
  for (i = _switch_vector.begin(), child = 0; 
       i != _switch_vector.end(); ++i, ++child) {
    if ((*i).in_range(dist))
      break;
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
