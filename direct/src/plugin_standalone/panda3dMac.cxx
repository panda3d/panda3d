// Filename: panda3dMac.cxx
// Created by:  drose (23Oct09)
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

#include "panda3dMac.h"
#include "load_plugin.h"

#include <iostream>
#include <fstream>
using namespace std;

// Having a global Panda3DMac object just makes things easier.
static Panda3DMac *this_prog;

////////////////////////////////////////////////////////////////////
//     Function: Panda3DMac::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Panda3DMac::
Panda3DMac() : Panda3D(false) {
  // Mac applications traditionally keep running even when all windows
  // are closed.
  _exit_with_last_instance = false;

  // No command-line arguments, so just run.
  if (!post_arg_processing()) {
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3DMac::open_p3d_file
//       Access: Public
//  Description: Opens a p3d file received via the "open documents"
//               event as its own instance.
////////////////////////////////////////////////////////////////////
void Panda3DMac::
open_p3d_file(FSRef *ref) {
  OSErr err;

  static const size_t buffer_size = 4096;
  UInt8 filename[buffer_size];
  err = FSRefMakePath(ref, filename, buffer_size);
  if (err) {
    cerr << "Couldn't get filename\n";
    return;
  }

  // Create an instance.
  create_instance((char *)filename, true, NULL, 0);
}

static pascal OSErr
open_documents_handler(const AppleEvent *theAppleEvent, AppleEvent *reply, 
                       SRefCon handlerRefcon) {
  AEDescList docList;
  FSRef theFSRef;
  long index;
  long count = 0;
  
  // Get the list of file aliases from the event.
  OSErr err = AEGetParamDesc(theAppleEvent,
                             keyDirectObject, typeAEList, &docList);
  require_noerr(err, CantGetDocList);
  
  err = AECountItems(&docList, &count);
  require_noerr(err, CantGetCount);
  
  for (index = 1; index <= count; index++) {
    err = AEGetNthPtr(&docList, index, typeFSRef,
                      NULL, NULL, &theFSRef, sizeof(FSRef), NULL);// 5
    require_noerr(err, CantGetDocDescPtr);
    
    // Here's the file, do something with it.
    this_prog->open_p3d_file(&theFSRef);
  }
  
  // Release list of files
  AEDisposeDesc(&docList);
  
  // Error handlers.
 CantGetDocList:
 CantGetCount:
 CantGetDocDescPtr:
  return (err);
}

int
main(int argc, char *argv[]) {
  OSErr err;
  AEEventHandlerUPP handler;

  this_prog = new Panda3DMac;
  handler = NewAEEventHandlerUPP(open_documents_handler);
  err = AEInstallEventHandler
    (kCoreEventClass, kAEOpenDocuments, handler, 0, false);

  // The command-line options are weird when we start from the
  // Launcher.  Just ignore them.
  this_prog->run_main_loop();
  return 0;
}
