// Filename: guiManager.I
// Created by:  cary (25Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiManager* GuiManager::get_ptr(void) {
  if (_singleton == (GuiManager*)0L) {
    _singleton = new GuiManager;
  }
  return _singleton;
}
