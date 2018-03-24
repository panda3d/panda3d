/**
 * Returns the singleton EventMapper instance.
 */
INLINE EventMapper *EventMapper::
get_global_ptr() {
  if (_global_ptr != nullptr) {
    return _global_ptr;
  } else {
    make_global_ptr();
    return _global_ptr;
  }
}
