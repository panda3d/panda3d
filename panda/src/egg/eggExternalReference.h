// Filename: eggExternalReference.h
// Created by:  drose (11Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGEXTERNALREFERENCE_H
#define EGGEXTERNALREFERENCE_H

#include <pandabase.h>

#include "eggFilenameNode.h"

////////////////////////////////////////////////////////////////////
// 	 Class : EggExternalReference
// Description : Defines a reference to another egg file which should
//               be inserted at this point.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggExternalReference : public EggFilenameNode {
public:
  EggExternalReference(const string &node_name, const string &filename);
  EggExternalReference(const EggExternalReference &copy);
  EggExternalReference &operator = (const EggExternalReference &copy);

  INLINE EggExternalReference &operator = (const string &filename);
  INLINE EggExternalReference &operator = (const char *filename);
  INLINE EggExternalReference &operator = (const Filename &copy);

  virtual void write(ostream &out, int indent_level) const;

  virtual string get_default_extension() const;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggFilenameNode::init_type();
    register_type(_type_handle, "EggExternalReference",
                  EggFilenameNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "eggExternalReference.I"

#endif
