// Filename: sedProcess.h
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SEDPROCESS_H
#define SEDPROCESS_H

#include "ppremake.h"
#include "sedScript.h"

///////////////////////////////////////////////////////////////////
//       Class : SedProcess
// Description : This supervises the whole sed process, from beginning
//               to end.
////////////////////////////////////////////////////////////////////
class SedProcess {
public:
  SedProcess();
  ~SedProcess();

  bool add_script_line(const string &line);

  void run(istream &in, ostream &out);

private:
  SedScript _script;
};

#endif
