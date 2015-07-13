// Filename: pandaIOSystem.cxx
// Created by:  rdb (29Mar11)
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

#include "pandaIOSystem.h"
#include "pandaIOStream.h"

////////////////////////////////////////////////////////////////////
//     Function: PandaIOSystem::Constructor
//       Access: Public
//  Description: Initializes the object with the given VFS, or the
//               global one if none was specified.
////////////////////////////////////////////////////////////////////
PandaIOSystem::
PandaIOSystem(VirtualFileSystem *vfs) : _vfs(vfs) {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOSystem::Exists
//       Access: Public
//  Description: Returns true if the file exists, duh.
////////////////////////////////////////////////////////////////////
bool PandaIOSystem::
Exists(const char *file) const {
  Filename fn = Filename::from_os_specific(file);
  return _vfs->exists(fn);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOSystem::Close
//       Access: Public
//  Description: Closes the indicated file stream.
////////////////////////////////////////////////////////////////////
void PandaIOSystem::
Close(Assimp::IOStream *file) {
  PandaIOStream *pstr = (PandaIOStream*) file;
  _vfs->close_read_file(&pstr->_istream);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOSystem::ComparePaths
//       Access: Public
//  Description: Returns true if the two paths point to the same
//               file, false if not.
////////////////////////////////////////////////////////////////////
bool PandaIOSystem::
ComparePaths(const char *p1, const char *p2) const {
  Filename fn1 = Filename::from_os_specific(p1);
  Filename fn2 = Filename::from_os_specific(p2);
  fn1.make_canonical();
  fn2.make_canonical();
  return fn1 == fn2;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOSystem::getOsSeparator
//       Access: Public
//  Description: Returns the path separator for this operating
//               system.
////////////////////////////////////////////////////////////////////
char PandaIOSystem::
getOsSeparator() const {
#ifdef _WIN32
  return '\\';
#else
  return '/';
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOSystem::Open
//       Access: Public
//  Description: Opens the indicated file.
////////////////////////////////////////////////////////////////////
Assimp::IOStream *PandaIOSystem::
Open(const char *file, const char *mode) {
  Filename fn = Filename::from_os_specific(file);

  if (mode[0] == 'r') {
    istream *stream = _vfs->open_read_file(file, true);
    if (stream == NULL) {
      return NULL;
    }
    return new PandaIOStream(*stream);

  } else {
    nassertr(false, NULL); // Not implemented on purpose.
    return NULL;
  }
}
