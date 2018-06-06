/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfstream.I
 * @author cary
 * @date 2000-12-12
 */

INLINE IPipeStream::IPipeStream(const std::string cmd)
  : std::istream(&_psb), _psb(PipeStreamBuf::Input) {
  _psb.command(cmd);
}

INLINE void IPipeStream::flush(void) {
  _psb.flush();
}

INLINE IPipeStream::IPipeStream(void)
  : std::istream(&_psb), _psb(PipeStreamBuf::Input) {
  std::cerr << "should never call default constructor of IPipeStream" << std::endl;
}

INLINE OPipeStream::OPipeStream(const std::string cmd)
  : std::ostream(&_psb), _psb(PipeStreamBuf::Output) {
  _psb.command(cmd);
}

INLINE void OPipeStream::flush(void) {
  _psb.flush();
}

INLINE OPipeStream::OPipeStream(void)
  : std::ostream(&_psb), _psb(PipeStreamBuf::Output) {
  std::cerr << "should never call default constructor of OPipeStream" << std::endl;
}
