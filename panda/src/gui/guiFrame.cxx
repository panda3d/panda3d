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

  freeze();

  Boxes::iterator i;

  // go thru and make sure everything is packed correctly.  This is a stupid
  // and brute-force algorithm.  Hopefully it will be replaced with something
  // more ellegant later
    for (i=_items.begin(); i!=_items.end(); ++i) {
    GuiItem* here = (*i).get_item();
    here->recompute();
    int n = (*i).get_num_links();
    if (n > 0) {
      LVector4f ext_h = here->get_frame();
      LVector3f pos_h = here->get_pos();
      for (int j=0; j<n; ++j) {
	Packing pack = (*i).get_nth_packing(j);
	if (pack == NONE)
	  continue;
	GuiItem* to = (*i).get_nth_to(j);
	LVector4f ext_t = to->get_frame();
	float gap = (*i).get_nth_gap(j);
	switch (pack) {
	case ABOVE:
	  {
	    // to(top) - here(bottom)
	    float diff = ext_t[3] - ext_h[2] + gap;
	    LVector3f move = LVector3f::rfu(0., 0., diff);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case UNDER:
	  {
	    // to(bottom) - here(top)
	    float diff = ext_t[2] - ext_h[3] - gap;
	    LVector3f move = LVector3f::rfu(0., 0., diff);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case LEFT:
	  {
	    // to(left) - here(right)
	    float diff = ext_t[0] - ext_h[1] - gap;
	    LVector3f move = LVector3f::rfu(diff, 0., 0.);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case RIGHT:
	  {
	    // to(right) - here(left)
	    float diff = ext_t[1] - ext_h[0] + gap;
	    LVector3f move = LVector3f::rfu(diff, 0., 0.);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case ALIGN_ABOVE:
	  {
	    // to(top) - here(top)
	    float diff = ext_t[3] - ext_h[3];
	    LVector3f move = LVector3f::rfu(0., 0., diff);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case ALIGN_UNDER:
	  {
	    // to(bottom) - here(bottom)
	    float diff = ext_t[2] - ext_h[2];
	    LVector3f move = LVector3f::rfu(0., 0., diff);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case ALIGN_LEFT:
	  {
	    // to(left) - here(left)
	    float diff = ext_t[0] - ext_h[0];
	    LVector3f move = LVector3f::rfu(diff, 0., 0.);
	    here->set_pos(pos_h + move);
	    ext_h = here->get_frame();
	    pos_h = here->get_pos();
	  }
	  break;
	case ALIGN_RIGHT:
	  {
	    // to(right) - here(right)
	    float diff = ext_t[1] - ext_h[1];
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
  // get the bounds again
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
  // check for alignment to the DisplayRegion
  LVector3f move_left, move_right, move_top, move_bottom;
  move_left = move_right = move_top = move_bottom = LVector3f(0., 0., 0.);
  if (_align_to_left) {
    float diff = -1. - _left;
    move_left = LVector3f::rfu(diff, 0., 0.);
  }
  if (_align_to_right) {
    float diff = 1. - _right;
    move_right = LVector3f::rfu(diff, 0., 0.);
  }
  if (_align_to_top) {
    float diff = 1. - _top;
    move_top = LVector3f::rfu(0., 0., diff);
  }
  if (_align_to_bottom) {
    float diff = -1. - _bottom;
    move_bottom = LVector3f::rfu(0., 0., diff);
  }
  LVector3f move = move_left + move_right + move_top + move_bottom;
  for (i=_items.begin(); i!=_items.end(); ++i) {
    GuiItem* foo = (*i).get_item();
    foo->set_pos(foo->get_pos() + move);
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

  thaw();
}

void GuiFrame::set_priority(GuiLabel* l, const GuiItem::Priority p) {
  Boxes::iterator i;

  for (i=_items.begin(); i!=_items.end(); ++i) {
    GuiItem* here = (*i).get_item();
    here->set_priority(l, p);
  }
}

GuiFrame::GuiFrame(const string& name) : GuiItem(name), _align_to_left(false),
					 _align_to_right(false),
					 _align_to_top(false),
					 _align_to_bottom(false) {
}

GuiFrame::~GuiFrame(void) {
  this->unmanage();
}

int GuiFrame::freeze() {
  int result = 0;
  Boxes::iterator i;

  for (i=_items.begin(); i!=_items.end(); ++i) {
    GuiItem* here = (*i).get_item();
    int count = here->freeze();
    result = max(result, count);
  }

  return result;
}

int GuiFrame::thaw() {
  int result = 0;
  Boxes::iterator i;

  for (i=_items.begin(); i!=_items.end(); ++i) {
    GuiItem* here = (*i).get_item();
    int count = here->thaw();
    result = max(result, count);
  }

  return result;
}

void GuiFrame::add_item(GuiItem* item) {
  bool found = false;
  for (Boxes::iterator i=_items.begin(); (!found)&&(i!=_items.end()); ++i)
    if ((*i).get_item() == item)
      found = true;
  if (!found) {
    _items.push_back(Box(item));
    //    this->recompute_frame();
  }
}

void GuiFrame::remove_item(GuiItem* item) {
  Boxes::iterator i = find_box(item);
  if (i == _items.end())
    return;
  item->unmanage();
  (*i).erase_all_links();
  // should NEVER link forward in the items, only backward, so it should be
  // safe to start here, and go to the end
  for (Boxes::iterator j=i+1; j!=_items.end(); ++j) {
    bool done;
    do {
      done = true;
      for (int k=0; k<(*j).get_num_links(); ++k)
	if ((*j).get_nth_to(k) == item) {
	  done = false;
	  (*j).erase_nth_link(k);
	  break;
	}
    } while (!done);
  }
  // now get rid of the thing itself
  _items.erase(i);
  this->recompute();
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
  //  this->recompute_frame();
}

void GuiFrame::clear_packing(GuiItem* item) {
  Boxes::iterator box = find_box(item);
  (*box).erase_all_links();
  //  this->recompute_frame();
}

void GuiFrame::clear_all_packing(void) {
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i).erase_all_links();
  //  this->recompute_frame();
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

void GuiFrame::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  if (!_added_hooks) {
    _added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
      (*i).get_item()->manage(mgr, eh, n);
    GuiItem::manage(mgr, eh, n);
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
  //  this->recompute_frame();
}

void GuiFrame::set_scale(float x, float y, float z) {
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i).get_item()->set_scale(x * (*i).get_item()->get_scale_x(),
			       y * (*i).get_item()->get_scale_y(),
			       z * (*i).get_item()->get_scale_z());
  GuiItem::set_scale(x, y, z);
}

void GuiFrame::set_pos(const LVector3f& p) {
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i).get_item()->set_pos(p);
  GuiItem::set_pos(p);
  //  this->recompute_frame();
}

void GuiFrame::set_priority(GuiItem* it, const GuiItem::Priority p) {
  Boxes::iterator i;

  for (i=_items.begin(); i!=_items.end(); ++i) {
    GuiItem* here = (*i).get_item();
    here->set_priority(it, p);
  }
}

int GuiFrame::set_draw_order(int v) {
  int o;
  bool first = true;
  for (Boxes::iterator i=_items.begin(); i!=_items.end(); ++i)
    if (first) {
      first = false;
      o = (*i).get_item()->set_draw_order(v);
    } else
      o = (*i).get_item()->set_draw_order(o);
  return GuiItem::set_draw_order(o);
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
