// Filename: config.I
// Created by:  cary (20Mar00)
//
///////////////////////////////////////////////////////////////////////


INLINE double get_total_time_config_init() {
  return (double)total_time_config_init / (double)CLOCKS_PER_SEC;
}

INLINE double get_total_time_external_init() {
  return (double)total_time_external_init / (double)CLOCKS_PER_SEC;
}

INLINE double get_total_num_get() {
  return total_num_get;
}
