// Filename: guiRegion.h
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIREGION_H__
#define __GUIREGION_H__

#include <pandabase.h>
#include <mouseWatcherRegion.h>
#include <pointerTo.h>
#include <typedReferenceCount.h>

// container for active regions of a GUI

class GuiManager;

class EXPCL_PANDA GuiRegion : public TypedReferenceCount, public Namable {
private:
  float _left, _right, _bottom, _top;
  PT(MouseWatcherRegion) _region;

  INLINE GuiRegion(void);

  INLINE MouseWatcherRegion* get_region(void) const;

  friend GuiManager;

PUBLISHED:
  INLINE GuiRegion(const string&, float, float, float, float, bool);
  ~GuiRegion(void);

  INLINE void trap_clicks(bool);

  INLINE void set_region(float, float, float, float);
  INLINE LVector4f get_frame(void) const;

public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GuiRegion",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "guiRegion.I"

#endif /* __GUIREGION_H__ */
