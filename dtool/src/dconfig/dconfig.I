// Filename: dconfig.I
// Created by:  cary (20Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
