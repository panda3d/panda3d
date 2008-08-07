// Filename: bindAnimRequest.I
// Created by:  drose (05Aug08)
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


////////////////////////////////////////////////////////////////////
//     Function: BindAnimRequest::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BindAnimRequest::
BindAnimRequest(const Filename &filename, const LoaderOptions &options,
                AnimControl *control, int hierarchy_match_flags,
                const PartSubset &subset) :
  ModelLoadRequest(filename, options),
  _control(control),
  _hierarchy_match_flags(hierarchy_match_flags),
  _subset(subset)
{
}

  
