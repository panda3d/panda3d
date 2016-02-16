/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dTemporaryFile.h
 * @author drose
 * @date 2009-08-19
 */

#ifndef P3DTEMPORARYFILE_H
#define P3DTEMPORARYFILE_H

#include "p3d_plugin_common.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DTemporaryFile
// Description : This represents a temporary filename for some
//               transitory purpose.  This returns a filename which is
//               guaranteed to be unique at the time the constructor
//               was called.
//
//               The file on disk, if it exists, will automatically be
//               deleted when the destructor is called.
////////////////////////////////////////////////////////////////////
class P3DTemporaryFile {
public:
  P3DTemporaryFile(const string &extension);
  ~P3DTemporaryFile();

  inline const string &get_filename() const;

private:
  string _filename;
};

inline ostream &operator << (ostream &out, P3DTemporaryFile &tfile) {
  return out << tfile.get_filename();
}

#include "p3dTemporaryFile.I"

#endif
