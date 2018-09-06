/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_recorder.cxx
 * @author drose
 * @date 2004-01-28
 */

#include "config_recorder.h"

#include "mouseRecorder.h"
#include "recorderController.h"
#include "recorderFrame.h"
#include "recorderHeader.h"
#include "recorderTable.h"
#include "socketStreamRecorder.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_RECORDER)
  #error Buildsystem error: BUILDING_PANDA_RECORDER not defined
#endif

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
