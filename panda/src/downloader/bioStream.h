// Filename: bioStream.h
// Created by:  drose (25Sep02)
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

#ifndef BIOSTREAM_H
#define BIOSTREAM_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "socketStream.h"
#include "bioStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : IBioStream
// Description : An input stream object that reads data from an
//               OpenSSL BIO object.  This is used by the HTTPClient
//               and HTTPChannel classes to provide a C++ interface
//               to OpenSSL.
//
//               Seeking is not supported.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS IBioStream : public ISocketStream {
public:
  INLINE IBioStream();
  INLINE IBioStream(BioPtr *source);

  INLINE IBioStream &open(BioPtr *source);

  virtual bool is_closed();
  virtual void close();

private:
  BioStreamBuf _buf;
};

////////////////////////////////////////////////////////////////////
//       Class : OBioStream
// Description : An output stream object that writes data to an
//               OpenSSL BIO object.  This is used by the HTTPClient
//               and HTTPChannel classes to provide a C++ interface
//               to OpenSSL.
//
//               Seeking is not supported.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS OBioStream : public OSocketStream {
public:
  INLINE OBioStream();
  INLINE OBioStream(BioPtr *source);

  INLINE OBioStream &open(BioPtr *source);

  virtual bool is_closed();
  virtual void close();

private:
  BioStreamBuf _buf;
};

////////////////////////////////////////////////////////////////////
//       Class : BioStream
// Description : A bi-directional stream object that reads and writes
//               data to an OpenSSL BIO object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS BioStream : public SocketStream {
public:
  INLINE BioStream();
  INLINE BioStream(BioPtr *source);

  INLINE BioStream &open(BioPtr *source);

  virtual bool is_closed();
  virtual void close();

private:
  BioStreamBuf _buf;
};

#include "bioStream.I"

#endif  // HAVE_OPENSSL


#endif


