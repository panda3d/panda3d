// Filename: httpDocument.I
// Created by:  drose (24Sep02)
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
//     Function: HTTPDocument::is_valid
//       Access: Published
//  Description: Returns true if the document was successfully
//               retrieved and is ready to be read, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPDocument::
is_valid() const {
  return (_source != (IBioStream *)NULL && 
          (_status_code / 100) == 2);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_http_version
//       Access: Published
//  Description: Returns the HTTP version number returned by the
//               server, formatted as a string, e.g. "HTTP/1.1".
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPDocument::
get_http_version() const {
  return _http_version;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_status_code
//       Access: Published
//  Description: Returns the HTML return code from the document
//               retrieval request.  This will be in the 200 range if
//               the document is successfully retrieved, or some other
//               value in the case of an error.
////////////////////////////////////////////////////////////////////
INLINE int HTTPDocument::
get_status_code() const {
  return _status_code;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_status_string
//       Access: Published
//  Description: Returns the string as returned by the server
//               describing the status code for humans.  This may or
//               may not be meaningful.
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPDocument::
get_status_string() const {
  return _status_string;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_file_size
//       Access: Published
//  Description: Returns the size of the file, if it is known.
//               Returns 0 if the file size is not known.  
//
//               If the file is dynamically generated, the size may
//               not be available until a read has started
//               (e.g. open_read_file() has been called); and even
//               then it may increase as more of the file is read due
//               to the nature of HTTP/1.1 requests which can change
//               their minds midstream about how much data they're
//               sending you.
////////////////////////////////////////////////////////////////////
INLINE size_t HTTPDocument::
get_file_size() const {
  return _file_size;
}
