// Filename: config_loader.cxx
// Created by:  drose (19Mar00)
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

#include "config_loader.h"
#include "loaderFileType.h"
#include "loaderFileTypeBam.h"
#include "loaderFileTypeRegistry.h"

#include <dconfig.h>
#include <get_config_path.h>

Configure(config_loader);
NotifyCategoryDef(loader, "");

// Set this true to support actual asynchronous loads via the
// request_load()/fetch_load() interface to Loader.  Set it false to
// map these to blocking, synchronous loads instead.  Currently, the
// rest of Panda isn't quite ready for asynchronous loads, so leave
// this false for now.
const bool asynchronous_loads = config_loader.GetBool("asynchronous-loads", false);

Config::ConfigTable::Symbol *load_file_type = (Config::ConfigTable::Symbol *)NULL;

ConfigureFn(config_loader) {
  load_file_type = new Config::ConfigTable::Symbol;
  config_loader.GetAll("load-file-type", *load_file_type);

  LoaderFileType::init_type();
  LoaderFileTypeBam::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();

  reg->register_type(new LoaderFileTypeBam);
}

const DSearchPath &
get_bam_path() {
  static DSearchPath *bam_path = NULL;
  return get_config_path("bam-path", bam_path);
}
