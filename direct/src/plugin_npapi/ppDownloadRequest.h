/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ppDownloadRequest.h
 * @author drose
 * @date 2009-06-23
 */

#ifndef PPDOWNLOADREQUEST_H
#define PPDOWNLOADREQUEST_H

#include "nppanda3d_common.h"

/**
 * An instance of this object is assigned as the notifyData for URL requests,
 * to help the plugin associate streams with requests.
 */
class PPDownloadRequest {
public:
  enum RequestType {
    RT_contents_file,
    RT_core_dll,
    RT_user,
    RT_instance_data
  };

  inline PPDownloadRequest(RequestType rtype, int user_id = 0);

public:
  RequestType _rtype;
  int _user_id;

  // This is sent true when we have notified the plugin that the stream is
  // done.
  bool _notified_done;
};

#include "ppDownloadRequest.I"

#endif
