// Filename: guiBackground.I
// Created by:  cary (05Feb01)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiBackground::GuiBackground(void) {
}

INLINE void GuiBackground::set_color(float r, float g, float b, float a) {
  _bg->set_foreground_color(r, g, b, a);
}

INLINE void GuiBackground::set_color(const Colorf& c) {
  _bg->set_foreground_color(c);
}

INLINE Colorf GuiBackground::get_color(void) const {
  return _bg->get_foreground_color();
}

INLINE void GuiBackground::reassert(void) {
  _item->set_priority(_bg, P_High);
}
