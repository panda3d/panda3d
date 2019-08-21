INLINE CollisionHeightfield::
CollisionHeightfield() {
}

INLINE CollisionHeightfield::
CollisionHeightfield(const CollisionHeightfield &copy) {
}

INLINE void CollisionHeightfield::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

INLINE PNMImage CollisionHeightfield::
get_heightfield() {
  return _heightfield;
}

INLINE void CollisionHeightfield::
set_heightfield(PNMImage heightfield) {
  int r = _heightfield.get_read_x_size();
  int c = _heightfield.get_read_y_size();
  _heightfield = heightfield;

  if (_heightfield.get_read_x_size() == r &&
      _heightfield.get_read_y_size() == c) {
    fill_quadtree_heights();
  } else {
    fill_quadtree_areas();
    fill_quadtree_heights();
  }
}

INLINE PN_stdfloat CollisionHeightfield::
get_max_height() {
  return _max_height;
}

INLINE void CollisionHeightfield::
set_max_height(PN_stdfloat max_height) {
  _max_height = max_height;
  fill_quadtree_heights();
}

INLINE int CollisionHeightfield::
get_subdivisions() {
  return _subdivisions;
}

INLINE PN_stdfloat CollisionHeightfield::
get_height(int x, int y) const {
  return _heightfield.get_gray(x, y) * _max_height;
}
