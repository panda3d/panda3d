// Filename: cullBin.cxx
// Created by:  drose (28Feb02)
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

#include "cullBin.h"
#include "config_pgraph.h"

PStatCollector CullBin::_cull_bin_pcollector("Cull:Sort");
PStatCollector CullBin::_draw_bin_pcollector("Draw:Bins");

TypeHandle CullBin::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBin::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullBin::
~CullBin() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::make_next
//       Access: Public, Virtual
//  Description: Returns a newly-allocated CullBin object that
//               contains a copy of just the subset of the data from
//               this CullBin object that is worth keeping around
//               for next frame.
//
//               If a particular CullBin object has no data worth
//               preserving till next frame, it is acceptable to
//               return NULL (which is the default behavior of this
//               method).
////////////////////////////////////////////////////////////////////
PT(CullBin) CullBin::
make_next() const {
  return (CullBin *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::finish_cull
//       Access: Public, Virtual
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullBin::
finish_cull(SceneSetup *, Thread *) {
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::check_flash_color
//       Access: Private
//  Description: Checks the config variables for a user variable of
//               the name flash-bin-binname.  If found, it defines the
//               r g b color to flash geometry in this bin.
////////////////////////////////////////////////////////////////////
void CullBin::
check_flash_color() {
#ifdef NDEBUG
  _has_flash_color = false;
#else
  ConfigVariableDouble flash_bin
    ("flash-bin-" + _name, "", "", ConfigVariable::F_dynamic);
  if (flash_bin.get_num_words() == 0) {
    _has_flash_color = false;

  } else if (flash_bin.get_num_words() == 3) {
    _has_flash_color = true;
    _flash_color.set(flash_bin[0], flash_bin[1], flash_bin[2], 1.0f);

  } else if (flash_bin.get_num_words() == 4) {
    _has_flash_color = true;
    _flash_color.set(flash_bin[0], flash_bin[1], flash_bin[2], flash_bin[3]);

  } else {
    _has_flash_color = false;
    pgraph_cat.warning()
      << "Invalid value for flash-bin-" << _name << ": " 
      << flash_bin.get_string_value() << "\n";
  }
#endif  // NDEBUG
}
