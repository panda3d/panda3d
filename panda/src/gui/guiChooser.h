// Filename: guiChooser.h
// Created by:  cary (08Feb01)
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
    PT(GuiBehavior::BehaviorFunctor) _prev;
    GuiChooser* _ch;
  public:
    ChooseFunctor(GuiChooser*, GuiBehavior::BehaviorFunctor*);
    virtual ~ChooseFunctor(void);
    virtual void doit(GuiBehavior*);
    INLINE GuiBehavior::BehaviorFunctor* get_prev(void) { return _prev; }
  public:
    // type interface
    static TypeHandle get_class_type(void) {
      return _type_handle;
    }
    static void init_type(void) {
      GuiBehavior::BehaviorFunctor::init_type();
      register_type(_type_handle, "ChooseFunctor",
                    GuiBehavior::BehaviorFunctor::get_class_type());
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

  friend ChooseFunctor;

  ItemVector _items;
  int _curr;
  bool _loop;
  PT(GuiButton) _prev_button;
  PT(GuiButton) _next_button;

  PT(ChooseFunctor) _prev_functor;
  PT(ChooseFunctor) _next_functor;

  INLINE GuiChooser(void);
  virtual void recompute_frame(void);
  void adjust_buttons(void);
PUBLISHED:
  GuiChooser(const string&, GuiButton*, GuiButton*);
  ~GuiChooser(void);

  void move_prev(void);
  void move_next(void);
  void add_item(GuiItem*);

  INLINE int get_curr_item(void) const;
  void set_curr_item(int);

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
    ChooseFunctor::init_type();
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
