// Filename: backupCatalog.h
// Created by:  drose (29Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef BACKUPCATALOG_H
#define BACKUPCATALOG_H

#include "pandaappbase.h"

#include "documentSpec.h"
#include "filename.h"
#include "pvector.h"
#include "pmap.h"
#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : BackupCatalog
// Description : This is the list of previous versions of this file
//               (and possibly other files) stored in the "catalog", a
//               text file within the download directory.
////////////////////////////////////////////////////////////////////
class BackupCatalog {
public:
  BackupCatalog();
  ~BackupCatalog();

  bool read(const Filename &filename);
  bool write(const Filename &filename);
  void clear();

  class Entry {
  public:
    INLINE Entry();
    INLINE bool operator < (const Entry &other) const;
    INLINE const HTTPDate &get_date() const;

    void delete_file(const Filename &dirname, const string &reason);
    void input(istream &in);
    void output(ostream &out) const;
    void write(ostream &out) const;

    string _document_name;
    string _filename;
    DocumentSpec _document_spec;
    HTTPDate _download_date;
  };

  typedef pvector<Entry *> Entries;
  typedef pmap<string, Entries> Table;
  Table _table;
  
  typedef pset<string> Filenames;
  Filenames _filenames;

  bool _dirty;
};

INLINE istream &operator >> (istream &in, BackupCatalog::Entry &entry);
INLINE ostream &operator << (ostream &out, const BackupCatalog::Entry &entry);

#include "backupCatalog.I"

#endif

