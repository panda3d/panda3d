// Filename: bamReaderParam.h
// Created by:  jason (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BAMREADERPARAM_H
#define BAMREADERPARAM_H

#include <pandabase.h>

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
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BamReaderParam",
                  FactoryParam::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "bamReaderParam.I"

#endif

