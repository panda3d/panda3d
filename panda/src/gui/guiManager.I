// Filename: guiManager.I
// Created by:  cary (25Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiManager::GuiManager(MouseWatcher* w, Node* n, EventHandler* h)
  : _start_draw_order(0), _next_draw_order(0), _root(n), _watcher(w),
    _eh(h) {
}

INLINE int GuiManager::get_next_draw_order(void) const {
  return _next_draw_order;
}

INLINE void GuiManager::set_next_draw_order(int v) {
#ifndef NDEBUG
  if (v < _next_draw_order)
    gui_cat->warning() << "GuiManager::set_next_draw_order(" << v
		       << ") is < existing (" << _next_draw_order << ")"
		       << endl;
#endif /* NDEBUG */
  this->_next_draw_order = v;
}

INLINE int GuiManager::get_start_draw_order(void) const {
  return _start_draw_order;
}

INLINE void GuiManager::set_start_draw_order(int v) {
  this->_start_draw_order = v;
}

INLINE EventHandler* GuiManager::get_private_handler(void) const {
  return _eh;
}
