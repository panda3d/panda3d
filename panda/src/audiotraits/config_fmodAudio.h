// Filename: config_fmodAudio.h
// Created by:  cort
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

#ifndef CONFIG_FMODAUDIO_H
#define CONFIG_FMODAUDIO_H

#include "pandabase.h"

#ifdef HAVE_FMOD //[
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_fmodAudio, EXPCL_FMOD_AUDIO, EXPTP_FMOD_AUDIO);
NotifyCategoryDecl(fmodAudio, EXPCL_FMOD_AUDIO, EXPTP_FMOD_AUDIO);

extern EXPCL_FMOD_AUDIO void init_libFmodAudio();

#endif //]

#endif // CONFIG_FMODAUDIO_H
