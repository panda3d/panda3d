// Filename: httpDocument.cxx
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

#include "httpDocument.h"
#include "bioStream.h"
#include "chunkedStream.h"

#ifdef HAVE_SSL

TypeHandle HTTPDocument::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
HTTPDocument::
HTTPDocument(BIO *bio, bool owns_bio) {
  _file_size = 0;

  if (bio != (BIO *)NULL) {
    _source = new IBioStream(bio, owns_bio);
    read_headers();
    determine_content_length();

  } else {
    _source = (IBioStream *)NULL;
    _status_code = 0;
    _status_string = "No connection";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
HTTPDocument::
~HTTPDocument() {
  if (_source != (IBioStream *)NULL) {
    delete _source;
    _source = (IBioStream *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_file_system
//       Access: Published, Virtual
//  Description: Returns the VirtualFileSystem this file is associated
//               with.
////////////////////////////////////////////////////////////////////
VirtualFileSystem *HTTPDocument::
get_file_system() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_filename
//       Access: Published, Virtual
//  Description: Returns the full pathname to this file within the
//               virtual file system.
////////////////////////////////////////////////////////////////////
Filename HTTPDocument::
get_filename() const {
  return Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::is_regular_file
//       Access: Published, Virtual
//  Description: Returns true if this file represents a regular file
//               (and read_file() may be called), false otherwise.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
is_regular_file() const {
  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::open_read_file
//       Access: Public, Virtual
//  Description: Opens the document for reading.  Returns a newly
//               allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.  This may only be called
//               once for a particular HTTPDocument.
////////////////////////////////////////////////////////////////////
istream *HTTPDocument::
open_read_file() const {
  if (_source == (IBioStream *)NULL) {
    return NULL;
  }

  string transfer_coding = get_header_value("Transfer-Encoding");
  for (string::iterator si = transfer_coding.begin();
       si != transfer_coding.end();
       ++si) {
    (*si) = tolower(*si);
  }

  istream *result = _source;
  if (transfer_coding == "chunked") {
    result = new IChunkedStream(_source, true, (HTTPDocument *)this);
  }

  ((HTTPDocument *)this)->_source = (IBioStream *)NULL;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_header_value
//       Access: Published
//  Description: Returns the HTML header value associated with the
//               indicated key, or empty string if the key was not
//               defined in the message returned by the server.
////////////////////////////////////////////////////////////////////
string HTTPDocument::
get_header_value(const string &key) const {
  Headers::const_iterator hi = _headers.find(key);
  if (hi != _headers.end()) {
    return (*hi).second;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::write_headers
//       Access: Published
//  Description: Outputs a list of all headers defined by the server
//               to the indicated output stream.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
write_headers(ostream &out) const {
  Headers::const_iterator hi;
  for (hi = _headers.begin(); hi != _headers.end(); ++hi) {
    out << (*hi).first << ": " << (*hi).second << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::read_headers
//       Access: Private
//  Description: Reads all of the responses from the server up until
//               the first blank line, and stores the list of header
//               key:value pairs so retrieved.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
read_headers() {
  nassertv(_source != (IBioStream *)NULL);

  // The first line back should include the HTTP version and the
  // result code.
  string line;
  getline(*_source, line);
  if (!line.empty() && line[line.length() - 1] == '\r') {
    line = line.substr(0, line.length() - 1);
  }
  if (downloader_cat.is_debug()) {
    downloader_cat.debug() << "recv: " << line << "\n";
  }
  if (!(*_source) || line.length() < 5 || line.substr(0, 5) != "HTTP/") {
    // Not an HTTP response.
    _status_code = 0;
    _status_string = "Not an HTTP response";
    return;
  }

  // Split out the first line into its three components.
  size_t p = 5;
  while (p < line.length() && !isspace(line[p])) {
    p++;
  }
  _http_version = line.substr(0, p);

  while (p < line.length() && isspace(line[p])) {
    p++;
  }
  size_t q = p;
  while (q < line.length() && !isspace(line[q])) {
    q++;
  }
  string status_code = line.substr(p, q - p);
  _status_code = atoi(status_code.c_str());

  while (q < line.length() && isspace(line[q])) {
    q++;
  }
  _status_string = line.substr(q, line.length() - q);

  // Now read the rest of the lines.  These will be field: value
  // pairs.
  string field_name;
  string field_value;

  getline(*_source, line);
  if (!line.empty() && line[line.length() - 1] == '\r') {
    line = line.substr(0, line.length() - 1);
  }
  if (downloader_cat.is_debug()) {
    downloader_cat.debug() << "recv: " << line << "\n";
  }
  while (!_source->eof() && !_source->fail() && !line.empty()) {
    if (isspace(line[0])) {
      // If the line begins with a space, that continues the previous
      // field.
      p = 0;
      while (p < line.length() && isspace(line[p])) {
        p++;
      }
      field_value += line.substr(p - 1);

    } else {
      // If the line does not begin with a space, that defines a new
      // field.
      if (!field_name.empty()) {
        _headers[field_name] = field_value;
        field_value = string();
      }

      size_t colon = line.find(':');
      if (colon != string::npos) {
        field_name = line.substr(0, colon);
        p = colon + 1;
        while (p < line.length() && isspace(line[p])) {
          p++;
        }
        field_value = line.substr(p);
      }
    }

    getline(*_source, line);
    if (!line.empty() && line[line.length() - 1] == '\r') {
      line = line.substr(0, line.length() - 1);
    }
    if (downloader_cat.is_debug()) {
      downloader_cat.debug() << "recv: " << line << "\n";
    }
  }
  if (!field_name.empty()) {
    _headers[field_name] = field_value;
    field_value = string();
  }

  // A blank line terminates the headers.
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::determine_content_length
//       Access: Private
//  Description: Determines the file size based on the Content-Length
//               field if it has been supplied.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
determine_content_length() {
  string content_length = get_header_value("Content-Length");
  if (!content_length.empty()) {
    _file_size = atoi(content_length.c_str());
  }
}


#endif  // HAVE_SSL
