// Filename: bufferContext.cxx
// Created by:  drose (16Mar06)
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

#include "bufferContext.h"

TypeHandle BufferContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BufferContext::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BufferContext::
~BufferContext() {
#ifdef DO_PSTATS
  --(_owning_chain->_count);
  _owning_chain->adjust_bytes(-(int)_data_size_bytes);
  remove_from_list();
#endif  // DO_PSTATS
  _owning_chain = NULL;
}
