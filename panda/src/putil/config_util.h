// Filename: config_util.h
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

#ifndef __CONFIG_UTIL_H__
#define __CONFIG_UTIL_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

class DSearchPath;

ConfigureDecl(config_util, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(util, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(bam, EXPCL_PANDA, EXPTP_PANDA);

// Actually, we can't determine this config variable the normal way,
// because we must be able to access it at static init time.  Instead
// of declaring it a global constant, we'll make it a member of
// MemoryUsage.

//extern EXPCL_PANDA const bool track_memory_usage;

// These are functions instead of constant variables because they are
// computed based on the concatenation of all appearances of the
// corresponding variable in the config files.

BEGIN_PUBLISH
EXPCL_PANDA DSearchPath &get_model_path();
EXPCL_PANDA DSearchPath &get_texture_path();
EXPCL_PANDA DSearchPath &get_sound_path();
END_PUBLISH

#endif /* __CONFIG_UTIL_H__ */
