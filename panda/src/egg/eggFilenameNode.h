/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggFilenameNode.h
 * @author drose
 * @date 1999-02-11
 */

#ifndef EGGFILENAMENODE_H
#define EGGFILENAMENODE_H

#include "pandabase.h"

#include "eggNode.h"
#include "filename.h"

/**
 * This is an egg node that contains a filename.  It references a physical
 * file relative to the directory the egg file was loaded in.  It is a base
 * class for EggTexture and EggExternalReference.
 */
class EXPCL_PANDA_EGG EggFilenameNode : public EggNode {
PUBLISHED:
  INLINE EggFilenameNode();
  INLINE explicit EggFilenameNode(const std::string &node_name, const Filename &filename);
  INLINE EggFilenameNode(const EggFilenameNode &copy);
  INLINE EggFilenameNode &operator = (const EggFilenameNode &copy);

  virtual std::string get_default_extension() const;

  INLINE const Filename &get_filename() const;
  INLINE void set_filename(const Filename &filename);

  INLINE const Filename &get_fullpath() const;
  INLINE void set_fullpath(const Filename &fullpath);

public:
  class IndirectOrderByBasename {
  public:
    bool operator () (const EggFilenameNode *a, const EggFilenameNode *b) const {
      return a->get_filename().get_basename() < b->get_filename().get_basename();
    }
  };

protected:
  Filename _filename;
  Filename _fullpath;

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
