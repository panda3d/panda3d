// Filename: guiButton.h
// Created by:  cary (30Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIBUTTON_H__
#define __GUIBUTTON_H__

#include "guiBehavior.h"
#include "guiRegion.h"
#include "guiLabel.h"
#include "guiManager.h"

class EXPCL_PANDA GuiButton : public GuiBehavior {
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

  string _behavior_event;
  GuiBehavior::BehaviorFunctor* _behavior_functor;

  INLINE GuiButton(void);
  void switch_state(States);
  virtual void recompute_frame(void);

  static void behavior_up(CPT_Event, void*);
  static void behavior_down(CPT_Event, void*);
  void run_button_up(void);
  void run_button_down(void);
PUBLISHED:
  GuiButton(const string&, GuiLabel*, GuiLabel*);
  GuiButton(const string&, GuiLabel*, GuiLabel*, GuiLabel*);
  GuiButton(const string&, GuiLabel*, GuiLabel*, GuiLabel*, GuiLabel*,
	    GuiLabel*);
  virtual ~GuiButton(void);

public:
  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

PUBLISHED:
  virtual int freeze();
  virtual int thaw();

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
  INLINE void set_behavior_event(const string&);

  INLINE const string& get_up_event(void) const;
  INLINE const string& get_up_rollover_event(void) const;
  INLINE const string& get_down_event(void) const;
  INLINE const string& get_down_rollover_event(void) const;
  INLINE const string& get_inactive_event(void) const;
  INLINE const string& get_behavior_event(void) const;

  INLINE void set_up_rollover(GuiLabel*);
  INLINE void set_down_rollover(GuiLabel*);

  INLINE void set_behavior_functor(GuiBehavior::BehaviorFunctor*);
  INLINE GuiBehavior::BehaviorFunctor* get_behavior_functor(void) const;

  virtual void set_scale(float);
  virtual void set_pos(const LVector3f&);
  virtual void set_priority(GuiLabel*, const Priority);
  virtual void set_priority(GuiItem*, const Priority);

  virtual void start_behavior(void);
  virtual void stop_behavior(void);
  virtual void reset_behavior(void);

  virtual void output(ostream&) const;

public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiBehavior::init_type();
    register_type(_type_handle, "GuiButton",
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

#include "guiButton.I"

#endif /* __GUIBUTTON_H__ */
