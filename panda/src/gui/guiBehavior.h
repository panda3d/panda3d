// Filename: guiBehavior.h
// Created by:  cary (07Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIBEHAVIOR_H__
#define __GUIBEHAVIOR_H__

#include "guiItem.h"

class EXPCL_PANDA GuiBehavior : public GuiItem {
protected:
  EventHandler* _eh;
  bool _behavior_running;

  INLINE GuiBehavior(void);
PUBLISHED:
  class EXPCL_PANDA BehaviorFunctor {
  public:
    BehaviorFunctor(void);
    ~BehaviorFunctor(void);
    virtual void doit(GuiBehavior*) = 0;
  };
PUBLISHED:
  GuiBehavior(const string&);
  virtual ~GuiBehavior(void);

  virtual void manage(GuiManager*, EventHandler&) = 0;
  virtual void unmanage(void) = 0;

  virtual void start_behavior(void) = 0;
  virtual void stop_behavior(void) = 0;
  virtual void reset_behavior(void) = 0;

  virtual void output(ostream&) const = 0;
public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiItem::init_type();
    register_type(_type_handle, "GuiBehavior",
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

#include "guiBehavior.I"

#endif /* __GUIBEHAVIOR_H__ */
