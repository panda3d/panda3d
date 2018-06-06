/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeRegistry.h
 * @author drose
 * @date 2000-06-15
 */

#ifndef PNMFILETYPEREGISTRY_H
#define PNMFILETYPEREGISTRY_H

#include "pandabase.h"

#include "typedObject.h"
#include "pmap.h"
#include "pvector.h"

class PNMFileType;

/**
 * This class maintains the set of all known PNMFileTypes in the universe.
 */
class EXPCL_PANDA_PNMIMAGE PNMFileTypeRegistry {
protected:
  PNMFileTypeRegistry();

public:
  ~PNMFileTypeRegistry();

  void register_type(PNMFileType *type);
  void unregister_type(PNMFileType *type);

PUBLISHED:
  int get_num_types() const;
  PNMFileType *get_type(int n) const;
  MAKE_SEQ(get_types, get_num_types, get_type);
  MAKE_SEQ_PROPERTY(types, get_num_types, get_type);

  PNMFileType *get_type_from_extension(const std::string &filename) const;
  PNMFileType *get_type_from_magic_number(const std::string &magic_number) const;
  PNMFileType *get_type_by_handle(TypeHandle handle) const;

  void write(std::ostream &out, int indent_level = 0) const;

  static PNMFileTypeRegistry *get_global_ptr();

private:
  void sort_preferences();

  typedef pvector<PNMFileType *> Types;
  Types _types;

  typedef pmap<std::string, Types> Extensions;
  Extensions _extensions;

  typedef pmap<TypeHandle, PNMFileType *> Handles;
  Handles _handles;

  bool _requires_sort;

  static PNMFileTypeRegistry *_global_ptr;
};

#endif
