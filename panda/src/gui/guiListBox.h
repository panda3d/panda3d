// Filename: guiListBox.h
// Created by:  cary (18Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUILISTBOX_H__
#define __GUILISTBOX_H__

#include "guiItem.h"

#include <vector>

class EXPCL_PANDA GuiListBox : public GuiItem {
private:
  INLINE GuiListBox(void);
  virtual void recompute_frame(void);
PUBLISHED:
  GuiListBox(const string&);
  ~GuiListBox(void);

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
    register_type(_type_handle, "GuiListBox",
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

#include "guiListBox.I"

#endif /* __GUILISTBOX_H__ */
