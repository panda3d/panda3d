/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseIntegrator.h
 * @author charles
 * @date 2000-08-11
 */

#ifndef BASEINTEGRATOR_H
#define BASEINTEGRATOR_H

#include "pandabase.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "luse.h"

#include "linearForce.h"
#include "angularForce.h"

#include "pvector.h"
#include "epvector.h"

class Physical;

/**
 * pure virtual integrator class that holds cached matrix information that
 * really should be common to any possible child implementation.
 */
class EXPCL_PANDA_PHYSICS BaseIntegrator : public ReferenceCount {
public:
  typedef epvector<LMatrix4> MatrixVector;
  typedef pvector<PT(LinearForce)> LinearForceVector;
  typedef pvector<PT(AngularForce)> AngularForceVector;

  virtual ~BaseIntegrator();

PUBLISHED:
  virtual void output(std::ostream &out) const;
  virtual void write_precomputed_linear_matrices(std::ostream &out,
                                                 int indent=0) const;
  virtual void write_precomputed_angular_matrices(std::ostream &out,
                                                  int indent=0) const;
  virtual void write(std::ostream &out, int indent=0) const;

protected:
  BaseIntegrator();

  INLINE const MatrixVector &get_precomputed_linear_matrices() const;
  INLINE const MatrixVector &get_precomputed_angular_matrices() const;

  void precompute_linear_matrices(Physical *physical,
                                  const LinearForceVector &forces);
  void precompute_angular_matrices(Physical *physical,
                                   const AngularForceVector &forces);

private:
  // since the wrt for each physicsobject between its physicalnode and however
  // many forces will be the same among one physical, the transformation
  // matrices can be pulled out of the inner loop and precomputed.
  MatrixVector _precomputed_linear_matrices;
  MatrixVector _precomputed_angular_matrices;
};

#include "baseIntegrator.I"

#endif // BASEINTEGRATOR_H
