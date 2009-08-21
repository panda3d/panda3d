// Filename: p3dHost.I
// Created by:  drose (21Aug09)
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
//     Function: P3DHost::get_host_dir
//       Access: Public
//  Description: Returns the local directory into which files
//               downloaded from this host will be installed.
////////////////////////////////////////////////////////////////////
inline const string &P3DHost::
get_host_dir() const {
  return _host_dir;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::get_host_url
//       Access: Public
//  Description: Returns the root URL of this particular host, as
//               passed from the package file.  This is a unique
//               string that identifies each host.
////////////////////////////////////////////////////////////////////
inline const string &P3DHost::
get_host_url() const {
  return _host_url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::get_host_url_prefix
//       Access: Public
//  Description: Returns the root URL of this host, for constructing
//               full URL sequences.  This is the same as
//               get_host_url(), except it is guaranteed to end in a
//               slash character.
////////////////////////////////////////////////////////////////////
inline const string &P3DHost::
get_host_url_prefix() const {
  return _host_url_prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::get_descriptive_name
//       Access: Public
//  Description: Returns the descriptive name provided for this host,
//               if any.  This will be available after
//               read_contents_file() has been called.
////////////////////////////////////////////////////////////////////
inline const string &P3DHost::
get_descriptive_name() const {
  return _descriptive_name;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::has_contents_file
//       Access: Public
//  Description: Returns true if a contents.xml file has been
//               successfully read for this host, false otherwise.
////////////////////////////////////////////////////////////////////
inline bool P3DHost::
has_contents_file() const {
  return (_xcontents != NULL);
}
