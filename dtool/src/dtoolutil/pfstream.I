// Filename: pfstream.I
// Created by:  cary (12Dec00)
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

INLINE IPipeStream::IPipeStream(const std::string cmd)
  : istream(&_psb), _psb(PipeStreamBuf::Input) {
  _psb.command(cmd);
}

INLINE void IPipeStream::flush(void) {
  _psb.flush();
}

INLINE IPipeStream::IPipeStream(void)
  : istream(&_psb), _psb(PipeStreamBuf::Input) {
  cerr << "should never call default constructor of IPipeStream" << endl;
}

INLINE OPipeStream::OPipeStream(const std::string cmd)
  : ostream(&_psb), _psb(PipeStreamBuf::Output) {
  _psb.command(cmd);
}

INLINE void OPipeStream::flush(void) {
  _psb.flush();
}

INLINE OPipeStream::OPipeStream(void)
  : ostream(&_psb), _psb(PipeStreamBuf::Output) {
  cerr << "should never call default constructor of OPipeStream" << endl;
}
