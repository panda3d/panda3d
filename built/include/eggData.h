/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggData.h
 * @author drose
 * @date 1999-01-20
 */

#ifndef EGGDATA_H
#define EGGDATA_H

#include "pandabase.h"

#include "eggGroupNode.h"
#include "filename.h"
#include "coordinateSystem.h"
#include "pnotify.h"
#include "dSearchPath.h"

class BamCacheRecord;

/**
 * This is the primary interface into all the egg data, and the root of the
 * egg file structure.  An EggData structure corresponds exactly with an egg
 * file on the disk.
 *
 * The EggData class inherits from EggGroupNode its collection of children,
 * which are accessed by using the EggData itself as an STL container with
 * begin() and end() calls.  The children of the EggData class are the
 * toplevel nodes in the egg file.
 */
class EXPCL_PANDA_EGG EggData : public EggGroupNode {
PUBLISHED:
  INLINE EggData();
  INLINE EggData(const EggData &copy);
  INLINE EggData &operator = (const EggData &copy);

  static bool resolve_egg_filename(Filename &egg_filename,
                                   const DSearchPath &searchpath = DSearchPath());

  bool read(Filename filename, std::string display_name = std::string());
  bool read(std::istream &in);
  void merge(EggData &other);

  bool load_externals(const DSearchPath &searchpath = DSearchPath());
  bool load_externals(const DSearchPath &searchpath, BamCacheRecord *record);
  int collapse_equivalent_textures();
  int collapse_equivalent_materials();

  bool write_egg(Filename filename);
  bool write_egg(std::ostream &out);

  INLINE void set_auto_resolve_externals(bool resolve);
  INLINE bool get_auto_resolve_externals() const;
  INLINE bool original_had_absolute_pathnames() const;

  void set_coordinate_system(CoordinateSystem coordsys);
  INLINE CoordinateSystem get_coordinate_system() const;

  INLINE void set_egg_filename(const Filename &egg_filename);
  INLINE const Filename &get_egg_filename() const;

  INLINE void set_egg_timestamp(time_t egg_timestamp);
  INLINE time_t get_egg_timestamp() const;

  MAKE_PROPERTY(auto_resolve_externals, get_auto_resolve_externals,
                                        set_auto_resolve_externals);
  MAKE_PROPERTY(coordinate_system, get_coordinate_system, set_coordinate_system);
  MAKE_PROPERTY(egg_filename, get_egg_filename, set_egg_filename);
  MAKE_PROPERTY(egg_timestamp, get_egg_timestamp, set_egg_timestamp);

  INLINE void recompute_vertex_normals(double threshold);
  INLINE void recompute_polygon_normals();
  INLINE void strip_normals();

protected:
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  void post_read();
  void pre_write();

  bool _auto_resolve_externals;
  bool _had_absolute_pathnames;
  CoordinateSystem _coordsys;
  Filename _egg_filename;
  time_t _egg_timestamp;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggGroupNode::init_type();
    register_type(_type_handle, "EggData",
                  EggGroupNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggData.I"

#endif
