// Filename: guiSign.h
// Created by:  cary (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUISIGN_H__
#define __GUISIGN_H__

#include "guiItem.h"
#include "guiLabel.h"
#include "guiManager.h"

class EXPCL_PANDA GuiSign : public GuiItem {
private:
  PT(GuiLabel) _sign;
  float _sign_scale;

  INLINE GuiSign(void);
  virtual void recompute_frame(void);

PUBLISHED:
  GuiSign(const string&, GuiLabel*);
  ~GuiSign(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

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
    register_type(_type_handle, "GuiSign",
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

#include "guiSign.I"

#endif /* __GUISIGN_H__ */
