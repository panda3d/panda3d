// Filename: guiRollover.h
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIROLLOVER_H__
#define __GUIROLLOVER_H__

#include "guiBehavior.h"
#include "guiLabel.h"
#include "guiManager.h"

#include <mouseWatcherRegion.h>

class EXPCL_PANDA GuiRollover : public GuiBehavior {
private:
  PT(GuiLabel) _off;
  PT(GuiLabel) _on;
  PT(MouseWatcherRegion) _rgn;

  float _off_scale;
  float _on_scale;

  bool _state;

  INLINE GuiRollover(void);
  virtual void recompute_frame(void);
  virtual void adjust_region(void);

PUBLISHED:
  GuiRollover(const string&, GuiLabel*, GuiLabel*);
  virtual ~GuiRollover(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void manage(GuiManager*, EventHandler&, Node*);
  virtual void unmanage(void);

  virtual int freeze(void);
  virtual int thaw(void);

  INLINE void enter(void);
  INLINE void exit(void);

  INLINE bool is_over(void) const;

  virtual void set_scale(float);
  virtual void set_scale(float, float, float);
  virtual void set_pos(const LVector3f&);
  virtual void set_priority(GuiLabel*, const Priority);
  virtual void set_priority(GuiItem*, const Priority);

  virtual void start_behavior(void);
  virtual void stop_behavior(void);
  virtual void reset_behavior(void);

  virtual int set_draw_order(int);

  virtual void output(ostream&) const;

public:
  INLINE bool owns_region(const MouseWatcherRegion*) const;

public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiItem::init_type();
    register_type(_type_handle, "GuiRollover",
		  GuiBehavior::get_class_type());
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

#include "guiRollover.I"

#endif /* __GUIROLLOVER_H__ */
