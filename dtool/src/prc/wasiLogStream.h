/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file WasiLogStream.h
 * @author rdb
 * @date 2015-04-02
 */

#ifndef WasiLOGSTREAM_H
#define WasiLOGSTREAM_H

#if defined(__wasi__)

#include "dtoolbase.h"
#include "notifySeverity.h"

#include <string>
#include <iostream>

/**
 * This is a type of ostream that writes each line to the JavaScript log
 * window.
 */
class WasiLogStream : public std::ostream {
private:
  class WasiLogStreamBuf : public std::streambuf {
  public:
    WasiLogStreamBuf(int flags);
    virtual ~WasiLogStreamBuf();

  protected:
    virtual int overflow(int c);
    virtual int sync();

  private:
    void write_char(char c);

    int _flags;
    string _data;
  };

  WasiLogStream(int flags);

public:
  virtual ~WasiLogStream();

  friend class Notify;
};

#endif  // __wasi__

#endif  // WasiLOGSTREAM_H
