// Filename: guiListBox.h
// Created by:  cary (18Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUILISTBOX_H__
#define __GUILISTBOX_H__

#include "guiBehavior.h"

#include <vector>
#include <deque>

class EXPCL_PANDA GuiListBox : public GuiBehavior {
private:
  typedef vector< PT(GuiItem) > ItemVector;
  typedef deque< PT(GuiItem) > ItemDeque;

  class EXPCL_PANDA ListFunctor : public GuiBehavior::BehaviorFunctor {
  protected:
    GuiBehavior::BehaviorFunctor* _prev;
    GuiListBox* _lb;
  public:
    ListFunctor(GuiListBox*, GuiBehavior::BehaviorFunctor*);
    virtual ~ListFunctor(void);
    virtual void doit(GuiBehavior*);
    INLINE GuiBehavior::BehaviorFunctor* get_prev(void) { return _prev; }
  };

  friend ListFunctor;

  ItemVector _top_stack;
  ItemDeque _bottom_stack;
  ItemVector _visible;
  bool _arrow_top;
  bool _arrow_bottom;
  PT(GuiItem) _up_arrow;
  PT(GuiItem) _down_arrow;
  unsigned int _n_visible;

  ListFunctor* _up_functor;
  ListFunctor* _down_functor;

  INLINE GuiListBox(void);
  virtual void recompute_frame(void);
  void visible_patching(void);
PUBLISHED:
  GuiListBox(const string&, int, GuiItem*, GuiItem*);
  ~GuiListBox(void);

  void scroll_up(void);
  void scroll_down(void);
  void add_item(GuiItem*);

  virtual int freeze(void);
  virtual int thaw(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void manage(GuiManager*, EventHandler&, Node*);
  virtual void unmanage(void);

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
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiBehavior::init_type();
    register_type(_type_handle, "GuiListBox",
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

#include "guiListBox.I"

#endif /* __GUILISTBOX_H__ */
