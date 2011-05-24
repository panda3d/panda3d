// Filename: config_collada.h
// Created by: Xidram (21Dec10)
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

#ifndef CONFIG_COLLADA_H
#define CONFIG_COLLADA_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"
#include "dconfig.h"

template<class T> class daeTArray;
template<class T> class daeSmartRef;

ConfigureDecl(config_collada, EXPCL_COLLADA, EXPTP_COLLADA);
NotifyCategoryDecl(collada, EXPCL_COLLADA, EXPTP_COLLADA);

extern EXPCL_COLLADA ConfigVariableBool collada_flatten;
extern EXPCL_COLLADA ConfigVariableBool collada_unify;
extern EXPCL_COLLADA ConfigVariableDouble collada_flatten_radius;
extern EXPCL_COLLADA ConfigVariableBool collada_combine_geoms;
extern EXPCL_COLLADA ConfigVariableBool collada_accept_errors;

extern EXPCL_COLLADA void init_libcollada();

#endif
