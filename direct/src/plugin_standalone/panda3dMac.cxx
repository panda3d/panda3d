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

  // Get the size and basename of the file.
  FSCatalogInfo catalog_info;
  HFSUniStr255 basename_unicode;

  err = FSGetCatalogInfo(ref, kFSCatInfoDataSizes, &catalog_info,
                         &basename_unicode, NULL, NULL);
  if (err) {
    cerr << "Couldn't query file information.\n";
    return;
  }

  // A poor-man's unicode-to-ascii conversion.
  string basename;
  for (int i = 0; i < basename_unicode.length; ++i) {
    basename += (char)basename_unicode.unicode[i];
  }
  size_t data_size = (size_t)catalog_info.dataLogicalSize;

  // We could try to figure out full pathname of the p3d file we've
  // got here, but it's probably better just to open the file and read
  // it.  This way, it works regardless of the source of the p3d file,
  // even if it's not actually a file on disk.
  FSIORefNum io_ref;
  err = FSOpenFork(ref, 0, NULL, fsRdPerm, &io_ref);
  if (!err) {
    // Create an instance, and tell it we'll be sending it the p3d
    // data in a forthcoming stream.
    P3D_instance *inst = create_instance
      (basename.c_str(), false, 
       _win_x, _win_y, _win_width, _win_height,
       NULL, 0);
    int stream_id = P3D_instance_start_stream(inst, basename.c_str());

    // Now start to read the data.
    static const size_t buffer_size = 8192;
    static char buffer[buffer_size];
    ByteCount read_count;
    err = FSReadFork(io_ref, fsAtMark, 0, buffer_size, buffer, &read_count);
    while (read_count != 0) {
      P3D_instance_feed_url_stream(inst, stream_id, P3D_RC_in_progress, 0,
                                   data_size, buffer, read_count);
      err = FSReadFork(io_ref, fsAtMark, 0, buffer_size, buffer, &read_count);
    }

    P3D_result_code status = P3D_RC_done;
    if (err != eofErr) {
      status = P3D_RC_generic_error;
      cerr << "Error reading file\n";
    }
    
    P3D_instance_feed_url_stream
      (inst, stream_id, status, 0, data_size, NULL, 0);
  }
}

static pascal OSErr
open_documents_handler(const AppleEvent *theAppleEvent, AppleEvent *reply, 
                       long handlerRefcon) {
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
    (kCoreEventClass, kAEOpenDocuments, handler, NULL, false);

  return this_prog->run_command_line(argc, argv);
}
