// Filename: writeableParam.h
// Created by:  jason (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef WRITEABLEPARAM_H
#define WRITEABLEPARAM_H

#include <pandabase.h>

#include "factoryParam.h"
#include "datagram.h"

#include <vector>

////////////////////////////////////////////////////////////////////
// 	 Class : WriteableParam
// Description : The specific derivation of FactoryParam that 
//               contains the information needed by a TypedWriteable
//               object.  Simply contains a Datagram for the object
//               to construct itself from.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA WriteableParam : public FactoryParam {
public:
  INLINE const Datagram &get_datagram(void);

private:
  const Datagram &_packet;

public:
  INLINE WriteableParam(const Datagram &datagram);
  INLINE WriteableParam(const WriteableParam &other);
  INLINE ~WriteableParam();

private:
  // The assignment operator cannot be used for this class.
  INLINE void operator = (const WriteableParam &other);

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
    register_type(_type_handle, "WriteableParam",
		  FactoryParam::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "writeableParam.I"

#endif

