// Filename: glOcclusionQueryContext_src.cxx
// Created by:  drose (27Mar06)
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

#include "pnotify.h"
#include "dcast.h"
#include "lightMutexHolder.h"
#include "pStatTimer.h"

#ifndef OPENGLES  // Occlusion queries not supported by OpenGL ES.

TypeHandle CLP(OcclusionQueryContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLOcclusionQueryContext::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CLP(OcclusionQueryContext)::
~CLP(OcclusionQueryContext)() {
  if (_index != 0) {
    // Tell the GSG to recycle this index when it gets around to it.
    CLP(GraphicsStateGuardian) *glgsg;
    DCAST_INTO_V(glgsg, _gsg);
    LightMutexHolder holder(glgsg->_lock);
    glgsg->_deleted_queries.push_back(_index);

    _index = 0;
  }
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
bool CLP(OcclusionQueryContext)::
is_answer_ready() const {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);
  GLuint result;
  glgsg->_glGetQueryObjectuiv(_index, GL_QUERY_RESULT_AVAILABLE, &result);

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "occlusion query " << (int)_index << " ready = " << (int)result << "\n";
  }

  return (result != 0);
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
void CLP(OcclusionQueryContext)::
waiting_for_answer() {
  PStatTimer timer(GraphicsStateGuardian::_wait_occlusion_pcollector);
  glFlush();
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
int CLP(OcclusionQueryContext)::
get_num_fragments() const {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_R(glgsg, _gsg, 0);

  GLuint result;
  glgsg->_glGetQueryObjectuiv(_index, GL_QUERY_RESULT_AVAILABLE, &result);
  if (result) {
    // The answer is ready now.
    glgsg->_glGetQueryObjectuiv(_index, GL_QUERY_RESULT, &result);
  } else {
    // The answer is not ready; this call will block.
    PStatTimer timer(GraphicsStateGuardian::_wait_occlusion_pcollector);
    glgsg->_glGetQueryObjectuiv(_index, GL_QUERY_RESULT, &result);
  }

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "occlusion query " << (int)_index << " reports " << (int)result
      << " fragments.\n";
  }

  glgsg->report_my_gl_errors();
  return result;
}

#endif  // OPENGLES
