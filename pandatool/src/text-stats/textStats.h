// Filename: textStats.h
// Created by:  drose (12Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTSTATS_H
#define TEXTSTATS_H

#include <pandatoolbase.h>

#include <programBase.h>
#include <pStatServer.h>

////////////////////////////////////////////////////////////////////
//       Class : TextStats
// Description : A simple, scrolling-text stats server.  Guaranteed to
//               compile on every platform.
////////////////////////////////////////////////////////////////////
class TextStats : public ProgramBase, public PStatServer {
public:
  TextStats();

  virtual PStatMonitor *make_monitor();

  void run();

  int _port;
};

#endif

