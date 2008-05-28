// Filename: eggCharacterDb.cxx
// Created by:  drose (05Oct06)
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

#include "eggCharacterDb.h"
#include "eggCharacterData.h"

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterDb::Constructor
//       Access: Public
//  Description: Constructs a database for storing the interim work
//               for the indicated EggCharacterData.  The parameter
//               max_ram_mb indicates the maximum amount of RAM (in
//               MB) that the database should consume; if it the
//               database would roughly fit within this limit, it will
//               be stored in RAM; otherwise, it will be written to
//               disk (if Berkeley DB is available).
////////////////////////////////////////////////////////////////////
EggCharacterDb::
EggCharacterDb() {
  /*
#ifdef HAVE_BDB
  _db = NULL;

  _db = new Db(NULL, 0);
  _db_filename = Filename::temporary("", "eggc_", ".db");

  string os_db_filename = _db_filename.to_os_specific();
  _db->open(NULL, os_db_filename.c_str(), NULL,
            DB_BTREE, DB_CREATE | DB_EXCL, 0);

  nout << "Using " << os_db_filename << " for rebuild database.\n";
#endif  // HAVE_BDB
  */
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterDb::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggCharacterDb::
~EggCharacterDb() {
  /*
#ifdef HAVE_BDB
  if (_db != (Db *)NULL){ 
    _db->close(0);
    delete _db;
    _db = NULL;

    string os_db_filename = _db_filename.to_os_specific();
    Db rmdb(NULL, 0);
    rmdb.remove(os_db_filename.c_str(), NULL, 0);
  }
#endif  // HAVE_BDB
  */
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterDb::get_matrix
//       Access: Public
//  Description: Looks up the data for the indicated joint, type, and
//               frame, and fills it in result (and returns true) if
//               it is found.  Returns false if this data has not been
//               stored in the database.
////////////////////////////////////////////////////////////////////
bool EggCharacterDb::
get_matrix(const EggJointPointer *joint, TableType type,
           int frame, LMatrix4d &mat) const {
  Key key(joint, type, frame);

  /*
#ifdef HAVE_BDB
  if (_db != (Db *)NULL){ 
    Dbt db_key(&key, sizeof(Key));
    Dbt db_data(&mat, sizeof(LMatrix4d));
    db_data.set_ulen(sizeof(LMatrix4d));
    db_data.set_flags(DB_DBT_USERMEM);

    int result = _db->get(NULL, &db_key, &db_data, 0);
    if (result == DB_NOTFOUND) {
      return false;
    }
    nassertr(result == 0, false);
    return true;
  }
#endif  // HAVE_BDB
  */

  Table::const_iterator ti;
  ti = _table.find(key);
  if (ti == _table.end()) {
    return false;
  }

  mat = (*ti).second;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterDb::set_matrix
//       Access: Public
//  Description: Stores the matrix for the indicated joint, type, and
//               frame in the database.  It is an error to call this
//               more than once for any given key combination (not for
//               any technical reason, but because we don't expect
//               this to happen).
////////////////////////////////////////////////////////////////////
void EggCharacterDb::
set_matrix(const EggJointPointer *joint, TableType type,
           int frame, const LMatrix4d &mat) {
  Key key(joint, type, frame);

  /*
#ifdef HAVE_BDB
  if (_db != (Db *)NULL){ 
    Dbt db_key(&key, sizeof(Key));
    Dbt db_data((void *)&mat, sizeof(LMatrix4d));
    int result = _db->put(NULL, &db_key, &db_data, DB_NOOVERWRITE);
    nassertv(result != DB_KEYEXIST);
    nassertv(result == 0);
    return;
  }
#endif  // HAVE_BDB
  */

  bool inserted = _table.insert(Table::value_type(key, mat)).second;
  nassertv(inserted);
}
