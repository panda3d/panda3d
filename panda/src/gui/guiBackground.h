// Filename: guiBackground.h
// Created by:  cary (05Feb01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __GUIBACKGROUND_H__
#define __GUIBACKGROUND_H__

#include "guiItem.h"
#include "guiManager.h"

class EXPCL_PANDA GuiBackground : public GuiItem {
private:
  PT(GuiLabel) _bg;
  PT(GuiItem) _item;

  INLINE GuiBackground(void);
  virtual void recompute_frame(void);
PUBLISHED:
  GuiBackground(const string&, GuiItem*);
  GuiBackground(const string&, GuiItem*, Texture*);
  GuiBackground(const string&, GuiItem*, GuiLabel*);
  ~GuiBackground(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void manage(GuiManager*, EventHandler&, Node*);
  virtual void unmanage(void);

  virtual int freeze(void);
  virtual int thaw(void);

  virtual void set_scale(float);
  virtual void set_scale(float, float, float);
  virtual void set_pos(const LVector3f&);
  virtual void set_priority(GuiLabel*, const Priority);
  virtual void set_priority(GuiItem*, const Priority);

  INLINE void set_color(float, float, float, float);
  INLINE void set_color(const Colorf&);
  INLINE Colorf get_color(void) const;

  virtual int set_draw_order(int);

  virtual void output(ostream&) const;

  INLINE void reassert(void);

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
