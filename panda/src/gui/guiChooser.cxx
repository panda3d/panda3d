// Filename: guiChooser.cxx
// Created by:  cary (08Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "guiChooser.h"

TypeHandle GuiChooser::_type_handle;

GuiChooser::ChooseFunctor::ChooseFunctor(GuiChooser* ch,
					 GuiBehavior::BehaviorFunctor* func)
  : _prev(func), _ch(ch) {
}

GuiChooser::ChooseFunctor::~ChooseFunctor(void) {
}

void GuiChooser::ChooseFunctor::doit(GuiBehavior* b) {
  if ((b == this->_ch->_prev_button) && this->_ch->_prev_button->is_active())
    this->_ch->move_prev();
  if ((b == this->_ch->_next_button) && this->_ch->_next_button->is_active())
    this->_ch->move_next();
  if (_prev != (GuiBehavior::BehaviorFunctor*)0L)
    _prev->doit(b);
}

void GuiChooser::recompute_frame(void) {
  float r, l, t, b;

  r = t = -1.;
  l = b = 1.;
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i) {
    if (r < (*i)->get_right())
      r = (*i)->get_right();
    if (l > (*i)->get_left())
      l = (*i)->get_left();
    if (b > (*i)->get_bottom())
      b = (*i)->get_bottom();
    if (t < (*i)->get_top())
      t = (*i)->get_top();
  }
  if (r < _prev_button->get_right())
    r = _prev_button->get_right();
  if (l > _prev_button->get_left())
    l = _prev_button->get_left();
  if (b > _prev_button->get_bottom())
    b = _prev_button->get_bottom();
  if (t < _prev_button->get_top())
    t = _prev_button->get_top();
  if (r < _next_button->get_right())
    r = _next_button->get_right();
  if (l > _next_button->get_left())
    l = _next_button->get_left();
  if (b > _next_button->get_bottom())
    b = _next_button->get_bottom();
  if (t < _next_button->get_top())
    t = _next_button->get_top();

  _left = l;
  _right = r;
  _bottom = b;
  _top = t;

  GuiBehavior::recompute_frame();
  this->adjust_region();
}

void GuiChooser::set_priority(GuiLabel* l, GuiItem::Priority p) {
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_priority(l, p);
  _prev_button->set_priority(l, p);
  _next_button->set_priority(l, p);
}

GuiChooser::GuiChooser(const string& name, GuiButton* prev, GuiButton* next)
  : GuiBehavior(name), _curr(-1), _loop(false), _prev_button(prev),
    _next_button(next), _prev_functor((GuiChooser::ChooseFunctor*)0L),
    _next_functor((GuiChooser::ChooseFunctor*)0L) {
}

GuiChooser::~GuiChooser(void) {
  this->unmanage();
}

void GuiChooser::adjust_buttons(void) {
  if (_curr < 0)
    return;
  if (_curr == 0) {
    _prev_button->exit();
    _prev_button->inactive();
  } else {
    _prev_button->up();
    if (_behavior_running) {
      _prev_button->start_behavior();
      _prev_button->set_behavior_functor(_prev_functor);
    }
  }
  int foo = _items.size() - 1;
  if (_curr == foo) {
    _next_button->exit();
    _next_button->inactive();
  } else {
    _next_button->up();
    if (_behavior_running) {
      _next_button->start_behavior();
      _next_button->set_behavior_functor(_next_functor);
    }
  }
}

void GuiChooser::move_prev(void) {
  if (_curr == -1)
    return;
  int tmp = _curr - 1;
  if (tmp < 0) {
    if (_loop)
      tmp += _items.size();
    else
      return;
  }
  if (_mgr != (GuiManager*)0L) {
    _items[_curr]->unmanage();
    if (_alt_root.is_null())
      _items[tmp]->manage(_mgr, *_eh);
    else
      _items[tmp]->manage(_mgr, *_eh, _alt_root);
    _curr = tmp;
    this->adjust_buttons();
    _mgr->recompute_priorities();
  } else
    _curr = tmp;
}

void GuiChooser::move_next(void) {
  if (_curr == -1)
    return;
  int tmp = _curr + 1;
  int foo = _items.size();
  if (tmp == foo) {
    if (_loop)
      tmp = 0;
    else
      return;
  }
  if (_mgr != (GuiManager*)0L) {
    _items[_curr]->unmanage();
    if (_alt_root.is_null())
      _items[tmp]->manage(_mgr, *_eh);
    else
      _items[tmp]->manage(_mgr, *_eh, _alt_root);
    _curr = tmp;
    this->adjust_buttons();
    _mgr->recompute_priorities();
  } else
    _curr = tmp;
}

void GuiChooser::add_item(GuiItem* item) {
  _items.push_back(item);
  if (_curr == -1)
    _curr = 0;
  this->recompute_frame();
}

int GuiChooser::freeze(void) {
  int result = 0;

  _prev_button->freeze();
  _next_button->freeze();
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i) {
    int count = (*i)->freeze();
    result = max(result, count);
  }
  return result;
}

int GuiChooser::thaw(void) {
  int result = 0;

  _prev_button->thaw();
  _next_button->thaw();
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i) {
    int count = (*i)->thaw();
    result = max(result, count);
  }
  return result;
}

void GuiChooser::manage(GuiManager* mgr, EventHandler& eh) {
  if (_mgr == (GuiManager*)0L) {
    _prev_button->manage(mgr, eh);
    _next_button->manage(mgr, eh);
    if (_curr != -1) {
      _items[_curr]->manage(mgr, eh);
      if (_curr == 0)
	_prev_button->inactive();
      int foo = _items.size() - 1;
      if (_curr == foo)
	_next_button->inactive();
    } else {
      _prev_button->inactive();
      _next_button->inactive();
    }
    GuiBehavior::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage chooser (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiChooser::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  if (_mgr == (GuiManager*)0L) {
    _prev_button->manage(mgr, eh, n);
    _next_button->manage(mgr, eh, n);
    if (_curr != -1) {
      _items[_curr]->manage(mgr, eh, n);
      if (_curr == 0)
	_prev_button->inactive();
      int foo = _items.size() - 1;
      if (_curr == foo)
	_next_button->inactive();
    } else {
      _prev_button->inactive();
      _next_button->inactive();
    }
    GuiBehavior::manage(mgr, eh, n);
  } else
    gui_cat->warning() << "tried to manage chooser (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiChooser::unmanage(void) {
  _prev_button->unmanage();
  _next_button->unmanage();
  if (_curr != -1)
    _items[_curr]->unmanage();
  GuiBehavior::unmanage();
}

void GuiChooser::set_scale(float f) {
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_scale(f);
  GuiBehavior::set_scale(f);
}

void GuiChooser::set_scale(float x, float y, float z) {
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_scale(x, y, z);
  GuiBehavior::set_scale(x, y, z);
}

void GuiChooser::set_pos(const LVector3f& p) {
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_pos(p);
  GuiBehavior::set_pos(p);
}

void GuiChooser::start_behavior(void) {
  GuiBehavior::start_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  if (_prev_functor != (GuiChooser::ChooseFunctor*)0L) {
    _prev_button->set_behavior_functor(_prev_functor->get_prev());
    delete _prev_functor;
  }
  _prev_functor =
    new GuiChooser::ChooseFunctor(this, _prev_button->get_behavior_functor());
  _prev_button->set_behavior_functor(_prev_functor);
  _prev_button->start_behavior();
  if (_next_functor != (GuiChooser::ChooseFunctor*)0L) {
    _next_button->set_behavior_functor(_next_functor->get_prev());
    delete _next_functor;
  }
  _next_functor =
    new GuiChooser::ChooseFunctor(this, _next_button->get_behavior_functor());
  _next_button->set_behavior_functor(_next_functor);
  _next_button->start_behavior();
}

void GuiChooser::stop_behavior(void) {
  GuiBehavior::stop_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  if (_prev_functor != (GuiChooser::ChooseFunctor*)0L) {
    _prev_button->set_behavior_functor(_prev_functor->get_prev());
    delete _prev_functor;
    _prev_functor = (GuiChooser::ChooseFunctor*)0L;
    _prev_button->stop_behavior();
  }
  if (_next_functor != (GuiChooser::ChooseFunctor*)0L) {
    _next_button->set_behavior_functor(_next_functor->get_prev());
    delete _next_functor;
    _next_functor = (GuiChooser::ChooseFunctor*)0L;
    _next_button->stop_behavior();
  }
}

void GuiChooser::reset_behavior(void) {
  GuiBehavior::reset_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  if (_prev_functor != (GuiChooser::ChooseFunctor*)0L)
    _prev_button->reset_behavior();
  if (_next_functor != (GuiChooser::ChooseFunctor*)0L)
    _next_button->reset_behavior();
}

void GuiChooser::set_priority(GuiItem* it, const GuiItem::Priority p) {
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_priority(it, p);
  _prev_button->set_priority(it, p);
  _next_button->set_priority(it, p);
}

int GuiChooser::set_draw_order(int v) {
  int o = _prev_button->set_draw_order(v);
  o = _next_button->set_draw_order(o);
  for (ItemVector::iterator i=_items.begin(); i!=_items.end(); ++i)
    o = (*i)->set_draw_order(o);
  return GuiBehavior::set_draw_order(o);
}

void GuiChooser::set_curr_item(int i) {
  int foo = _items.size();
  int ctmp = i;
  if (i < 0)
    return;
  if (i > foo) {
    if (_loop)
      ctmp = i % foo;
    else
      return;
  }
  if (_mgr != (GuiManager*)0L) {
    _items[_curr]->unmanage();
    if (_alt_root.is_null())
      _items[ctmp]->manage(_mgr, *_eh);
    else
      _items[ctmp]->manage(_mgr, *_eh, _alt_root);
    _curr = ctmp;
    this->adjust_buttons();
    _mgr->recompute_priorities();
  } else
    _curr = ctmp;
}

void GuiChooser::output(ostream& os) const {
  GuiBehavior::output(os);
  os << "  Chooser data:" << endl;
  os << "    prev button - 0x" << (void*)_prev_button << endl;
  os << "    next button - 0x" << (void*)_next_button << endl;
  os << "    current - " << _curr << endl;
  os << "    items (" << _items.size() << "):" << endl;
  for (ItemVector::const_iterator i=_items.begin(); i!=_items.end(); ++i)
    os << "      0x" << (void*)(*i) << endl;
}
