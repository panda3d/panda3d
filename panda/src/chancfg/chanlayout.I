// Filename: chanlayout.I
// Created by:  cary (04Feb99)
// 
////////////////////////////////////////////////////////////////////

INLINE LayoutItem::LayoutItem(void) {}

INLINE LayoutItem::LayoutItem(int x, int y) : _x(x), _y(y) {}

INLINE LayoutItem::LayoutItem(const LayoutItem& c) : _x(c._x), _y(c._y),
                                                     _regions(c._regions) {}

INLINE LayoutItem::~LayoutItem(void) {}

INLINE LayoutItem& LayoutItem::operator=(const LayoutItem& c) {
  _x = c._x;
  _y = c._y;
  _regions = c._regions;
  return *this;
}

INLINE void LayoutItem::AddRegion(const ChanViewport& r) {
  _regions.push_back(r);
}

INLINE int LayoutItem::GetNumRegions(void) {
  return _regions.size();
}

INLINE const ChanViewport& LayoutItem::operator[](int i) {
  return _regions[i];
}
