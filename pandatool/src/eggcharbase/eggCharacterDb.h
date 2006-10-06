// Filename: eggCharacterDb.h
// Created by:  drose (05Oct06)
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

#ifndef EGGCHARACTERDB_H
#define EGGCHARACTERDB_H

#include "pandatoolbase.h"
#include "pmap.h"

/*
#ifdef HAVE_BDB

// Apparently, we have to define this to make db_cxx files include the
// modern header files.
#define HAVE_CXX_STDHEADERS 1
#include <db_cxx.h>

#endif  // HAVE_BDB
*/

class EggJointPointer;

////////////////////////////////////////////////////////////////////
//       Class : EggCharacterDb
// Description : This class is used during joint optimization or
//               restructuring to store the table of interim joint
//               computations.
//
//               That is to say, this class provides an temporary data
//               store for three tables of matrices per each
//               EggJointPointer per frame.
////////////////////////////////////////////////////////////////////
class EggCharacterDb {
public:
  EggCharacterDb();
  ~EggCharacterDb();

  enum TableType {
    TT_rebuild_frame,
    TT_net_frame,
    TT_net_frame_inv,
  };

  bool get_matrix(const EggJointPointer *joint, TableType type,
                  int frame, LMatrix4d &mat) const;
  void set_matrix(const EggJointPointer *joint, TableType type,
                  int frame, const LMatrix4d &mat);

private:
  class Key {
  public:
    INLINE Key(const EggJointPointer *joint,
               TableType table_type,
               int frame);
    INLINE bool operator < (const Key &other) const;

  private:
    const EggJointPointer *_joint;
    TableType _table_type;
    int _frame;
  };

  /*
#ifdef HAVE_BDB
  Db *_db;
  Filename _db_filename;
#endif  // HAVE_BDB
  */

  typedef pmap<Key, LMatrix4d> Table;
  Table _table;
};

#include "eggCharacterDb.I"

#endif


