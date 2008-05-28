// Filename: dxOcclusionQueryContext9.cxx
// Created by:  drose (04Jun07)
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

#include "dxOcclusionQueryContext9.h"
#include "dxGraphicsStateGuardian9.h"
#include "pnotify.h"
#include "dcast.h"
#include "pStatTimer.h"

TypeHandle DXOcclusionQueryContext9::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLOcclusionQueryContext::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DXOcclusionQueryContext9::
~DXOcclusionQueryContext9() {
  _query->Release();
  _query = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GLOcclusionQueryContext::is_answer_ready
//       Access: Public, Virtual
//  Description: Returns true if the query's answer is ready, false
//               otherwise.  If this returns false, the application
//               must continue to poll until it returns true.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
bool DXOcclusionQueryContext9::
is_answer_ready() const {
  DWORD result;
  HRESULT hr = _query->GetData(&result, sizeof(result), 0);
  return (hr != S_FALSE);
}

////////////////////////////////////////////////////////////////////
//     Function: GLOcclusionQueryContext::waiting_for_answer
//       Access: Public, Virtual
//  Description: Requests the graphics engine to expedite the pending
//               answer--the application is now waiting until the
//               answer is ready.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
void DXOcclusionQueryContext9::
waiting_for_answer() {
  get_num_fragments();
}

////////////////////////////////////////////////////////////////////
//     Function: GLOcclusionQueryContext::get_num_fragments
//       Access: Public, Virtual
//  Description: Returns the number of fragments (pixels) of the
//               specified geometry that passed the depth test.
//               If is_answer_ready() did not return true, this
//               function may block before it returns.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
int DXOcclusionQueryContext9::
get_num_fragments() const {
  DWORD result;
  HRESULT hr = _query->GetData(&result, sizeof(result), 0);
  if (hr == S_OK) {
    // The answer is ready now.
    return result;
  }

  {
    // The answer is not ready; this call will block.
    PStatTimer timer(DXGraphicsStateGuardian9::_wait_occlusion_pcollector);
    while (hr == S_FALSE) {
      hr = _query->GetData(&result, sizeof(result), D3DGETDATA_FLUSH);
    }
  }

  if (FAILED(hr)) {
    // Some failure, e.g. devicelost.  Return a nonzero value as a
    // worst-case answer.
    dxgsg9_cat.info()
      << "occlusion query failed " << D3DERRORSTRING(hr);
    return 1;
  }

  return result;
}
