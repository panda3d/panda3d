// Filename: nullAudioDSP.cxx
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

#include "nullAudioDSP.h"

TypeHandle nullAudioDSP::_type_handle;



////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::reset
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void nullAudioDSP::
reset() {
	// intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::remove
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void nullAudioDSP::
remove() {
	// intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::set_bypass
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void nullAudioDSP::
set_bypass(bool bypass) {
	// intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::get_bypass
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
bool nullAudioDSP::get_bypass() {
	// intentionally blank

	return 0;

}



////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::set_parameter
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void nullAudioDSP::
set_parameter(const string &name, float value) {
	// intentionally blank
}



////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::list_parameters_info
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void nullAudioDSP::
list_parameters_info() {
	// intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::get_num_parameters
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
int nullAudioDSP::
get_num_parameters() {
	// intentionally blank
	return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::get_parameter_name
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
string nullAudioDSP::
get_parameter_name(int index) {

	return "";
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::get_parameter_description
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
string nullAudioDSP::
get_parameter_description(int index) {
	// intentionally blank
	return "";
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::get_parameter_min
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float nullAudioDSP::
get_parameter_min(int index) {
	// intentionally blank
	return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::get_parameter_max
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float nullAudioDSP::
get_parameter_max(int index) {
	// intentionally blank
	return 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::get_parameter_value
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float nullAudioDSP::
get_parameter_value(const string &name) {
	// intentionally blank
	return 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::Constructor
//       Access: Protected
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
nullAudioDSP::nullAudioDSP() {
  // Intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: nullAudioDSP::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
nullAudioDSP::~nullAudioDSP() {
}

