// Filename: pnmTextMaker.I
// Created by:  drose (07Sep03)
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


////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::is_valid
//       Access: Public
//  Description: Returns true if the PNMTextMaker is valid and ready to
//               generate text, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PNMTextMaker::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::set_align
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PNMTextMaker::
set_align(PNMTextMaker::Alignment align_type) {
  if (_align != align_type) {
    _align = align_type;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::get_align
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PNMTextMaker::Alignment PNMTextMaker::
get_align() const {
  return _align;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::generate_into
//       Access: Public
//  Description: Generates text into the indicated image at the
//               indicated position.
////////////////////////////////////////////////////////////////////
INLINE void PNMTextMaker::
generate_into(const string &text, PNMImage &dest_image, int x, int y) {
  TextEncoder encoder;
  encoder.set_text(text);
  generate_into(encoder.get_wtext(), dest_image, x, y);
}
