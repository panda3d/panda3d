// Filename: guiRegion.I
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiRegion::GuiRegion(void) {
}

INLINE GuiRegion::GuiRegion(const string& name, float l, float r, float b,
			    float t, bool buttons) : Namable(name), _left(l),
						     _right(r), _bottom(b),
						     _top(t) {
  _region = new MouseWatcherRegion(name, l, r, b, t);
  _region->set_suppress_below(buttons);
}

INLINE MouseWatcherRegion* GuiRegion::get_region(void) const {
  return _region;
}

INLINE void GuiRegion::trap_clicks(bool t) {
  _region->set_suppress_below(t);
}
