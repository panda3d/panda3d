// Filename: spamDeletor.h
// Created by:  drose (11Apr07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef SPAMDELETOR_H
#define SPAMDELETOR_H

#include "pandabase.h"
#include "objectDeletor.h"

////////////////////////////////////////////////////////////////////
//       Class : SpamDeletor
// Description : The only purpose of the SpamDeletor is to issue a
//               spam Notify message as each object is deleted.  Maybe
//               it will be useful to track down accidental deletions
//               due to PT mismanagement.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SpamDeletor : public ObjectDeletor {
public:
  virtual void delete_object(DeleteFunc *func, void *ptr);

  static void register_deletor();
};

#endif
