// Filename: guiChooser.I
// Created by:  cary (08Feb01)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiChooser::GuiChooser(void) {}

INLINE void GuiChooser::set_loop(bool b) {
  _loop = b;
}

INLINE bool GuiChooser::get_loop(void) const {
  return _loop;
}
