// Filename: binaryXml.cxx
// Created by:  drose (13Jul09)
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

#include "binaryXml.h"
#include <sstream>

// Actually, we haven't implemented the binary I/O for XML files yet.
// We just map these directly to the classic formatted I/O for now.

static const bool debug_xml_output = true;

////////////////////////////////////////////////////////////////////
//     Function: write_xml
//  Description: Writes the indicated TinyXml document to the given
//               stream.
////////////////////////////////////////////////////////////////////
void
write_xml(HandleStream &out, TiXmlDocument *doc, ostream &logfile) {
  ostringstream strm;
  strm << *doc;
  string data = strm.str();

  size_t length = data.length();
  out.write((char *)&length, sizeof(length));
  out.write(data.data(), length);
  out << flush;

  if (debug_xml_output) {
    logfile << "sent: " << data << "\n" << flush;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: read_xml
//  Description: Reads a TinyXml document from the given stream, and
//               returns it.  If the document is not yet available,
//               blocks until it is, or until there is an error
//               condition on the input.
//
//               The return value is NULL if there is an error, or the
//               newly-allocated document if it is successfully read.
//               If not NULL, the document has been allocated with
//               new, and should be eventually freed by the caller
//               with delete.
////////////////////////////////////////////////////////////////////
TiXmlDocument *
read_xml(HandleStream &in, ostream &logfile) {

#ifdef _WIN32
  HANDLE handle = in.get_handle();

  size_t length;
  DWORD bytes_read = 0;
  logfile << "ReadFile\n" << flush;
  BOOL success = ReadFile(handle, &length, sizeof(length), &bytes_read, NULL);
  logfile << "done ReadFile\n" << flush;
  if (!success) {
    DWORD error = GetLastError();
    if (error != ERROR_HANDLE_EOF && error != ERROR_BROKEN_PIPE) {
      logfile << "Error reading " << sizeof(length)
              << " bytes, windows error code 0x" << hex
              << error << dec << ".\n";
    }
    return NULL;
  }
  assert(bytes_read == sizeof(length));

  if (debug_xml_output) {
    ostringstream logout;
    logout << "reading " << length << " bytes\n";
    logfile << logout.str() << flush;
  }

  char *buffer = new char[length];

  bytes_read = 0;
  success = ReadFile(handle, buffer, length, &bytes_read, NULL);
  if (!success) {
    DWORD error = GetLastError();
    if (error != ERROR_HANDLE_EOF && error != ERROR_BROKEN_PIPE) {
      logfile << "Error reading " << length
              << " bytes, windows error code 0x" << hex
              << error << dec << ".\n";
    }
    delete[] buffer;
    return NULL;
  }
  assert(bytes_read == length);

  string data(buffer, length);
  delete[] buffer;

#else
  size_t length;
  in.read((char *)&length, sizeof(length));
  if (in.gcount() != sizeof(length)) {
    logfile << "read " << in.gcount() << " bytes instead of " << sizeof(length)
            << "\n";
    return NULL;
  }

  if (debug_xml_output) {
    ostringstream logout;
    logout << "reading " << length << " bytes\n";
    logfile << logout.str() << flush;
  }

  char *buffer = new char[length];
  in.read(buffer, length);
  if (in.gcount() != length) {
    delete[] buffer;
    return NULL;
  }

  string data(buffer, length);
  delete[] buffer;

#endif  // _WIN32

  istringstream strm(data);
  TiXmlDocument *doc = new TiXmlDocument;
  strm >> *doc;

  if (debug_xml_output) {
    ostringstream logout;
    logout << "received: " << *doc << "\n";
    logfile << logout.str() << flush;
  }
    
  return doc;
}
