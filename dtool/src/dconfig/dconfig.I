// Filename: dconfig.I
// Created by:  cary (20Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


INLINE double get_total_time_config_init() {
  return (double)total_time_config_init / (double)CLOCKS_PER_SEC;
}

INLINE double get_total_time_external_init() {
  return (double)total_time_external_init / (double)CLOCKS_PER_SEC;
}

INLINE double get_total_num_get() {
  return total_num_get;
}
