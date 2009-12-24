// Filename: config_awesomium.cxx
// Created by:  rurbino (12Oct09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source cawesomium in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_awesomium.h"
#include "awWebCore.h"
#include "awWebView.h"
#include "awWebViewListener.h"
#include "dconfig.h"


Configure(config_awesomium);
NotifyCategoryDef(awesomium, "");


ConfigureFn(config_awesomium) {
  init_libawesomium();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libawesomium
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libawesomium() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
  
  AwWebCore::init_type();
  AwWebView::init_type();
  AwWebViewListener::init_type();

}
