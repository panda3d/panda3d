// Filename: guiBackground.h
// Created by:  cary (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIBACKGROUND_H__
#define __GUIBACKGROUND_H__

#include "guiItem.h"
#include "guiManager.h"

class EXPCL_PANDA GuiBackground : public GuiItem {
private:
  INLINE GuiBackground(void);
  virtual void recompute_frame(void);
PUBLISHED:
  GuiBackground(const string&);
  ~GuiBackground(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

  virtual int freeze(void);
  virtual int thaw(void);

  virtual void set_scale(float);
  virtual void set_pos(const LVector3f&);

  virtual void output(ostream&) const;

public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiItem::init_type();
    register_type(_type_handle, "GuiBackground",
		  GuiItem::get_class_type());
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

#include "guiBackground.I"

#endif /* __GUIBACKGROUND_H__ */
