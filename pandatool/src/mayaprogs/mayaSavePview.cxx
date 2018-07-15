/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaSavePview.cxx
 * @author drose
 * @date 2003-10-27
 */

#include "mayaSavePview.h"

#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MFileIO.h>
#include <maya/MArgParser.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>

#include <stdlib.h>

#ifdef WIN32_VC
#include <process.h>
#endif

/**
 *
 */
MayaSavePview::
MayaSavePview() {
}

/**
 * Called when the plugin command is invoked.
 */
MStatus MayaSavePview::
doIt(const MArgList &args) {
  MStatus result;

  // First, parse the plugin arguments.
  MSyntax syntax;
  syntax.addFlag("a", "animate");

  MArgParser parser(syntax, args, &result);
  if (!result) {
    result.perror("arguments");
    return result;
  }

  bool animate = parser.isFlagSet("a", &result);
  if (!result) {
    result.perror("isFlagSet");
    return result;
  }

  // Now make sure the current buffer is saved.
  result = MFileIO::save(false);
  if (result != MS::kSuccess) {
    return result;
  }

  MString filename = MFileIO::currentFile();

  MString pview_args = "-cl";
  if (animate) {
    pview_args = "-cla";
  }

#ifdef WIN32_VC
  // On Windows, we use the spawn function to run pview asynchronously.
  MString quoted = MString("\"") + filename + MString("\"");
  intptr_t retval = _spawnlp(_P_DETACH, "pview",
                             "pview", pview_args.asChar(), quoted.asChar(), nullptr);
  if (retval == -1) {
    return MS::kFailure;
  }

#else  // WIN32_VC
  // On non-Windows (e.g.  Unix), we just use the system function, which runs
  // synchronously.  We could fork a process, but no one's asked for this yet.
  MString command = MString("pview " + pview_args + MString(" \"") + filename + MString("\""));

  int command_result = system(command.asChar());
  if (command_result != 0) {
    return MS::kFailure;
  }
#endif // WIN32_VC

  return MS::kSuccess;
}

/**
 * This is used to create a new instance of the plugin.
 */
void *MayaSavePview::
creator() {
  return new MayaSavePview;
}



/**
 * Called by Maya when the plugin is loaded.
 */
EXPCL_MISC MStatus
initializePlugin(MObject obj) {
  MFnPlugin plugin(obj, "VR Studio", "1.0");
  MStatus status;
  status = plugin.registerCommand("pview", MayaSavePview::creator);
  if (!status) {
    status.perror("registerCommand");
  }

  return status;
}

/**
 * Called by Maya when the plugin is unloaded.
 */
EXPCL_MISC MStatus
uninitializePlugin(MObject obj) {
  MFnPlugin plugin(obj);
  MStatus status;
  status = plugin.deregisterCommand("pview");

  if (!status) {
    status.perror("deregisterCommand");
  }
  return status;
}
