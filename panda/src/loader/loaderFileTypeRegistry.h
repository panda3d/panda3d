// Filename: loaderFileTypeRegistry.h
// Created by:  drose (20Jun00)
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

#ifndef LOADERFILETYPEREGISTRY_H
#define LOADERFILETYPEREGISTRY_H

#include <pandabase.h>

#include <vector>
#include <map>

class LoaderFileType;
class Filename;

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypeRegistry
// Description : This class maintains the set of all known
//               LoaderFileTypes in the universe.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LoaderFileTypeRegistry {
protected:
  LoaderFileTypeRegistry();

public:
  ~LoaderFileTypeRegistry();

  static LoaderFileTypeRegistry *get_ptr();

  int get_num_types() const;
  LoaderFileType *get_type(int n) const;

  LoaderFileType *get_type_from_extension(const string &extension) const;

  void write_types(ostream &out, int indent_level = 0) const;

  void register_type(LoaderFileType *type);

private:
  typedef vector<LoaderFileType *> Types;
  Types _types;

  typedef map<string, LoaderFileType *> Extensions;
  Extensions _extensions;

  static LoaderFileTypeRegistry *_global_ptr;
};

#endif

