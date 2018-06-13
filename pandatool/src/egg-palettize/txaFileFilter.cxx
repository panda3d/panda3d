/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file txaFileFilter.cxx
 * @author drose
 * @date 2006-07-27
 */

#include "txaFileFilter.h"
#include "palettizer.h"
#include "txaFile.h"
#include "textureImage.h"
#include "sourceTextureImage.h"
#include "texturePool.h"
#include "dconfig.h"
#include "configVariableFilename.h"
#include "virtualFileSystem.h"
#include "config_putil.h"

NotifyCategoryDeclNoExport(txafile);
NotifyCategoryDef(txafile, "");

// A few lines to register this filter type with the TexturePool when the
// shared library is loaded.
Configure(config_txaFileFilter);
ConfigureFn(config_txaFileFilter) {
  TxaFileFilter::init_type();
  TexturePool *pool = TexturePool::get_global_ptr();
  pool->register_filter(new TxaFileFilter);
}

TypeHandle TxaFileFilter::_type_handle;
TxaFile *TxaFileFilter::_txa_file;
bool TxaFileFilter::_got_txa_file;

/**
 * This method is called after each texture has been loaded from disk, via the
 * TexturePool, for the first time.  By the time this method is called, the
 * Texture has already been fully read from disk.  This method should return
 * the Texture pointer that the TexturePool should actually return (usually it
 * is the same as the pointer supplied).
 */
PT(Texture) TxaFileFilter::
post_load(Texture *tex) {
  if (!_got_txa_file) {
    read_txa_file();
  }

  TextureImage tex_image;
  std::string name = tex->get_filename().get_basename_wo_extension();
  tex_image.set_name(name);

  SourceTextureImage *source = tex_image.get_source
    (tex->get_fullpath(), tex->get_alpha_fullpath(), 0);
  PNMImage pnm_image;
  tex->store(pnm_image);
  source->set_header(pnm_image);
  tex_image.set_source_image(pnm_image);

  tex_image.pre_txa_file();

  bool matched = _txa_file->match_texture(&tex_image);
  if (txafile_cat.is_debug()) {
    if (!matched) {
      txafile_cat.debug()
        << "Not matched: " << name << "\n";
    } else {
      txafile_cat.debug()
        << "Matched: " << name << "\n";
    }
  }

  tex_image.post_txa_file();

  PNMImage dest(tex_image.get_x_size(),
                tex_image.get_y_size(),
                tex_image.get_num_channels(),
                pnm_image.get_maxval());
  dest.quick_filter_from(pnm_image);

  tex->load(dest);

  // Create an EggTexture to pass back the requested alpha mode to the egg
  // loader, if the texture is now being loaded from an egg file.
  PT_EggTexture egg_tex = new EggTexture(tex->get_name(), tex->get_fullpath());
  const TextureProperties &props = tex_image.get_properties();

  egg_tex->set_alpha_mode(tex_image.get_alpha_mode());
  egg_tex->set_format(props._format);
  egg_tex->set_minfilter(props._minfilter);
  egg_tex->set_minfilter(props._magfilter);
  egg_tex->set_anisotropic_degree(props._anisotropic_degree);

  tex->set_aux_data("egg", egg_tex);

  return tex;
}

/**
 * Reads the textures.txa file named by the variable txa-file.  Called only
 * once, at startup.
 */
void TxaFileFilter::
read_txa_file() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // We need to create a global Palettizer object to hold some of the global
  // properties that may be specified in a txa file.
  if (pal == nullptr) {
    pal = new Palettizer;
  }

  _txa_file = new TxaFile;
  _got_txa_file = true;

  ConfigVariableFilename txa_file
    ("txa-file", Filename("textures.txa"),
     PRC_DESC("Specify the name of the txa file to load when the txafile texture filter"
              "is in effect."));

  Filename filename = txa_file;
  vfs->resolve_filename(filename, get_model_path());

  if (!vfs->exists(filename)) {
    txafile_cat.warning()
      << "Filename " << filename << " not found.\n";
  } else {
    filename.set_text();
    std::istream *ifile = vfs->open_read_file(filename, true);
    if (ifile == nullptr) {
      txafile_cat.warning()
        << "Filename " << filename << " cannot be read.\n";
    } else {
      if (!_txa_file->read(*ifile, filename)) {
        txafile_cat.warning()
          << "Syntax errors in " << filename << "\n";
      } else {
        txafile_cat.info()
          << "Read " << filename << "\n";
      }
      vfs->close_read_file(ifile);
    }
  }
}
