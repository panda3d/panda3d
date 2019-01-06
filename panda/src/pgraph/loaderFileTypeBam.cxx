/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeBam.cxx
 * @author jason
 * @date 2000-06-21
 */

#include "loaderFileTypeBam.h"
#include "config_pgraph.h"
#include "bamFile.h"
#include "bamCacheRecord.h"
#include "modelRoot.h"
#include "loaderOptions.h"

#include "dcast.h"

TypeHandle LoaderFileTypeBam::_type_handle;

/**
 *
 */
LoaderFileTypeBam::
LoaderFileTypeBam() {
}

/**
 *
 */
std::string LoaderFileTypeBam::
get_name() const {
  return "Bam";
}

/**
 *
 */
std::string LoaderFileTypeBam::
get_extension() const {
  return "bam";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypeBam::
supports_compressed() const {
  return true;
}

/**
 * Returns true if the file type can be used to load files, and load_file() is
 * supported.  Returns false if load_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypeBam::
supports_load() const {
  return true;
}

/**
 * Returns true if the file type can be used to save files, and save_file() is
 * supported.  Returns false if save_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypeBam::
supports_save() const {
  return true;
}

/**
 *
 */
PT(PandaNode) LoaderFileTypeBam::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {
  if (record != nullptr) {
    record->add_dependent_file(path);
  }

  bool report_errors = (options.get_flags() & LoaderOptions::LF_report_errors) != 0;

  BamFile bam_file;
  if (!bam_file.open_read(path, report_errors)) {
    return nullptr;
  }
  bam_file.get_reader()->set_loader_options(options);
  time_t timestamp = bam_file.get_reader()->get_source()->get_timestamp();

  PT(PandaNode) node = bam_file.read_node(report_errors);
  if (node != nullptr && node->is_of_type(ModelRoot::get_class_type())) {
    ModelRoot *model_root = DCAST(ModelRoot, node.p());
    model_root->set_fullpath(path);
    model_root->set_timestamp(timestamp);
  }

  return node;
}


/**
 *
 */
bool LoaderFileTypeBam::
save_file(const Filename &path, const LoaderOptions &options,
          PandaNode *node) const {
  BamFile bam_file;

  bool report_errors = (options.get_flags() & LoaderOptions::LF_report_errors) != 0;

  bool okflag = false;

  if (bam_file.open_write(path, report_errors)) {
    if (bam_file.write_object(node)) {
      okflag = true;
    }
    bam_file.close();
  }
  return okflag;
}
