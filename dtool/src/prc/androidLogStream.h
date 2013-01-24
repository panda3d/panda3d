// Filename: androidLogStream.h
// Created by:  rdb (12Jan13)
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

#ifndef ANDROIDLOGSTREAM_H
#define ANDROIDLOGSTREAM_H

#ifdef ANDROID

#include "dtoolbase.h"
#include "notifySeverity.h"

#include <string>
#include <iostream>

////////////////////////////////////////////////////////////////////
//       Class : AndroidLogStream
// Description : This is a type of ostream that writes each line
//               to the Android log.
///////////////////////////////////////////////////////////////////
class AndroidLogStream : public ostream {
private:
  class AndroidLogStreamBuf : public streambuf {
  public:
    AndroidLogStreamBuf(int priority);
    virtual ~AndroidLogStreamBuf();

  protected:
    virtual int overflow(int c);
    virtual int sync();

  private:
    void write_char(char c);

    int _priority;
    string _tag;
    string _data;
  };

  AndroidLogStream(int priority);

public:
  virtual ~AndroidLogStream();
  static ostream &out(NotifySeverity severity);
};

#endif  // ANDROID

#endif
