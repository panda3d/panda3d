// Filename: LOD.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef LOD_H
#define LOD_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <typeHandle.h>
#include <luse.h>
#include <referenceCount.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : LODSwitch
// Description : Defines a switching region for an LOD.  An object
//               will be visible when it is closer than "in" units,
//               but further than "out" units from the camera.
//
//               The sense of in vs. out distances is as if the object
//               were coming towards you from far away: it switches
//               "in" at the far distance, and switches "out" at the
//               close distance.  Thus, "in" should be larger than
//               "out".
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LODSwitch {
public:
  INLINE LODSwitch(float in, float out);
  INLINE void get_range(float &in, float &out) const;
  INLINE void set_range(float in, float out);
  INLINE bool in_range(float dist_squared) const;

  INLINE void rescale(float factor_squared);

  // We must declare these operators to allow VC++ to explicitly
  // export vector<LODSwitch>, below.  They don't do anything useful.
  INLINE bool operator == (const LODSwitch &other) const;
  INLINE bool operator != (const LODSwitch &other) const;
  INLINE bool operator < (const LODSwitch &other) const;

  INLINE void write_datagram(Datagram &destination) const;
  INLINE void read_datagram(DatagramIterator &source);

protected:
  float _in;
  float _out;
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<LODSwitch>)
typedef vector<LODSwitch> LODSwitchVector;

////////////////////////////////////////////////////////////////////
//       Class : LOD
// Description : Computes whether a level-of-detail should be rendered
//		 or not based on distance from the rendering camera.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LOD : public ReferenceCount, public TypedObject {
public:
  LOD(void);
  LOD(const LOD &copy);
  ~LOD(void);

  void xform(const LMatrix4f &mat);

  int compute_child(const LPoint3f &cam_pos,
		    const LPoint3f &center) const;

  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

public:
  LPoint3f		_center;
  LODSwitchVector	_switch_vector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    TypedObject::init_type();
    register_type(_type_handle, "LOD",
		  ReferenceCount::get_class_type(),
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "LOD.I"

#endif
