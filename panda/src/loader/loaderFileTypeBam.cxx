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
#include "config_loader.h"
#include "bamFile.h"

#include "config_util.h"
#include "dcast.h"

TypeHandle LoaderFileTypeBam::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeBam::
LoaderFileTypeBam()
{
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeBam::
get_name() const
{
  return "Bam";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeBam::
get_extension() const
{
  return "bam";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::resolve_filename
//       Access: Public, Virtual
//  Description: Searches for the indicated filename on whatever paths
//               are appropriate to this file type, and updates it if
//               it is found.
////////////////////////////////////////////////////////////////////
void LoaderFileTypeBam::
resolve_filename(Filename &path) const {
  path.resolve_filename(get_bam_path());
  path.resolve_filename(get_model_path());
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeBam::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeBam::
qpload_file(const Filename &path, bool report_errors) const {
  BamFile bam_file;
  if (!bam_file.open_read(path, report_errors)) {
    return NULL;
  }

  PT(PandaNode) result;

  TypedWritable *object = bam_file.read_object();
  if (object == TypedWritable::Null) {
    if (report_errors) {
      loader_cat.error() << "Bam file " << path << " is empty.\n";
    }

  } else if (!object->is_of_type(PandaNode::get_class_type())) {
    if (report_errors) {
      loader_cat.error()
        << "Bam file " << path
        << " contains a " << object->get_type() << ", not a PandaNode.\n";
    }

  } else {
    result = DCAST(PandaNode, object);

    if (report_errors) {
      bam_file.read_object();
      if (!bam_file.is_eof()) {
        loader_cat.warning()
          << "Ignoring extra objects in " << path << "\n";
      }
    }
  }

  if (!bam_file.resolve()) {
    if (report_errors) {
      loader_cat.error()
        << "Unable to resolve Bam file.\n";
      result = (PandaNode *)NULL;
    }
  }

  return result;
}

