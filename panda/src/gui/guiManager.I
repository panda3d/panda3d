// Filename: guiManager.I
// Created by:  cary (25Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiManager::GuiManager(MouseWatcher* w, Node* n) : _next_draw_order(0),
							  _root(n),
							  _watcher(w) {
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
