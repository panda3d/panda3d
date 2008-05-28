// Filename: factoryParams.h
// Created by:  drose (08May00)
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

#ifndef FACTORYPARAMS_H
#define FACTORYPARAMS_H

#include "pandabase.h"

#include "typedObject.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "factoryParam.h"
#include "dcast.h"

#include "pvector.h"

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
class EXPCL_PANDA_PUTIL FactoryParams {
public:
  FactoryParams();
  ~FactoryParams();

  void add_param(FactoryParam *param);
  void clear();

  int get_num_params() const;
  FactoryParam *get_param(int n) const;

  FactoryParam *get_param_of_type(TypeHandle type) const;

private:
  typedef pvector< PT(TypedReferenceCount) > Params;

  Params _params;
};

template<class ParamType>
INLINE bool get_param_into(ParamType *&pointer, const FactoryParams &params);

#include "factoryParams.I"

#endif
