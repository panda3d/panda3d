// Filename: guiCollection.h
// Created by:  cary (07Mar01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUICOLLECTION_H__
#define __GUICOLLECTION_H__

#include "guiItem.h"

#include <vector>

class EXPCL_PANDA GuiCollection : public GuiItem {
private:
  typedef vector< PT(GuiItem) > Items;

  Items _items;

  INLINE GuiCollection(void);
  virtual void recompute_frame(void);
PUBLISHED:
  GuiCollection(const string&);
  ~GuiCollection(void);

  virtual int freeze();
  virtual int thaw();

  void add_item(GuiItem*);
  void remove_item(GuiItem*);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void manage(GuiManager*, EventHandler&, Node*);
  virtual void unmanage(void);

  virtual void set_scale(float);
  virtual void set_scale(float, float, float);
  virtual void set_pos(const LVector3f&);
  virtual void set_priority(GuiLabel*, const Priority);
  virtual void set_priority(GuiItem*, const Priority);

  virtual int set_draw_order(int);

  virtual void output(ostream&) const;
public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiItem::init_type();
    register_type(_type_handle, "GuiCollection",
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

#include "guiCollection.I"

#endif /* __GUICOLLECTION_H__ */
