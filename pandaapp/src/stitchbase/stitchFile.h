// Filename: stitchFile.h
// Created by:  drose (08Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHFILE_H
#define STITCHFILE_H

#include "stitchCommand.h"

class StitchImage;
class StitchImageOutputter;

class StitchFile {
public:
  StitchFile();
  ~StitchFile();

  bool read(const string &filename);
  void write(ostream &out) const;

  void process(StitchImageOutputter &outputter);

  StitchCommand _root;
};

inline ostream &operator << (ostream &out, const StitchFile &f) {
  f.write(out);
  return out;
}

#endif

  
