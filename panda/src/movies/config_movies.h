// Filename: config_movies.h
// Created by:  cary (04Jan00)
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

#ifndef __CONFIG_MOVIES_H__
#define __CONFIG_MOVIES_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "dconfig.h"

ConfigureDecl(config_movies, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(movies, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA void init_libmovies();

#endif /* __CONFIG_MOVIES_H__ */
