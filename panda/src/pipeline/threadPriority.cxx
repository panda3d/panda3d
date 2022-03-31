/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadPriority.cxx
 * @author drose
 * @date 2008-09-26
 */

#include "threadPriority.h"
#include "pnotify.h" // nassertr
#include "pipeline.h"

using std::istream;
using std::ostream;
using std::string;

ostream &
operator << (ostream &out, ThreadPriority pri) {
  switch (pri) {
  case TP_low:
    return out << "low";

  case TP_normal:
    return out << "normal";

  case TP_high:
    return out << "high";

  case TP_urgent:
    return out << "urgent";
  }

  pipeline_cat->error()
    << "Invalid ThreadPriority value: " << (int)pri << "\n";
  nassertr(false, out);
  return out;
}

istream &
operator >> (istream &in, ThreadPriority &pri) {
  string word;
  in >> word;
  if (word == "low") {
    pri = TP_low;

  } else if (word == "normal") {
    pri = TP_normal;

  } else if (word == "high") {
    pri = TP_high;

  } else if (word == "urgent") {
    pri = TP_urgent;

  } else {
    pri = TP_normal;
    pipeline_cat->error()
      << "Invalid ThreadPriority string: " << word << "\n";
    nassertr(false, in);
  }

  return in;
}
