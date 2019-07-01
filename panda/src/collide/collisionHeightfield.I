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

INLINE PNMImage &CollisionHeightfield::
heightfield() {
  return _heightfield;
}

INLINE double CollisionHeightfield::
get_height(int x, int y) const {
  return _heightfield.get_gray(x, y) * _max_height;
}
