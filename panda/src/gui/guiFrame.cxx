// Filename: guiFrame.cxx
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "guiFrame.h"

TypeHandle GuiFrame::_type_handle;

GuiFrame::Boxes::iterator GuiFrame::find_box(GuiItem* item) {
  bool found = false;
  Boxes::iterator ret = _items.end();
  Boxes::iterator i;
  for (i=_items.begin(); (!found)&&(i!=_items.end()); ++i)
    if ((*i).get_item() == item) {
      found = true;
      ret = i;
    }
  if (!found)
    ret = _items.end();
  return ret;
}

void GuiFrame::recompute_frame(void) {
  GuiItem::recompute_frame();
  Boxes::iterator i;
  // go thru and make sure everything is packed correctly.  This is a stupid
  // and brute-force algorithm.  Hopefully it will be replaced with something
  // more ellegant later
  for (i=_items.begin(); i!=_items.end(); ++i) {
    int n = (*i).get_num_links();
    if (n > 0) {
      GuiItem* here = (*i).get_item();
      LVector4f ext_h = here->get_frame();
      LVector3f pos_h = here->get_pos();
      for (int j=0; j<n; ++j) {
	Packing pack = (*i).get_nth_packing(j);
	if (pack == NONE)
	  continue;
	GuiItem* to = (*i).get_nth_to(j);
	LVector4f ext_t = to->get_frame();
	switch (pack) {
	case ABOVE:
	  {
	    // to(top) - here(bottom)
	    float diff = ext_t[3] - ext_h[2];
	    LVector3f move = LVector3f::rfu(0., 0., diff);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case UNDER:
	  {
	    // to(bottom) - here(top)
	    float diff = ext_t[2] - ext_h[3];
	    LVector3f move = LVector3f::rfu(0., 0., diff);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case LEFT:
	  {
	    // to(left) - here(right)
	    float diff = ext_t[0] - ext_h[1];
	    LVector3f move = LVector3f::rfu(diff, 0., 0.);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case RIGHT:
	  {
	    // to(right) - here(left)
	    float diff = ext_t[1] - ext_h[0];
	    LVector3f move = LVector3f::rfu(diff, 0., 0.);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	default:
	  gui_cat->warning() << "unknown packing type (" << (int)pack << ")"
			     << endl;
	}
      }
    }
  }
  // these should be exactly opposite the max, so they will (hopefully) be
  // overwritten
  _left = _bottom = 10000000.;
  _right = _top = -10000000.;
  for (i=_items.begin(); i!=_items.end(); ++i) {
    float tmp = (*i).get_item()->get_left();
    _left = (_left<tmp)?_left:tmp;
    tmp = (*i).get_item()->get_right();
    _right = (_right<tmp)?tmp:_right;
    tmp = (*i).get_item()->get_bottom();
    _bottom = (_bottom<tmp)?_bottom:tmp;
    tmp = (*i).get_item()->get_top();
    _top = (_top<tmp)?tmp:_top;
  }
  // now put it at the position it is supposed to be
  float x = (_left + _right) / 2.;
  float y = (_bottom + _top) / 2.;
  LVector3f pos = LVector3f::rfu(x, 0., y);
  LVector3f delta = _pos - pos;
  for (i=_items.begin(); i!=_items.end(); ++i) {
    GuiItem* foo = (*i).get_item();
    foo->set_pos(foo->get_pos() + delta);
  }
  // lastly, get the finial bounds
  _left = _bottom = 10000000.;
  _right = _top = -10000000.;
  for (i=_items.begin(); i!=_items.end(); ++i) {
    float tmp = (*i).get_item()->get_left();
    _left = (_left<tmp)?_left:tmp;
    tmp = (*i).get_item()->get_right();
    _right = (_right<tmp)?tmp:_right;
    tmp = (*i).get_item()->get_bottom();
    _bottom = (_bottom<tmp)?_bottom:tmp;
    tmp = (*i).get_item()->get_top();
    _top = (_top<tmp)?tmp:_top;
  }
}

GuiFrame::GuiFrame(const string& name) : GuiItem(name) {
}

GuiFrame::~GuiFrame(void) {
  this->unmanage();
}

void GuiFrame::add_item(GuiItem* item) {
  bool found = false;
  for (Boxes::iterator i=_items.begin(); (!found)&&(i!=_items.end()); ++i)
    if ((*i).get_item() == item)
      found = true;
  if (!found) {
    _items.push_back(Box(item));
    this->recompute_frame();
  }
}

void GuiFrame::pack_item(GuiItem* item, Packing rel, GuiItem* to, float gap) {
  Boxes::iterator box = find_box(item);
  if (box == _items.end()) {
    gui_cat->warning() << "tried to pack an item we don't have yet" << endl;
    return;
  }
  Boxes::iterator tobox = find_box(to);
  if (tobox == _items.end()) {
    gui_cat->warning()
      << "tried to pack an item relative to something we don't have" << endl;
    return;
  }
  (*box).add_link(Connection(rel, to, gap));
  this->recompute_frame();
}

void GuiFrame::clear_packing(GuiItem* item) {
  Boxes::iterator box = find_box(item);
  (*box).erase_all_links();
  this->recompute_frame();
}

void GuiFrame::clear_all_packing(void) {
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i).erase_all_links();
  this->recompute_frame();
}

void GuiFrame::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks) {
    _added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
      (*i).get_item()->manage(mgr, eh);
    GuiItem::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage frame (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiFrame::unmanage(void) {
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i).get_item()->unmanage();
  GuiItem::unmanage();
}

void GuiFrame::set_scale(float f) {
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i).get_item()->set_scale(f * (*i).get_scale());
  GuiItem::set_scale(f);
  this->recompute_frame();
}

void GuiFrame::set_pos(const LVector3f& p) {
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i).get_item()->set_pos(p);
  GuiItem::set_pos(p);
  this->recompute_frame();
}

void GuiFrame::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Frame data:" << endl;
  Boxes::const_iterator i;
  for (i=_items.begin(); i!=_items.end(); ++i) {
    os << "    box - 0x" << (void*)((*i).get_item()) << endl;
    int n = (*i).get_num_links();
    if (n > 0) {
      for (int j=0; j<n; ++j)
	os << "      linked by " << (int)((*i).get_nth_packing(j)) << " to 0x"
	   << (void*)((*i).get_nth_to(j)) << endl;
    }
  }
  for (i=_items.begin(); i!=_items.end(); ++i) {
    os << *((*i).get_item());
  }
}
