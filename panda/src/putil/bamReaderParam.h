/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamReaderParam.h
 * @author jason
 * @date 2000-06-13
 */

#ifndef BAMREADERPARAM_H
#define BAMREADERPARAM_H

#include "pandabase.h"

#include "factoryParam.h"

class BamReader;
class DatagramIterator;

/**
 * The parameters that are passed through the Factory to any object
 * constructing itself from a Bam file.
 */
class EXPCL_PANDA_PUTIL BamReaderParam : public FactoryParam {
public:
  INLINE const DatagramIterator &get_iterator();
  INLINE BamReader *get_manager();

private:
  const DatagramIterator &_iterator;
  BamReader *_manager;

public:
  INLINE BamReaderParam(const DatagramIterator &dgi, BamReader *manager);
  INLINE ~BamReaderParam();

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FactoryParam::init_type();
    register_type(_type_handle, "BamReaderParam",
                  FactoryParam::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "bamReaderParam.I"

#endif
