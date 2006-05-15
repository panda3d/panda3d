// Filename: audioDSP.cxx
// Created by:  Stan Rosenbaum "Staque" - Spring 2006
//
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

#include "audioDSP.h"

TypeHandle AudioDSP::_type_handle;



////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::reset
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void AudioDSP::
reset() {
	// intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::remove
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void AudioDSP::
remove() {
	// intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::set_bypass
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void AudioDSP::
set_bypass(bool bypass) {
	// intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::get_bypass
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
bool AudioDSP::get_bypass() {
	// intentionally blank

	return 0;

}



////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::set_parameter
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void AudioDSP::
set_parameter(const string &name, float value) {
	// intentionally blank
}



////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::list_parameters_info
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void AudioDSP::
list_parameters_info() {
	int np = get_num_parameters();
	for (int i=0; i<np; i++) {
		string name = get_parameter_name(i);
		string desc = get_parameter_description(i);
		float minv = get_parameter_min(i);
		float maxv = get_parameter_max(i);
		cerr << "Parameter: " << name << " (" << desc << ") " << minv << " to " << maxv << endl ;
	}
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::get_num_parameters
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
int AudioDSP::
get_num_parameters() {
	// intentionally blank
	return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::get_parameter_name
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
string AudioDSP::
get_parameter_name(int index) {
	// intentionally blank
	return "";
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::get_parameter_description
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
string AudioDSP::
get_parameter_description(int index) {
	// intentionally blank
	return "";
}


////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::get_parameter_min
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float AudioDSP::
get_parameter_min(int index) {
	// intentionally blank
	return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::get_parameter_max
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float AudioDSP::
get_parameter_max(int index) {
	// intentionally blank
	return 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::get_parameter_value
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float AudioDSP::
get_parameter_value(const string &name) {
	// intentionally blank
	return 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::Constructor
//       Access: Protected
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
AudioDSP::AudioDSP() {
  // Intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: AudioDSP::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AudioDSP::~AudioDSP() {
}

