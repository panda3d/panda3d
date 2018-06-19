/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file androidLogStream.cxx
 * @author rdb
 * @date 2013-01-12
 */

#include "androidLogStream.h"
#include "configVariableString.h"

#ifdef ANDROID

#include <android/log.h>

/**
 *
 */
AndroidLogStream::AndroidLogStreamBuf::
AndroidLogStreamBuf(int priority) :
  _priority(priority) {

  static ConfigVariableString android_log_tag
  ("android-log-tag", "Panda3D",
   PRC_DESC("This defines the tag that Panda3D will use when writing to the "
            "Android log.  The default is \"Panda3D\"."));

  if (_tag.empty()) {
    _tag = android_log_tag.get_value();
  }

  // The AndroidLogStreamBuf doesn't actually need a buffer--it's happy
  // writing characters one at a time, since they're just getting stuffed into
  // a string.  (Although the code is written portably enough to use a buffer
  // correctly, if we had one.)
  setg(0, 0, 0);
  setp(0, 0);
}

/**
 *
 */
AndroidLogStream::AndroidLogStreamBuf::
~AndroidLogStreamBuf() {
  sync();
}

/**
 * Called by the system ostream implementation when the buffer should be
 * flushed to output (for instance, on destruction).
 */
int AndroidLogStream::AndroidLogStreamBuf::
sync() {
  std::streamsize n = pptr() - pbase();

  // Write the characters that remain in the buffer.
  for (char *p = pbase(); p < pptr(); ++p) {
    write_char(*p);
  }

  pbump(-n);  // Reset pptr().
  return 0;  // EOF to indicate write full.
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int AndroidLogStream::AndroidLogStreamBuf::
overflow(int ch) {
  std::streamsize n = pptr() - pbase();

  if (n != 0 && sync() != 0) {
    return EOF;
  }

  if (ch != EOF) {
    // Write one more character.
    write_char(ch);
  }

  return 0;
}

/**
 * Stores a single character.
 */
void AndroidLogStream::AndroidLogStreamBuf::
write_char(char c) {
  nout.put(c);
  if (c == '\n') {
    // Write a line to the log file.
    __android_log_write(_priority, _tag.c_str(), _data.c_str());
    _data.clear();
  } else {
    _data += c;
  }
}

/**
 *
 */
AndroidLogStream::
AndroidLogStream(int priority) :
  std::ostream(new AndroidLogStreamBuf(priority)) {
}

/**
 *
 */
AndroidLogStream::
~AndroidLogStream() {
  delete rdbuf();
}

/**
 * Returns an AndroidLogStream suitable for writing log messages with the
 * indicated severity.
 */
std::ostream &AndroidLogStream::
out(NotifySeverity severity) {
  static AndroidLogStream* streams[NS_fatal + 1] = {nullptr};

  if (streams[severity] == nullptr) {
    int priority = ANDROID_LOG_UNKNOWN;
    if (severity != NS_unspecified) {
      priority = ((int)severity) + 1;
    }
    streams[severity] = new AndroidLogStream(priority);
  }

  return *streams[severity];
}

#endif  // ANDROID
