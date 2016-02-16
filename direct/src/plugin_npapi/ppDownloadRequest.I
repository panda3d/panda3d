/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ppDownloadRequest.I
 * @author drose
 * @date 2009-06-23
 */

////////////////////////////////////////////////////////////////////
//     Function: PPDownloadRequest::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline PPDownloadRequest::
PPDownloadRequest(RequestType rtype, int user_id) :
  _rtype(rtype),
  _user_id(user_id),
  _notified_done(false)
{
}

