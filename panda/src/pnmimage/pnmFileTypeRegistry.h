// Filename: pnmFileTypeRegistry.h
// Created by:  drose (15Jun00)
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

#ifndef PNMFILETYPEREGISTRY_H
#define PNMFILETYPEREGISTRY_H

#include "pandabase.h"

#include "typedObject.h"

class PNMFileType;

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypeRegistry
// Description : This class maintains the set of all known
//               PNMFileTypes in the universe.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypeRegistry {
protected:
  PNMFileTypeRegistry();

public:
  ~PNMFileTypeRegistry();

  static PNMFileTypeRegistry *get_ptr();

  int get_num_types() const;
  PNMFileType *get_type(int n) const;

  PNMFileType *get_type_from_extension(const string &filename) const;
  PNMFileType *get_type_from_magic_number(const string &magic_number) const;
  PNMFileType *get_type_by_handle(TypeHandle handle) const;

  void write_types(ostream &out, int indent_level = 0) const;

  void register_type(PNMFileType *type);

private:
  void sort_preferences();

  typedef pvector<PNMFileType *> Types;
  Types _types;

  typedef pmap<string, Types> Extensions;
  Extensions _extensions;

  typedef pmap<TypeHandle, PNMFileType *> Handles;
  Handles _handles;

  bool _requires_sort;

  static PNMFileTypeRegistry *_global_ptr;
};

#endif

