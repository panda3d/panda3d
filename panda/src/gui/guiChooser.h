// Filename: guiChooser.h
// Created by:  cary (08Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUICHOOSER_H__
#define __GUICHOOSER_H__

#include "guiBehavior.h"
#include "guiButton.h"

#include <vector>

// Only shows one item at a time.  Has two buttons that move back and forth
// over the list of things it has.  Can do both circular and linear lists.
class EXPCL_PANDA GuiChooser : public GuiBehavior {
private:
  typedef vector< PT(GuiItem) > ItemVector;

  class EXPCL_PANDA ChooseFunctor : public GuiBehavior::BehaviorFunctor {
  protected:
    GuiBehavior::BehaviorFunctor* _prev;
    GuiChooser* _ch;
  public:
    ChooseFunctor(GuiChooser*, GuiBehavior::BehaviorFunctor*);
    virtual ~ChooseFunctor(void);
    virtual void doit(GuiBehavior*);
    INLINE GuiBehavior::BehaviorFunctor* get_prev(void) { return _prev; }
  };

  friend ChooseFunctor;

  ItemVector _items;
  int _curr;
  bool _loop;
  PT(GuiButton) _prev_button;
  PT(GuiButton) _next_button;

  ChooseFunctor* _prev_functor;
  ChooseFunctor* _next_functor;

  INLINE GuiChooser(void);
  virtual void recompute_frame(void);
PUBLISHED:
  GuiChooser(const string&, GuiButton*, GuiButton*);
  ~GuiChooser(void);

  void move_prev(void);
  void move_next(void);
  void add_item(GuiItem*);

  virtual int freeze(void);
  virtual int thaw(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

  virtual void set_scale(float);
  virtual void set_pos(const LVector3f&);

  virtual void start_behavior(void);
  virtual void stop_behavior(void);
  virtual void reset_behavior(void);

  virtual void output(ostream&) const;

  INLINE void set_loop(bool);
  INLINE bool get_loop(void) const;
public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiBehavior::init_type();
    register_type(_type_handle, "GuiChooser",
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

#include "guiChooser.I"

#endif /* __GUICHOOSER_H__ */
