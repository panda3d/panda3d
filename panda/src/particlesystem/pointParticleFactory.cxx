// Filename: pointParticleFactory.cxx
// Created by:  charles (05Jul00)
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

#include "pointParticleFactory.h"
#include "pointParticle.h"

#include <stdlib.h>

////////////////////////////////////////////////////////////////////
//    Function : PointParticleFactory
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////
PointParticleFactory::
PointParticleFactory() :
  BaseParticleFactory() {
}

////////////////////////////////////////////////////////////////////
//    Function : PointParticleFactory
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
PointParticleFactory::
PointParticleFactory(const PointParticleFactory &copy) :
  BaseParticleFactory(copy) {
}

////////////////////////////////////////////////////////////////////
//    Function : ~PointParticleFactory
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////
PointParticleFactory::
~PointParticleFactory() {
}

////////////////////////////////////////////////////////////////////
//    Function : populate_child_particle
//      Access : public
// Description : child particle generation function
////////////////////////////////////////////////////////////////////
void PointParticleFactory::
populate_child_particle(BaseParticle *bp) const {
  bp->set_oriented(false);
}

////////////////////////////////////////////////////////////////////
//    Function : alloc_particle
//      Access : public
// Description : child particle generation function
////////////////////////////////////////////////////////////////////
BaseParticle *PointParticleFactory::
alloc_particle() const {
  return new PointParticle;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PointParticleFactory::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PointParticleFactory";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PointParticleFactory::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PointParticleFactory:\n";
  BaseParticleFactory::write(out, indent+2);
  #endif //] NDEBUG
}
