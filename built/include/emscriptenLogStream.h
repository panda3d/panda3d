/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file emscriptenLogStream.h
 * @author rdb
 * @date 2015-04-02
 */

#ifndef EMSCRIPTENLOGSTREAM_H
#define EMSCRIPTENLOGSTREAM_H

#ifdef __EMSCRIPTEN__

#include "dtoolbase.h"
#include "notifySeverity.h"

#include <string>
#include <iostream>

/**
 * This is a type of ostream that writes each line to the JavaScript log
 * window.
 */
class EmscriptenLogStream : public std::ostream {
private:
  class EmscriptenLogStreamBuf : public std::streambuf {
  public:
    EmscriptenLogStreamBuf(int flags);
    virtual ~EmscriptenLogStreamBuf();

  protected:
    virtual int overflow(int c);
    virtual int sync();

  private:
    void write_char(char c);

    int _flags;
    string _data;
  };

  EmscriptenLogStream(int flags);

public:
  virtual ~EmscriptenLogStream();

  friend class Notify;
};

#endif  // __EMSCRIPTEN__

#endif  // EMSCRIPTENLOGSTREAM_H
