// Filename: baseIntegrator.h
// Created by:  charles (11Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BASEINTEGRATOR_H
#define BASEINTEGRATOR_H

#include <pandabase.h>
#include <pointerTo.h>
#include <referenceCount.h>
#include <luse.h>

#include "linearForce.h"
#include "angularForce.h"

#include <vector>

class Physical;

////////////////////////////////////////////////////////////////////
//       Class : BaseIntegrator
// Description : pure virtual integrator class that holds cached
//               matrix information that really should be common to
//               any possible child implementation.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseIntegrator : public ReferenceCount {
private:
  // since the wrt for each physicsobject between its physicalnode
  // and however many forces will be the same among one physical,
  // the transformation matrices can be pulled out of the inner loop
  // and precomputed.
  vector< LMatrix4f > _precomputed_linear_matrices;
  vector< LMatrix4f > _precomputed_angular_matrices;

protected:
  BaseIntegrator(void);

  INLINE const vector< LMatrix4f > &get_precomputed_linear_matrices(void) const;
  INLINE const vector< LMatrix4f > &get_precomputed_angular_matrices(void) const;

  void precompute_linear_matrices(Physical *physical,
				  const vector< PT(LinearForce) > &forces);
  void precompute_angular_matrices(Physical *physical,
				   const vector< PT(AngularForce) > &forces);

public:
  virtual ~BaseIntegrator(void);
};

#include "baseIntegrator.I"

#endif // BASEINTEGRATOR_H
