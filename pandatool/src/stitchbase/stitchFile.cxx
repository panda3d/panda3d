// Filename: stitchFile.cxx
// Created by:  drose (08Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "stitchFile.h"
#include "stitchImage.h"
#include "stitchImageOutputter.h"
#include "stitchParserDefs.h"
#include "stitchLexerDefs.h"

StitchFile::
StitchFile() {
}

StitchFile::
~StitchFile() {
}

bool StitchFile::
read(const string &filename) {
  _root.clear();

  ifstream in(filename.c_str());
  if (!in) {
    nout << "Unable to read " << filename << "\n";
    return false;
  }
  stitch_init_parser(in, filename, &_root);
  stitchyyparse();
  if (stitch_error_count() != 0) {
    return false;
  }

  return true;
}

void StitchFile::
write(ostream &out) const {
  _root.write(out, 0);
}

void StitchFile::
process(StitchImageOutputter &outputter) {
  _root.process(outputter, (Stitcher *)NULL, *this);
  outputter.execute();
}  
