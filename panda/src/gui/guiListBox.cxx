// Filename: guiListBox.cxx
// Created by:  cary (18Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "guiListBox.h"

TypeHandle GuiListBox::_type_handle;
TypeHandle GuiListBox::ListFunctor::_type_handle;

GuiListBox::ListFunctor::ListFunctor(GuiListBox* box,
                                     GuiBehavior::BehaviorFunctor* func)
  : GuiBehavior::BehaviorFunctor(), _prev(func), _lb(box) {
}

GuiListBox::ListFunctor::~ListFunctor(void) {
  _prev.clear();
}

void GuiListBox::ListFunctor::doit(GuiBehavior* b) {
  if ((b == this->_lb->_up_arrow) && this->_lb->_up_arrow->is_active())
    this->_lb->scroll_up();
  if ((b == this->_lb->_down_arrow) && this->_lb->_down_arrow->is_active())
    this->_lb->scroll_down();
  if (_prev != (GuiBehavior::BehaviorFunctor*)0L)
    _prev->doit(b);
}

void GuiListBox::recompute_frame(void) {
  this->freeze();
  LVector3f p = _pos;
  float lft = 100000.;
  float rgt = -100000.;
  float tp = -100000.;
  float btm = 100000.;
  LVector4f frm;
  for (ItemVector::iterator i=_visible.begin(); i!=_visible.end();
       p-=((*i)->get_height() * LVector3f::up()), ++i) {
    (*i)->set_pos(p);
    frm = (*i)->get_frame();
    if (frm[0] < lft)
      lft = frm[0];
    if (frm[1] > rgt)
      rgt = frm[1];
    if (frm[2] < btm)
      btm = frm[2];
    if (frm[3] > tp)
      tp = frm[3];
    (*i)->freeze();
    int lvl = (*i)->thaw();
#ifdef _DEBUG
    gui_cat->debug() << "in recompute: freeze lvl (" << (*i)->get_name()
                     << ") = " << lvl << endl;
#endif
  }
  _left = lft;
  _right = rgt;
  _top = tp;
  _bottom = btm;
  GuiBehavior::recompute_frame();
  this->adjust_region();
  this->thaw();
  this->deal_with_buttons();
}

void GuiListBox::deal_with_buttons(void) {
  // check the state of the up and down buttons
  if (_bottom_stack.size() == 0) {
    _down_arrow->exit();
    _down_arrow->inactive();
  } else {
    _down_arrow->up();
    if (_behavior_running) {
      _down_arrow->start_behavior();
      _down_arrow->set_behavior_functor(_down_functor);
    }
  }
  if (_top_stack.size() == 0) {
    _up_arrow->exit();
    _up_arrow->inactive();
  } else {
    _up_arrow->up();
    if (_behavior_running) {
      _up_arrow->start_behavior();
      _up_arrow->set_behavior_functor(_up_functor);
    }
  }
}

void GuiListBox::set_priority(GuiLabel* l, const GuiItem::Priority p) {
  ItemVector::iterator i;
  ItemDeque::iterator j;

  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    (*i)->set_priority(l, p);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->set_priority(l, p);
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j)
    (*j)->set_priority(l, p);
  _up_arrow->set_priority(l, p);
  _down_arrow->set_priority(l, p);
}

GuiListBox::GuiListBox(const string& name, int N, GuiButton* up,
                       GuiButton* down)
  : GuiBehavior(name), _up_arrow(up), _down_arrow(down), _n_visible(N),
    _up_functor((GuiListBox::ListFunctor*)0L),
    _down_functor((GuiListBox::ListFunctor*)0L) {
  if (N < 4) {
    gui_cat->warning() << "ListBoxes should have at least 4 visible slots"
                       << endl;
    _n_visible = 4;
  }
}

GuiListBox::~GuiListBox(void) {
  this->unmanage();
}

void GuiListBox::scroll_down(void) {
  if (_bottom_stack.size() == 0)
    return;   // nothing to scroll
  // compute what the first and list item in the visible list are
  int first = 0;
  int last = _n_visible-1;
  // first push one off the top onto the top stack
  _top_stack.push_back(_visible[first]);
  _visible[first]->unmanage();
  // then move everything up one
  for (int i=first; i!=last; ++i)
    _visible[i] = _visible[i+1];
  // then add one from the bottom stack to the bottom
  _visible[last] = *(_bottom_stack.rbegin());
  if (_mgr != (GuiManager*)0L) {
    if (_alt_root.is_null())
      _visible[last]->manage(_mgr, *_eh);
    else
      _visible[last]->manage(_mgr, *_eh, _alt_root);
  }
  // and pop it off the bottom stack
  _bottom_stack.pop_back();
  this->deal_with_buttons();
  // finally recompute all the possitions
  this->recompute_frame();
  if (_mgr != (GuiManager*)0L)
    _mgr->recompute_priorities();
}

void GuiListBox::scroll_up(void) {
  if (_top_stack.size() == 0)
    return;  // nothing to scroll
  // compute what the first and last item in the visible list are
  int first = 0;
  int last = _n_visible - 1;
  // first push one off the bottom onto the bottom stack
  _bottom_stack.push_back(_visible[last]);
  _visible[last]->unmanage();
  // then move everything down one
  for (int i=last; i!=first; --i)
    _visible[i] = _visible[i-1];
  // then add one from the top stack to the top
  _visible[first] = *(_top_stack.rbegin());
  if (_mgr != (GuiManager*)0L) {
    if (_alt_root.is_null())
      _visible[first]->manage(_mgr, *_eh);
    else
      _visible[first]->manage(_mgr, *_eh, _alt_root);
  }
  // and pop it off the top stack
  _top_stack.pop_back();
  this->deal_with_buttons();
  // finally recompute all the possitions
  this->recompute_frame();
  if (_mgr != (GuiManager*)0L)
    _mgr->recompute_priorities();
}

void GuiListBox::add_item(GuiItem* item) {
  if (_bottom_stack.size() > 0)
    _bottom_stack.push_front(item);
  else {
    if (_visible.size() < _n_visible) {
      _visible.push_back(item);
      if (_mgr != (GuiManager*)0L) {
        if (_alt_root.is_null())
          item->manage(_mgr, *_eh);
        else
          item->manage(_mgr, *_eh, _alt_root);
      }
    } else
      _bottom_stack.push_back(item);
  }
  this->deal_with_buttons();
}

int GuiListBox::freeze(void) {
  int result = 0;
  ItemVector::iterator i;
  ItemDeque::iterator j;

  gui_cat->spam() << "GuiListBox::freeze()" << endl;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i) {
    int count = (*i)->freeze();
    result = max(result, count);
  }
  for (i=_visible.begin(); i!=_visible.end(); ++i) {
    int count = (*i)->freeze();
    result = max(result, count);
  }
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j) {
    int count = (*j)->freeze();
    result = max(result, count);
  }
  _up_arrow->freeze();
  _down_arrow->freeze();
  return result;
}

int GuiListBox::thaw(void) {
  int result = 0;
  ItemVector::iterator i;
  ItemDeque::iterator j;

  gui_cat->spam() << "GuiListBox::thaw()" << endl;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i) {
    int count = (*i)->thaw();
    result = max(result, count);
  }
  for (i=_visible.begin(); i!=_visible.end(); ++i) {
    int count = (*i)->thaw();
    result = max(result, count);
  }
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j) {
    int count = (*j)->thaw();
    result = max(result, count);
  }
  _up_arrow->thaw();
  _down_arrow->thaw();
  return result;
}

void GuiListBox::manage(GuiManager* mgr, EventHandler& eh) {
  if (_mgr == (GuiManager*)0L) {
    this->recompute_frame();
    for (ItemVector::iterator i=_visible.begin(); i!=_visible.end(); ++i)
      (*i)->manage(mgr, eh);
    //    _up_arrow->manage(mgr, eh);
    //    _down_arrow->manage(mgr, eh);
    this->deal_with_buttons();
    GuiBehavior::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage listbox (0x" << (void*)this
                       << ") that is already managed" << endl;
}

void GuiListBox::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  if (_mgr == (GuiManager*)0L) {
    this->recompute_frame();
    for (ItemVector::iterator i=_visible.begin(); i!=_visible.end(); ++i)
      (*i)->manage(mgr, eh, n);
    //    _up_arrow->manage(mgr, eh, n);
    //    _down_arrow->manage(mgr, eh, n);
    this->deal_with_buttons();
    GuiBehavior::manage(mgr, eh, n);
  } else
    gui_cat->warning() << "tried to manage listbox (0x" << (void*)this
                       << ") that is already managed" << endl;
}

void GuiListBox::unmanage(void) {
  for (ItemVector::iterator i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->unmanage();
  //  _up_arrow->unmanage();
  //  _down_arrow->unmanage();
  GuiBehavior::unmanage();
}

void GuiListBox::set_scale(float f) {
  ItemVector::iterator i;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    (*i)->set_scale(f);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->set_scale(f);
  for (ItemDeque::iterator j=_bottom_stack.begin(); j!=_bottom_stack.end();
       ++j)
    (*j)->set_scale(f);
  /*
  _up_arrow->set_scale(f);
  _down_arrow->set_scale(f);
  */
  GuiBehavior::set_scale(f);
  this->recompute_frame();
}

void GuiListBox::set_scale(float x, float y, float z) {
  ItemVector::iterator i;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    (*i)->set_scale(x, y, z);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->set_scale(x, y, z);
  for (ItemDeque::iterator j=_bottom_stack.begin(); j!=_bottom_stack.end();
       ++j)
    (*j)->set_scale(x, y, z);
  /*
  _up_arrow->set_scale(x, y, z);
  _down_arrow->set_scale(x, y, z);
  */
  GuiBehavior::set_scale(x, y, z);
  this->recompute_frame();
}

void GuiListBox::set_pos(const LVector3f& p) {
  /*
  ItemVector::iterator i;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    (*i)->set_pos(p);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->set_pos(p);
  for (ItemDeque::iterator j=_bottom_stack.begin(); j!=_bottom_stack.end();
       ++j)
    (*j)->set_pos(p);
  */
  /*
  _up_arrow->set_pos(p);
  _down_arrow->set_pos(p);
  */
  GuiBehavior::set_pos(p);
  this->recompute_frame();
}

void GuiListBox::start_behavior(void) {
  GuiBehavior::start_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  if (_up_functor != (GuiListBox::ListFunctor*)0L) {
    _up_arrow->set_behavior_functor(_up_functor->get_prev());
    _up_functor.clear();
  }
  _up_functor =
    new GuiListBox::ListFunctor(this, _up_arrow->get_behavior_functor());
  _up_arrow->set_behavior_functor(_up_functor);
  _up_arrow->start_behavior();
  if (_down_functor != (GuiListBox::ListFunctor*)0L) {
    _down_arrow->set_behavior_functor(_down_functor->get_prev());
    _down_functor.clear();
  }
  _down_functor =
    new GuiListBox::ListFunctor(this, _down_arrow->get_behavior_functor());
  _down_arrow->set_behavior_functor(_down_functor);
  _down_arrow->start_behavior();
}

void GuiListBox::stop_behavior(void) {
  GuiBehavior::stop_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  if (_up_functor != (GuiListBox::ListFunctor*)0L) {
    _up_arrow->set_behavior_functor(_up_functor->get_prev());
    _up_functor.clear();
    _up_arrow->stop_behavior();
  }
  if (_down_functor != (GuiListBox::ListFunctor*)0L) {
    _down_arrow->set_behavior_functor(_down_functor->get_prev());
    _down_functor.clear();
    _down_arrow->stop_behavior();
  }
}

void GuiListBox::reset_behavior(void) {
  GuiBehavior::reset_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  if (_up_functor != (GuiListBox::ListFunctor*)0L)
    _up_arrow->reset_behavior();
  if (_down_functor != (GuiListBox::ListFunctor*)0L)
    _down_arrow->reset_behavior();
}

void GuiListBox::set_priority(GuiItem* it, const GuiItem::Priority p) {
  ItemVector::iterator i;
  ItemDeque::iterator j;

  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    (*i)->set_priority(it, p);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->set_priority(it, p);
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j)
    (*j)->set_priority(it, p);
  _up_arrow->set_priority(it, p);
  _down_arrow->set_priority(it, p);
}

int GuiListBox::set_draw_order(int v) {
  int o = _up_arrow->set_draw_order(v);
  o = _down_arrow->set_draw_order(o);

  ItemVector::iterator i;
  ItemDeque::iterator j;

  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    o = (*i)->set_draw_order(o);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    o = (*i)->set_draw_order(o);
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j)
    o = (*j)->set_draw_order(o);
  return GuiBehavior::set_draw_order(o);
}

void GuiListBox::output(ostream& os) const {
  GuiBehavior::output(os);
  os << "  Listbox data:" << endl;
  os << "    up_arrow = (0x" << (void*)_up_arrow << ")" << endl;
  os << "    down_arrow = (0x" << (void*)_down_arrow << ")" << endl;
  os << "    Top stack (" << _top_stack.size() << "):" << endl;
  ItemVector::const_iterator i;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    os << "      0x" << (void*)(*i) << " (" << (*i)->get_name() << ")" << endl;
  os << "    Visible (" << _visible.size() << "):" << endl;
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    os << "      0x" << (void*)(*i) << " (" << (*i)->get_name() << ")" << endl;
  os << "    Bottom stack (" << _bottom_stack.size() << "):" << endl;
  ItemDeque::const_iterator j;
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j)
    os << "      0x" << (void*)(*j) << " (" << (*j)->get_name() << ")" << endl;
  os << *_up_arrow;
  os << *_down_arrow;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    os << *(*i);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    os << *(*i);
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j)
    os << *(*j);
}
