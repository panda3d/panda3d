// Filename: eggAnimData.h
// Created by:  drose (19Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGANIMDATA_H
#define EGGANIMDATA_H

#include <pandabase.h>

#include "eggNode.h"

#include <pointerToArray.h>
#include <pta_double.h>

////////////////////////////////////////////////////////////////////
//       Class : EggAnimData
// Description : A base class for EggSAnimData and EggXfmAnimData,
//               which contain rows and columns of numbers.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggAnimData : public EggNode {
public:

  INLINE EggAnimData(const string &name = "");
  INLINE EggAnimData(const EggAnimData &copy);
  INLINE EggAnimData &operator = (const EggAnimData &copy);

  INLINE void set_fps(double type);
  INLINE void clear_fps();
  INLINE bool has_fps() const;
  INLINE double get_fps() const;

  INLINE void clear_data();
  INLINE void add_data(double value);

  INLINE int get_size() const;
  INLINE PTA_double get_data() const;
  INLINE void set_data(const PTA_double &data);

protected:
  PTA_double _data;

private:
  double _fps;
  bool _has_fps;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggAnimData",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "eggAnimData.I"

#endif
