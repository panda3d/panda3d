// Filename: factoryParams.h
// Created by:  drose (08May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FACTORYPARAMS_H
#define FACTORYPARAMS_H

#include <pandabase.h>

#include "typedObject.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "factoryParam.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : FactoryParams
// Description : An instance of this class is passed to the Factory
//               when requesting it to do its business and construct a
//               new something.  It can be filled with optional
//               parameters to the CreateFunc for the particular
//               subclass the Factory will be creating.
//
//               This is just a vector of pointers to *something*; it
//               will be up to the individual CreateFuncs to interpret
//               this meaningfully.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FactoryParams {
public:
  FactoryParams();
  ~FactoryParams();

  void add_param(FactoryParam *param);
  void clear();

  int get_num_params() const;
  FactoryParam *get_param(int n) const;

  FactoryParam *get_param_of_type(TypeHandle type) const;

private:
  typedef vector< PT(TypedReferenceCount) > Params;

  Params _params;
};

template<class ParamType>
INLINE bool get_param_into(ParamType *&pointer, const FactoryParams &params);

#include "factoryParams.I"

#endif
