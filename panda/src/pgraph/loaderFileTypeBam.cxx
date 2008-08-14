// Filename: loaderFileTypeBam.cxx
// Created by:  jason (21Jun00)
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

#include "loaderFileTypeBam.h"
#include "config_pgraph.h"
#include "bamFile.h"
#include "bamCacheRecord.h"
#include "loaderOptions.h"

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
//     Function: LoaderFileTypeBam::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeBam::
supports_compressed() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeBam::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {
  if (record != (BamCacheRecord *)NULL) {
    record->add_dependent_file(path);
  }

  bool report_errors = (options.get_flags() & LoaderOptions::LF_report_errors) != 0;
  BamFile bam_file;
  if (!bam_file.open_read(path, report_errors)) {
    return NULL;
  }
  bam_file.get_reader()->set_loader_options(options);
  return bam_file.read_node(report_errors);
}

