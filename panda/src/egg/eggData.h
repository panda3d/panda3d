// Filename: eggData.h
// Created by:  drose (20Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGDATA_H
#define EGGDATA_H

#include <pandabase.h>

#include "eggGroupNode.h"
#include <filename.h>
#include <coordinateSystem.h>
#include <notify.h>
#include <dSearchPath.h>

#include <string>


///////////////////////////////////////////////////////////////////
// 	 Class : EggData
// Description : This is the primary interface into all the egg data,
//               and the root of the egg file structure.  An EggData
//               structure corresponds exactly with an egg file on the
//               disk.
//
//               The EggData class inherits from EggGroupNode its
//               collection of children, which are accessed by using
//               the EggData itself as an STL container with begin()
//               and end() calls.  The children of the EggData class
//               are the toplevel nodes in the egg file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggData : public EggGroupNode {
public:
  INLINE EggData();
  INLINE EggData(const EggData &copy);
  INLINE EggData &operator = (const EggData &copy);

  static bool resolve_egg_filename(Filename &egg_filename, 
				   const DSearchPath &searchpath = DSearchPath());

  bool read(Filename filename);
  bool read(istream &in);

  bool resolve_externals(const DSearchPath &searchpath = DSearchPath());

  bool write_egg(Filename filename);
  bool write_egg(ostream &out);

  void set_coordinate_system(CoordinateSystem coordsys);
  INLINE CoordinateSystem get_coordinate_system() const;

  INLINE void set_egg_filename(const Filename &directory);
  INLINE const Filename &get_egg_filename() const;

protected:
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  void post_read();
  void pre_write();

  CoordinateSystem _coordsys;
  Filename _egg_filename;

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
