// Filename: guiItem.h
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIITEM_H__
#define __GUIITEM_H__

#include "guiManager.h"

#include <eventHandler.h>

class EXPCL_PANDA GuiItem : public Namable {
protected:
  bool _added_hooks;
  float _scale, _left, _right, _bottom, _top;
  LVector3f _pos;
  GuiManager* _mgr;

  INLINE GuiItem(void);
  virtual void recompute_frame(void) = 0;
public:
  GuiItem(const string&);
  virtual ~GuiItem(void);

  virtual void manage(GuiManager*, EventHandler&) = 0;
  virtual void unmanage(void) = 0;

  virtual void set_scale(float) = 0;
  virtual void set_pos(const LVector3f&) = 0;

  INLINE float get_scale(void) const;
  INLINE LVector3f get_pos(void) const;
  INLINE float get_left(void) const;
  INLINE float get_right(void) const;
  INLINE float get_bottom(void) const;
  INLINE float get_top(void) const;
  INLINE LVector4f get_frame(void) const;

  virtual void output(ostream&) const = 0;
};

#include "guiItem.I"

#endif /* __GUIITEM_H__ */
