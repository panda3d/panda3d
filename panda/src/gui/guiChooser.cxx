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
  if (b == this->_ch->_prev_button)
    this->_ch->move_prev();
  if (b == this->_ch->_next_button)
    this->_ch->move_next();
}

void GuiChooser::recompute_frame(void) {
}

GuiChooser::GuiChooser(const string& name, GuiButton* prev, GuiButton* next)
  : GuiBehavior(name), _curr(-1), _prev_button(prev), _next_button(next),
    _prev_functor((GuiChooser::ChooseFunctor*)0L),
    _next_functor((GuiChooser::ChooseFunctor*)0L) {
}

GuiChooser::~GuiChooser(void) {
  this->unmanage();
}

void GuiChooser::move_prev(void) {
  if (_curr == -1)
    return;
}

void GuiChooser::move_next(void) {
  if (_curr == -1)
    return;
}

void GuiChooser::add_item(GuiItem* item) {
}

int GuiChooser::freeze(void) {
  int result = 0;

  if (_curr != -1)
    result = _items[_curr]->freeze();
  return result;
}

int GuiChooser::thaw(void) {
  int result = 0;

  if (_curr != -1)
    result = _items[_curr]->thaw();
  return result;
}

void GuiChooser::manage(GuiManager* mgr, EventHandler& eh) {
  if (_mgr == (GuiManager*)0L) {
    _prev_button->manage(mgr, eh);
    _next_button->manage(mgr, eh);
    if (_curr != -1)
      _items[_curr]->manage(mgr, eh);
    GuiBehavior::manage(mgr, eh);
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
  GuiBehavior::set_scale(f);
}

void GuiChooser::set_pos(const LVector3f& p) {
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
