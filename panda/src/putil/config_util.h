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
#include "configVariableSearchPath.h"
#include "configVariableEnum.h"
#include "bamEndian.h"
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
extern EXPCL_PANDA ConfigVariableEnum<BamEndian> bam_endian;

extern EXPCL_PANDA ConfigVariableSearchPath model_path;
extern EXPCL_PANDA ConfigVariableSearchPath texture_path;
extern EXPCL_PANDA ConfigVariableSearchPath sound_path;

// The above variables are also shadowed by these functions, so that
// they can easily be accessed in the interpreter (e.g. Python).
BEGIN_PUBLISH
EXPCL_PANDA ConfigVariableSearchPath &get_model_path();
EXPCL_PANDA ConfigVariableSearchPath &get_texture_path();
EXPCL_PANDA ConfigVariableSearchPath &get_sound_path();
END_PUBLISH

#endif /* __CONFIG_UTIL_H__ */
