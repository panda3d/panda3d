// Filename: guiButton.h
// Created by:  cary (30Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIBUTTON_H__
#define __GUIBUTTON_H__

#include "guiItem.h"
#include "guiRegion.h"
#include "guiLabel.h"
#include "guiManager.h"

class EXPCL_PANDA GuiButton : public GuiItem {
private:
  PT(GuiLabel) _up;
  PT(GuiLabel) _up_rollover;
  PT(GuiLabel) _down;
  PT(GuiLabel) _down_rollover;
  PT(GuiLabel) _inactive;
  string _up_event, _up_rollover_event, _down_event, _down_rollover_event;
  string _inactive_event;
  PT(GuiRegion) _rgn;

  float _up_scale;
  float _upr_scale;
  float _down_scale;
  float _downr_scale;
  float _inactive_scale;

  enum States { NONE, UP, UP_ROLLOVER, DOWN, DOWN_ROLLOVER, INACTIVE,
		INACTIVE_ROLLOVER };
  States _state;

  INLINE GuiButton(void);
  void switch_state(States);
  virtual void recompute_frame(void);
public:
  GuiButton(const string&, GuiLabel*, GuiLabel*);
  GuiButton(const string&, GuiLabel*, GuiLabel*, GuiLabel*);
  GuiButton(const string&, GuiLabel*, GuiLabel*, GuiLabel*, GuiLabel*,
	    GuiLabel*);
  virtual ~GuiButton(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);
  INLINE void enter(void);
  INLINE void exit(void);
  INLINE void up(void);
  INLINE void down(void);
  INLINE void inactive(void);
  INLINE void click(void);

  INLINE bool is_up(void) const;
  INLINE bool is_over(void) const;
  INLINE bool is_active(void) const;

  INLINE void set_up_event(const string&);
  INLINE void set_up_rollover_event(const string&);
  INLINE void set_down_event(const string&);
  INLINE void set_down_rollover_event(const string&);
  INLINE void set_inactive_event(const string&);

  INLINE const string& get_up_event(void) const;
  INLINE const string& get_up_rollover_event(void) const;
  INLINE const string& get_down_event(void) const;
  INLINE const string& get_down_rollover_event(void) const;
  INLINE const string& get_inactive_event(void) const;

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
    register_type(_type_handle, "GuiButton",
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

#include "guiButton.I"

#endif /* __GUIBUTTON_H__ */
