// Filename: mayaApi.cxx
// Created by:  drose (15Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "mayaApi.h"
#include "config_maya.h"

#include "pre_maya_include.h"
#include <maya/MGlobal.h>
#include <maya/MDistance.h>
#include <maya/MFileIO.h>
#include <maya/MLibrary.h>
#include <maya/MStatus.h>
#include "post_maya_include.h"

MayaApi *MayaApi::_global_api = (MayaApi *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::Constructor
//       Access: Protected
//  Description: Don't attempt to create this object directly;
//               instead, use the open_api() method.
////////////////////////////////////////////////////////////////////
MayaApi::
MayaApi(const string &program_name) {
  MStatus stat = MLibrary::initialize((char *)program_name.c_str());
  if (!stat) {
    stat.perror("MLibrary::initialize");
    _is_valid = false;
  } else {
    _is_valid = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::Copy Constructor
//       Access: Protected
//  Description: Don't attempt to copy MayaApi objects.  There should
//               be only one of these in the world at a time.
////////////////////////////////////////////////////////////////////
MayaApi::
MayaApi(const MayaApi &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::Copy Assignment Operator
//       Access: Protected
//  Description: Don't attempt to copy MayaApi objects.  There should
//               be only one of these in the world at a time.
////////////////////////////////////////////////////////////////////
void MayaApi::
operator = (const MayaApi &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaApi::
~MayaApi() {
  nassertv(_global_api == this);
  if (_is_valid) {
    // Caution!  Calling this function seems to call exit() somewhere
    // within Maya code.
    MLibrary::cleanup();
  }
  _global_api = (MayaApi *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::open_api
//       Access: Public, Static
//  Description: Opens the Maya API, if it is not already open, and
//               returns a pointer representing this connection.  When
//               you are done using the Maya API, let the pointer
//               destruct.
//
//               If program_name is supplied, it is passed to Maya as
//               the name of the currently-executing program.
//               Otherwise, the current program name is extracted from
//               the execution environment, if possible.
////////////////////////////////////////////////////////////////////
PT(MayaApi) MayaApi::
open_api(string program_name) {
  if (_global_api == (MayaApi *)NULL) {
    // We need to create a new MayaApi object.
    if (program_name.empty()) {
      program_name = ExecutionEnvironment::get_binary_name();
      if (program_name.empty()) {
        program_name = "Panda";
      }
    }

    _global_api = new MayaApi(program_name);
  }

  return _global_api;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::is_valid
//       Access: Public
//  Description: Returns true if the API has been successfully opened
//               and may be used, or false if there is some problem.
////////////////////////////////////////////////////////////////////
bool MayaApi::
is_valid() const {
  return _is_valid;
}

#ifdef WIN32
static string
back_to_front_slash(const string &str) {
  string result = str;
  string::iterator si;
  for (si = result.begin(); si != result.end(); ++si) {
    if ((*si) == '\\') {
      (*si) = '/';
    }
  }

  return result;
}
#endif  // WIN32

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::read
//       Access: Public
//  Description: Reads the indicated maya file into the global model
//               space.  Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool MayaApi::
read(const Filename &filename) {
  MFileIO::newFile(true);

  maya_cat.info() << "Reading " << filename << "\n";
  // Load the file into Maya
  string os_filename = filename.to_os_specific();

#ifdef WIN32
  // Actually, Maya seems to want forward slashes, even on Windows.
  os_filename = back_to_front_slash(os_filename);
#endif

  MStatus stat = MFileIO::open(os_filename.c_str());
  if (!stat) {
    stat.perror(filename.c_str());
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::clear
//       Access: Public
//  Description: Resets the global model space to the empty state, for
//               instance in preparation for building a new file.
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool MayaApi::
clear() {
  MStatus stat = MFileIO::newFile(true);
  if (!stat) {
    stat.perror("clear");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::get_units
//       Access: Public
//  Description: Returns Maya's internal units in effect.
////////////////////////////////////////////////////////////////////
DistanceUnit MayaApi::
get_units() {
  switch (MDistance::internalUnit()) {
  case MDistance::kInches:
    return DU_inches;
  case MDistance::kFeet:
    return DU_feet;
  case MDistance::kYards:
    return DU_yards;
  case MDistance::kMiles:
    return DU_statute_miles;
  case MDistance::kMillimeters:
    return DU_millimeters;
  case MDistance::kCentimeters:
    return DU_centimeters;
  case MDistance::kKilometers:
    return DU_kilometers;
  case MDistance::kMeters:
    return DU_meters;

  default:
    return DU_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaApi::get_coordinate_system
//       Access: Public
//  Description: Returns Maya's internal coordinate system in effect.
////////////////////////////////////////////////////////////////////
CoordinateSystem MayaApi::
get_coordinate_system() {
  if (MGlobal::isYAxisUp()) {
    return CS_yup_right;
  } else {
    return CS_zup_right;
  }
}
