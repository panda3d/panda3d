// Filename: bamReaderParam.h
// Created by:  jason (13Jun00)
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

#ifndef BAMREADERPARAM_H
#define BAMREADERPARAM_H

#include "pandabase.h"

#include "factoryParam.h"

class BamReader;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : BamReaderParam
// Description : The parameters that are passed through the Factory to
//               any object constructing itself from a Bam file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamReaderParam : public FactoryParam {
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

