// Filename: guiFrame.h
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIFRAME_H__
#define __GUIFRAME_H__

#include "guiItem.h"

#include <vector>

class EXPCL_PANDA GuiFrame : public GuiItem {
public:
  enum Packing { NONE, ABOVE, UNDER, LEFT, RIGHT };
private:
  class Connection {
  private:
    Packing _how;
    GuiItem* _who;
  public:
    inline Connection(void) : _how(NONE), _who((GuiItem*)0L) {}
    inline Connection(Packing how, GuiItem* who) : _how(how), _who(who) {}
    inline Connection(const Connection& c) : _how(c._how), _who(c._who) {}
    ~Connection(void) {}

    inline void set_how(Packing how) { _how = how; }
    inline void set_who(GuiItem* who) { _who = who; }

    inline Packing get_how(void) const { return _how; }
    inline GuiItem* get_who(void) const { return _who; }
  };
  typedef vector<Connection> Connections;
  class Box {
  private:
    GuiItem* _thing;
    Connections _links;
  public:
    inline Box(void) : _thing((GuiItem*)0L) {}
    inline Box(GuiItem* i) : _thing(i) {}
    inline Box(const Box& c) : _thing(c._thing), _links(c._links) {}
    ~Box(void) {}

    inline void set_item(GuiItem* i) { _thing = i; }
    inline void add_link(Connection c) { _links.push_back(c); }

    inline GuiItem* get_item(void) const { return _thing; }
    inline int get_num_links(void) const { return _links.size(); }
    inline Packing get_nth_packing(int n) const { return _links[n].get_how(); }
    inline GuiItem* get_nth_to(int n) const { return _links[n].get_who(); }
  };
  typedef vector<Box> Boxes;

  Boxes _items;

  INLINE GuiFrame(void);
  Boxes::iterator find_box(GuiItem*);
  virtual void recompute_frame(void);
public:
  GuiFrame(const string&);
  ~GuiFrame(void);

  void add_item(GuiItem*);
  void pack_item(GuiItem*, Packing, GuiItem*);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

  virtual void set_scale(float);
  virtual void set_pos(const LVector3f&);
};

#include "guiFrame.I"

#endif /* __GUIFRAME_H__ */
