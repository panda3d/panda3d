/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaApi.cxx
 * @author drose
 * @date 2002-04-15
 */

#include "mayaApi.h"
#include "config_maya.h"
#include "string_utils.h"
#include "thread.h"

#include "pre_maya_include.h"
#include <maya/MGlobal.h>
#include <maya/MDistance.h>
#include <maya/MFileIO.h>
#include <maya/MLibrary.h>
#include <maya/MStatus.h>
#include <maya/MFnAnimCurve.h>
#include "post_maya_include.h"

#ifdef WIN32_VC
#include <direct.h>  // for chdir()
#endif

using std::string;

MayaApi *MayaApi::_global_api = nullptr;

// We need this bogus object just to force the application to link with
// OpenMayaAnim.lib; otherwise, Maya will complain (when compiled on Windows)
// that it is unable to find source plug 'ikRPsolver.msg'.
static MFnAnimCurve force_link_with_OpenMayaAnim;

/**
 * Don't attempt to create this object directly; instead, use the open_api()
 * method.
 */
MayaApi::
MayaApi(const string &program_name, bool view_license, bool revert_dir) {
  if (program_name == "plug-in") {
    // In this special case, we are invoking the code from within a plug-in,
    // so we need not (and should not) call MLibrary::initialize().
    _plug_in = true;
    _is_valid = true;
    return;
  }

  // Otherwise, if program_name is any other name, we are invoking the code
  // from a standalone application and we do need to call
  // MLibrary::initialize().
  _plug_in = false;

  // Beginning with Maya4.5, the call to initialize seems to change the
  // current directory!  Yikes!

  // Furthermore, the current directory may change during the call to any Maya
  // function!  Egad!
  _cwd = ExecutionEnvironment::get_cwd();
  MStatus stat = MLibrary::initialize(false, (char *)program_name.c_str(), view_license);

  int error_count = init_maya_repeat_count;
  while (!stat && error_count > 1) {
    stat.perror("MLibrary::initialize");
    Thread::sleep(init_maya_timeout);
    stat = MLibrary::initialize(false, (char *)program_name.c_str(), view_license);
    --error_count;
  }

  // Restore the current directory.  Ever since Maya 2010, there seems to be
  // some bad mojo when you do this.
  if( revert_dir ){
    string dirname = _cwd.to_os_specific();
    if (chdir(dirname.c_str()) < 0) {
    maya_cat.warning()
      << "Unable to restore current directory to " << _cwd
      << " after initializing Maya.\n";
    } else {
    if (maya_cat.is_debug()) {
      maya_cat.debug()
      << "Restored current directory to " << _cwd << "\n";
    }
    }
  }


  if (!stat) {
    stat.perror("MLibrary::initialize");
    _is_valid = false;
  } else {
    _is_valid = true;
  }
}

/**
 * Don't attempt to copy MayaApi objects.  There should be only one of these
 * in the world at a time.
 */
MayaApi::
MayaApi(const MayaApi &copy) {
  nassertv(false);
}

/**
 * Don't attempt to copy MayaApi objects.  There should be only one of these
 * in the world at a time.
 */
void MayaApi::
operator = (const MayaApi &copy) {
  nassertv(false);
}

/**
 *
 */
MayaApi::
~MayaApi() {
  nassertv(_global_api == this);
  if (_is_valid && !_plug_in) {
    // Caution!  Calling this function seems to call exit() somewhere within
    // Maya code.
    MLibrary::cleanup();
  }
  _global_api = nullptr;
}

/**
 * Opens the Maya API, if it is not already open, and returns a pointer
 * representing this connection.  When you are done using the Maya API, let
 * the pointer destruct.
 *
 * If program_name is supplied, it is passed to Maya as the name of the
 * currently-executing program.  Otherwise, the current program name is
 * extracted from the execution environment, if possible.  The special
 * program_name "plug-in" is used for code that is intended to be invoked as a
 * plug-in only; in this case, the maya library is not re-initialized.
 */
PT(MayaApi) MayaApi::
open_api(string program_name, bool view_license, bool revertdir) {
  if (_global_api == nullptr) {
    // We need to create a new MayaApi object.
    if (program_name.empty()) {
      program_name = ExecutionEnvironment::get_binary_name();
      if (program_name.empty()) {
        program_name = "Panda";
      }
    }

    _global_api = new MayaApi(program_name, view_license, revertdir);

    // Try to compare the string-formatted runtime version number with the
    // numeric compile-time version number, so we can sanity check our runtime
    // environment.  (Sure would be nice if Maya provided an apples-to-apples
    // comparison for us.)

    // According to the Maya specs, the numeric value is derived by taking the
    // Maya version number and deleting the '.' characters, while also
    // ignoring everything after the second dot (and, for some reason,
    // appending a 0).

    string runtime_version = MGlobal::mayaVersion().asChar();
    string simple_runtime_version = runtime_version;
    runtime_version = trim(runtime_version);

    // If the version number contains a space, stop there (that would be
    // "service pack 1" or whatever).
    size_t space = runtime_version.find(' ');
    if (space != string::npos) {
      runtime_version = runtime_version.substr(0, space);
    }

    int rtver_a, rtver_b;
    size_t dot1 = runtime_version.find('.');
    if (dot1 == string::npos) {
      string_to_int(runtime_version, rtver_a);
      rtver_b = 0;

    } else {
      string_to_int(runtime_version.substr(0, dot1), rtver_a);

      size_t dot2 = runtime_version.find('.', dot1 + 1);
      if (dot2 == string::npos) {
        string_to_int(runtime_version.substr(dot1 + 1), rtver_b);

      } else {
        string_to_int(runtime_version.substr(dot1 + 1, dot2 - dot1 - 1), rtver_b);
        simple_runtime_version = runtime_version.substr(0, dot2);
      }
    }

    int runtime_version_int = rtver_a * 100 + rtver_b * 10;

    if (maya_cat.is_debug()) {
      maya_cat.debug()
        << "Compiled with Maya library version "
        << (MAYA_API_VERSION / 100) << "." << (MAYA_API_VERSION / 10) % 10
        << " (" << MAYA_API_VERSION << "); running with library version "
        << runtime_version << ".\n";
    }

    if (MAYA_API_VERSION / 10 != runtime_version_int / 10) {
      maya_cat.warning()
        << "This program was compiled using Maya version "
        << (MAYA_API_VERSION / 100) << "." << (MAYA_API_VERSION / 10) % 10
        << ", but you are now running it with Maya version "
        << simple_runtime_version
        << ".  The program may crash or produce incorrect results.\n\n";
    }
  }

  return _global_api;
}

/**
 * Returns true if the API has been successfully opened and may be used, or
 * false if there is some problem.
 */
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

/**
 * Reads the indicated maya file into the global model space.  Returns true if
 * successful, false otherwise.
 */
bool MayaApi::
read(const Filename &filename) {
  MFileIO::newFile(true);

  maya_cat.info() << "Reading " << filename << "\n";

  // Load the file into Maya.  Maya seems to want forward slashes, even on
  // Windows.
  string os_filename = filename.to_os_generic();

  string dirname = _cwd.to_os_specific();
  if (maya_cat.is_debug()) {
    maya_cat.debug() << "cwd(read:before): " << dirname.c_str() << std::endl;
  }

  MFileIO::newFile(true);
  MStatus stat = MFileIO::open(os_filename.c_str());
  // Beginning with Maya2008, the call to read seem to change the current
  // directory specially if there is a refrence file!  Yikes!

  // Furthermore, the current directory may change during the call to any Maya
  // function!  Egad!
  if (chdir(dirname.c_str()) < 0) {
    maya_cat.warning()
      << "Unable to restore current directory after ::read to " << _cwd
      << " after initializing Maya.\n";
  } else {
    if (maya_cat.is_debug()) {
      maya_cat.debug()
        << "Restored current directory after ::read to " << _cwd << "\n";
    }
  }
  if (!stat) {
    stat.perror(os_filename.c_str());
    return false;
  }
  return true;
}

/**
 * Writes the global model space to the indicated file.  Returns true if
 * successful, false otherwise.
 */
bool MayaApi::
write(const Filename &filename) {
  maya_cat.info() << "Writing " << filename << "\n";
  string os_filename = filename.to_os_generic();

  string dirname = _cwd.to_os_specific();
  if (maya_cat.is_debug()) {
    maya_cat.debug() << "cwd(write:before): " << dirname.c_str() << std::endl;
  }

  const char *type = "mayaBinary";
  string extension = filename.get_extension();
  if (extension == "ma") {
    type = "mayaAscii";
  }

  MStatus stat = MFileIO::saveAs(os_filename.c_str(), type, true);
  if (!stat) {
    stat.perror(os_filename.c_str());
    return false;
  }
  // Beginning with Maya2008, the call to read seem to change the current
  // directory specially if there is a refrence file!  Yikes!

  // Furthermore, the current directory may change during the call to any Maya
  // function!  Egad!
  if (chdir(dirname.c_str()) < 0) {
    maya_cat.warning()
      << "Unable to restore current directory after ::write to " << _cwd
      << " after initializing Maya.\n";
  } else {
    if (maya_cat.is_debug()) {
      maya_cat.debug()
        << "Restored current directory after ::write to " << _cwd << "\n";
    }
  }
  return true;
}

/**
 * Resets the global model space to the empty state, for instance in
 * preparation for building a new file.  Returns true if successful, false
 * otherwise.
 */
bool MayaApi::
clear() {
  MStatus stat = MFileIO::newFile(true);
  if (!stat) {
    stat.perror("clear");
    return false;
  }
  return true;
}

/**
 * Returns Maya's internal units in effect.
 */
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

/**
 * Set Maya's UI units.
 */
void MayaApi::
set_units(DistanceUnit unit) {
  switch (unit) {
  case DU_inches:
    MDistance::setUIUnit(MDistance::kInches);
    break;
  case DU_feet:
    MDistance::setUIUnit(MDistance::kFeet);
    break;
  case DU_yards:
    MDistance::setUIUnit(MDistance::kYards);
    break;
  case DU_statute_miles:
    MDistance::setUIUnit(MDistance::kMiles);
    break;
  case DU_millimeters:
    MDistance::setUIUnit(MDistance::kMillimeters);
    break;
  case DU_centimeters:
    MDistance::setUIUnit(MDistance::kCentimeters);
    break;
  case DU_kilometers:
    MDistance::setUIUnit(MDistance::kKilometers);
    break;
  case DU_meters:
    MDistance::setUIUnit(MDistance::kMeters);
    break;

  default:
    ;
  }
}

/**
 * Returns Maya's internal coordinate system in effect.
 */
CoordinateSystem MayaApi::
get_coordinate_system() {
  if (MGlobal::isYAxisUp()) {
    return CS_yup_right;
  } else {
    return CS_zup_right;
  }
}
