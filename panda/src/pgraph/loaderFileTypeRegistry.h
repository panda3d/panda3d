/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeRegistry.h
 * @author drose
 * @date 2000-06-20
 */

#ifndef LOADERFILETYPEREGISTRY_H
#define LOADERFILETYPEREGISTRY_H

#include "pandabase.h"

#include "pvector.h"
#include "pmap.h"

class LoaderFileType;
class Filename;

/**
 * This class maintains the set of all known LoaderFileTypes in the universe.
 */
class EXPCL_PANDA_PGRAPH LoaderFileTypeRegistry {
protected:
  LoaderFileTypeRegistry();

public:
  ~LoaderFileTypeRegistry();

  void register_type(LoaderFileType *type);
  void register_deferred_type(const std::string &extension, const std::string &library);

  void unregister_type(LoaderFileType *type);

PUBLISHED:
  EXTENSION(void register_type(PyObject *type));
  EXTENSION(void register_deferred_type(PyObject *entry_point));

  EXTENSION(void unregister_type(PyObject *type));

  int get_num_types() const;
  LoaderFileType *get_type(int n) const;
  MAKE_SEQ(get_types, get_num_types, get_type);
  MAKE_SEQ_PROPERTY(types, get_num_types, get_type);
  LoaderFileType *get_type_from_extension(const std::string &extension);

  void write(std::ostream &out, int indent_level = 0) const;

  static LoaderFileTypeRegistry *get_global_ptr();

private:
  void record_extension(const std::string &extension, LoaderFileType *type);

private:
  typedef pvector<LoaderFileType *> Types;
  Types _types;

  typedef pmap<std::string, LoaderFileType *> Extensions;
  Extensions _extensions;

  typedef pmap<std::string, std::string> DeferredTypes;
  DeferredTypes _deferred_types;

  static LoaderFileTypeRegistry *_global_ptr;
};

#endif
