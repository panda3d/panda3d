// Filename: config_recorder.cxx
// Created by:  drose (28Jan04)
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

#include "config_recorder.h"

#include "mouseRecorder.h"
#include "recorderController.h"
#include "recorderFrame.h"
#include "recorderHeader.h"
#include "recorderTable.h"
#include "socketStreamRecorder.h"

#include "dconfig.h"

ConfigureDef(config_recorder);
NotifyCategoryDef(recorder, "");

ConfigureFn(config_recorder) {
  MouseRecorder::init_type();
  RecorderController::init_type();
  RecorderFrame::init_type();
  RecorderHeader::init_type();
  RecorderTable::init_type();
  ReferenceCount::init_type();

  MouseRecorder::register_with_read_factory();
  RecorderFrame::register_with_read_factory();
  RecorderHeader::register_with_read_factory();
  RecorderTable::register_with_read_factory();

#ifdef HAVE_OPENSSL
  SocketStreamRecorder::init_type();
  SocketStreamRecorder::register_with_read_factory();
#endif
}
