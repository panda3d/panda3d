// Filename: ribGraphicsPipe.I
// Created by:  drose (17Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe::
RIBGraphicsPipe(const PipeSpecifier& spec)
  : NoninteractiveGraphicsPipe(spec), _filename(spec.get_file_name()) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::get_file_name
//       Access: Public
//  Description: gets the base filename
////////////////////////////////////////////////////////////////////
INLINE std::string RIBGraphicsPipe::
get_file_name(void) const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe::
RIBGraphicsPipe(void) {
  cerr << "RIBGraphicsPipes should not be created with default constructor"
       << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe::
RIBGraphicsPipe(const RIBGraphicsPipe&) {
  cerr << "RIBGraphicsPipes should not be copied" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::operator=
//       Access: Public
//  Description: assignment operator
////////////////////////////////////////////////////////////////////
INLINE RIBGraphicsPipe& RIBGraphicsPipe::
operator=(const RIBGraphicsPipe&) {
  cerr << "RIBGraphicsPipes should not be assigned" << endl;
  return *this;
}
