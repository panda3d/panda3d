// Filename: bamReaderParam.h
// Created by:  jason (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BAMREADERPARAM_H
#define BAMREADERPARAM_H

#include <pandabase.h>

#include "factoryParam.h"
#include "datagram.h"

#include <vector>

class BamReader;

////////////////////////////////////////////////////////////////////
// 	 Class : BamReaderParam
// Description : The specific derivation of FactoryParam that 
//               contains the information needed by a TypedWriteable
//               object.  Simply contains a pointer to the managing
//               BamReader
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamReaderParam : public FactoryParam {
public:
  INLINE BamReader *get_manager(void);

private:
  BamReader *_manager;

public:
  INLINE BamReaderParam(BamReader *manager);
  INLINE BamReaderParam(const BamReaderParam &other);
  INLINE void operator = (const BamReaderParam &other);
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

