/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfstream.h
 * @author cary
 * @date 1998-08-27
 */

#ifndef __PFSTREAM_H__
#define __PFSTREAM_H__

#include "pfstreamBuf.h"

class EXPCL_DTOOL_DTOOLUTIL IPipeStream : public std::istream {
public:
  INLINE IPipeStream(const std::string);

#if _MSC_VER >= 1800
  INLINE IPipeStream(const IPipeStream &copy) = delete;
#endif

  INLINE void flush();

private:
  PipeStreamBuf _psb;

  INLINE IPipeStream();
};

class EXPCL_DTOOL_DTOOLUTIL OPipeStream : public std::ostream {
public:
  INLINE OPipeStream(const std::string);

#if _MSC_VER >= 1800
  INLINE OPipeStream(const OPipeStream &copy) = delete;
#endif

  INLINE void flush();

private:
  PipeStreamBuf _psb;

  INLINE OPipeStream();
};

#include "pfstream.I"

#endif /* __PFSTREAM_H__ */
