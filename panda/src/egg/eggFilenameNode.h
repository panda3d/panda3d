// FilenameNode: eggFilenameNode.h
// Created by:  drose (11Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGFILENAMENODE_H
#define EGGFILENAMENODE_H

#include <pandabase.h>

#include "eggNode.h"
#include <filename.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggFilenameNode
// Description : This is an egg node that contains a filename.  It
//               references a physical file relative to the directory
//               the egg file was loaded in.  It is a base class for
//               EggTexture and EggExternalReference.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggFilenameNode : public EggNode {
public:
  INLINE EggFilenameNode();
  INLINE EggFilenameNode(const string &node_name, const Filename &filename);
  INLINE EggFilenameNode(const EggFilenameNode &copy);
  INLINE EggFilenameNode &operator = (const EggFilenameNode &copy);

  virtual string get_default_extension() const;

  INLINE const Filename &get_filename() const;
  INLINE void set_filename(const Filename &filename);
  INLINE Filename &update_filename();

private:
  Filename _filename;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggFilenameNode",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "eggFilenameNode.I"

#endif
