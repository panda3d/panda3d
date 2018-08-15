/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypePandatool.cxx
 * @author drose
 * @date 2001-04-26
 */

#include "loaderFileTypePandatool.h"
#include "config_ptloader.h"
#include "somethingToEggConverter.h"
#include "eggToSomethingConverter.h"
#include "config_putil.h"
#include "load_egg_file.h"
#include "save_egg_file.h"
#include "eggData.h"
#include "loaderOptions.h"
#include "bamCacheRecord.h"

TypeHandle LoaderFileTypePandatool::_type_handle;

/**
 *
 */
LoaderFileTypePandatool::
LoaderFileTypePandatool(SomethingToEggConverter *loader,
                        EggToSomethingConverter *saver) :
  _loader(loader), _saver(saver)
{
  if (_loader != nullptr) {
    _loader->set_merge_externals(true);
  }
}

/**
 *
 */
LoaderFileTypePandatool::
~LoaderFileTypePandatool() {
}

/**
 *
 */
std::string LoaderFileTypePandatool::
get_name() const {
  if (_loader != nullptr) {
    return _loader->get_name();
  }
  return _saver->get_name();
}

/**
 *
 */
std::string LoaderFileTypePandatool::
get_extension() const {
  if (_loader != nullptr) {
    return _loader->get_extension();
  }
  return _saver->get_extension();
}

/**
 * Returns a space-separated list of extension, in addition to the one
 * returned by get_extension(), that are recognized by this converter.
 */
std::string LoaderFileTypePandatool::
get_additional_extensions() const {
  if (_loader != nullptr) {
    return _loader->get_additional_extensions();
  }
  return _saver->get_additional_extensions();
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypePandatool::
supports_compressed() const {
  if (_loader != nullptr) {
    return _loader->supports_compressed();
  }
  return _saver->supports_compressed();
}

/**
 * Returns true if the file type can be used to load files, and load_file() is
 * supported.  Returns false if load_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypePandatool::
supports_load() const {
  return (_loader != nullptr);
}

/**
 * Returns true if the file type can be used to save files, and save_file() is
 * supported.  Returns false if save_file() is unimplemented and will always
 * fail.
 */
bool LoaderFileTypePandatool::
supports_save() const {
  return (_saver != nullptr);
}

/**
 * Searches for the indicated filename on whatever paths are appropriate to
 * this file type, and updates it if it is found.
 */
void LoaderFileTypePandatool::
resolve_filename(Filename &path) const {
  path.resolve_filename(get_model_path(), get_extension());
}

/**
 *
 */
PT(PandaNode) LoaderFileTypePandatool::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {
  if (_loader == nullptr) {
    return nullptr;
  }

  if (record != nullptr) {
    record->add_dependent_file(path);
  }

  PT(PandaNode) result;

  SomethingToEggConverter *loader = _loader->make_copy();

  DSearchPath file_path;
  file_path.append_directory(path.get_dirname());
  loader->get_path_replace()->_path = file_path;

  // Convert animation, if the converter supports it.
  switch (options.get_flags() & LoaderOptions::LF_convert_anim) {
  case LoaderOptions::LF_convert_anim:
    loader->set_animation_convert(AC_both);
    break;

  case LoaderOptions::LF_convert_skeleton:
    loader->set_animation_convert(AC_model);
    break;

  case LoaderOptions::LF_convert_channels:
    loader->set_animation_convert(AC_chan);
    break;

  default:
    break;
  }

  // Try to convert directly to PandaNode first, if the converter type
  // supports it.
  if (ptloader_load_node && loader->supports_convert_to_node(options)) {
    result = loader->convert_to_node(options, path);
    if (!result.is_null()) {
      return result;
    }
  }

  // If the converter type doesn't support the direct PandaNode conversion,
  // take the slower route through egg instead.
  PT(EggData) egg_data = new EggData;
  loader->set_egg_data(egg_data);

  if (loader->convert_file(path)) {
    DistanceUnit input_units = loader->get_input_units();
    if (input_units != DU_invalid && ptloader_units != DU_invalid &&
        input_units != ptloader_units) {
      // Convert the file to the units specified by the ptloader-units
      // Configrc variable.
      ptloader_cat.info()
        << "Converting from " << format_long_unit(input_units)
        << " to " << format_long_unit(ptloader_units) << "\n";
      double scale = convert_units(input_units, ptloader_units);
      egg_data->transform(LMatrix4d::scale_mat(scale));
    }

    if (!egg_data->has_primitives()) {
      egg_data->make_point_primitives();
    } else if (!egg_data->has_normals()) {
      egg_data->recompute_polygon_normals();
    }

    result = load_egg_data(egg_data);
  }
  delete loader;

  return result;
}

/**
 *
 */
bool LoaderFileTypePandatool::
save_file(const Filename &path, const LoaderOptions &options,
          PandaNode *node) const {
  if (_saver == nullptr) {
    return false;
  }

  PT(EggData) egg_data = new EggData;
  if (!save_egg_data(egg_data, node)) {
    return false;
  }

  EggToSomethingConverter *saver = _saver->make_copy();
  saver->set_egg_data(egg_data);

  bool result = saver->write_file(path);
  delete saver;
  return result;
}
