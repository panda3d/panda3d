// Filename: loaderFileTypeBam.cxx
// Created by:  jason (21Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "loaderFileTypeBam.h"
#include "config_pgraph.h"
#include "bamFile.h"

#include "dcast.h"

TypeHandle LoaderFileTypeBam::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeBam::
LoaderFileTypeBam() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeBam::
get_name() const {
  return "Bam";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeBam::
get_extension() const {
  return "bam";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeBam::
load_file(const Filename &path, bool report_errors) const {
  BamFile bam_file;
  if (!bam_file.open_read(path, report_errors)) {
    return NULL;
  }

  return bam_file.read_node(report_errors);
}

