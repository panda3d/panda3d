// Filename: chanwindow.I
// Created by:  cary (06Feb99)
// 
////////////////////////////////////////////////////////////////////

INLINE WindowItem::WindowItem(void) {}

INLINE WindowItem::WindowItem(const bool h, const bool d, const int o,
			      const std::string& l, const SetupSyms& s,
			      const int x, const int y, const bool b,
			      const bool c) : _hw_chans(h), _dvr(d),
                                              _border(b), _cursor(c),
                                              _chan_offset(o), _sizeX(x),
                                              _sizeY(y), _layout(l),
                                              _setups(s) {}

INLINE WindowItem::WindowItem(const WindowItem& c) : _hw_chans(c._hw_chans),
						     _dvr(c._dvr),
						     _border(c._border),
						     _cursor(c._cursor),
						     _chan_offset(c._chan_offset),
						     _sizeX(c._sizeX),
						     _sizeY(c._sizeY),
						     _layout(c._layout),
						     _setups(c._setups) {}

INLINE WindowItem::~WindowItem(void) {}

INLINE WindowItem& WindowItem::operator=(const WindowItem& c) {
  _hw_chans = c._hw_chans;
  _dvr = c._dvr;
  _border = c._border;
  _cursor = c._cursor;
  _chan_offset = c._chan_offset;
  _sizeX = c._sizeX;
  _sizeY = c._sizeY;
  _layout = c._layout;
  _setups = c._setups;
  return *this;
}

INLINE bool WindowItem::getHWChans(void) const {
  return _hw_chans;
}

INLINE bool WindowItem::getDVR(void) const {
  return _dvr;
}

INLINE bool WindowItem::getBorder(void) const {
  return _border;
}

INLINE bool WindowItem::getCursor(void) const {
  return _cursor;
}

INLINE int WindowItem::getChanOffset(void) const {
  return _chan_offset;
}

INLINE int WindowItem::getSizeX(void) const {
  return _sizeX;
}

INLINE int WindowItem::getSizeY(void) const {
  return _sizeY;
}

INLINE std::string WindowItem::getLayout(void) const {
  return _layout;
}

INLINE SetupSyms WindowItem::getSetups(void) const {
  return _setups;
}
