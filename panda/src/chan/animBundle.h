// Filename: animBundle.h
// Created by:  drose (21Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMBUNDLE_H
#define ANIMBUNDLE_H

#include <pandabase.h>

#include "animGroup.h"

#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : AnimBundle
// Description : This is the root of an AnimChannel hierarchy.  It
//               knows the frame rate and number of frames of all the
//               channels in the hierarchy (which must all match).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimBundle : public AnimGroup {
PUBLISHED:
  INLINE AnimBundle(const string &name, float fps, int num_frames);

  INLINE double get_base_frame_rate() const;
  INLINE int get_num_frames() const;

  virtual void output(ostream &out) const;

protected:
  INLINE AnimBundle(void);
  
  float _fps;
  int _num_frames;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWritable *make_AnimBundle(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimGroup::init_type();
    register_type(_type_handle, "AnimBundle",
                  AnimGroup::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

inline ostream &operator <<(ostream &out, const AnimBundle &bundle) {
  bundle.output(out);
  return out;
}


#include "animBundle.I"

#endif
