// Filename: guiFrame.h
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIFRAME_H__
#define __GUIFRAME_H__

#include "guiItem.h"

#include <vector>

class EXPCL_PANDA GuiFrame : public GuiItem {
PUBLISHED:
  enum Packing { NONE, ABOVE, UNDER, LEFT, RIGHT, ALIGN_ABOVE, ALIGN_UNDER,
		 ALIGN_LEFT, ALIGN_RIGHT };
private:
  class Connection {
  private:
    Packing _how;
    GuiItem* _who;
    float _gap;
  public:
    inline Connection(void) : _how(NONE), _who((GuiItem*)0L), _gap(0.) {}
    inline Connection(Packing how, GuiItem* who, float gap) : _how(how),
							      _who(who),
							      _gap(gap) {}
    inline Connection(const Connection& c) : _how(c._how), _who(c._who),
					     _gap(c._gap) {}
    ~Connection(void) {}

    inline void set_how(Packing how) { _how = how; }
    inline void set_who(GuiItem* who) { _who = who; }
    inline void set_gap(float gap) { _gap = gap; }

    inline Packing get_how(void) const { return _how; }
    inline GuiItem* get_who(void) const { return _who; }
    inline float get_gap(void) const { return _gap; }
  };
  typedef vector<Connection> Connections;
  class Box {
  private:
    PT(GuiItem) _thing;
    float _scale;
    Connections _links;
  public:
    inline Box(void) : _thing((GuiItem*)0L), _scale(1.) {}
    inline Box(GuiItem* i) : _thing(i), _scale(i->get_scale()) {}
    inline Box(const Box& c) : _thing(c._thing), _scale(c._scale),
			       _links(c._links) {}
    ~Box(void) {}

    inline void set_item(GuiItem* i) { _thing = i; }
    inline void set_scale(float f) { _scale = f; }
    inline void add_link(Connection c) { _links.push_back(c); }

    inline GuiItem* get_item(void) const { return _thing; }
    inline float get_scale(void) const { return _scale; }
    inline int get_num_links(void) const { return _links.size(); }
    inline Packing get_nth_packing(int n) const { return _links[n].get_how(); }
    inline GuiItem* get_nth_to(int n) const { return _links[n].get_who(); }
    inline float get_nth_gap(int n) const { return _links[n].get_gap(); }

    inline void erase_nth_link(int n) { _links.erase(_links.begin() + n); }
    inline void erase_all_links(void) { _links.clear(); }
  };
  typedef vector<Box> Boxes;

  Boxes _items;

  INLINE GuiFrame(void);
  Boxes::iterator find_box(GuiItem*);
  virtual void recompute_frame(void);

PUBLISHED:
  GuiFrame(const string&);
  ~GuiFrame(void);

  void add_item(GuiItem*);
  void remove_item(GuiItem*);
  void pack_item(GuiItem*, Packing, GuiItem*, float = 0.);
  void clear_packing(GuiItem*);
  void clear_all_packing(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

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
    register_type(_type_handle, "GuiFrame",
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

#include "guiFrame.I"

#endif /* __GUIFRAME_H__ */
