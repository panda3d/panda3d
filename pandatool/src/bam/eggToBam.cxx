/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToBam.cxx
 * @author drose
 * @date 2000-06-28
 */

#include "eggToBam.h"

#include "config_putil.h"
#include "bamFile.h"
#include "load_egg_file.h"
#include "config_egg2pg.h"
#include "config_gobj.h"
#include "config_chan.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "dcast.h"
#include "graphicsPipeSelection.h"
#include "graphicsEngine.h"
#include "graphicsBuffer.h"
#include "graphicsStateGuardian.h"
#include "load_prc_file.h"
#include "windowProperties.h"
#include "frameBufferProperties.h"

/**
 *
 */
EggToBam::
EggToBam() :
  EggToSomething("Bam", ".bam", true, false)
{
  set_program_brief("convert .egg files to .bam files");
  set_program_description
    ("This program reads Egg files and outputs Bam files, the binary format "
     "suitable for direct loading of animation and models into Panda.  Bam "
     "files are tied to a particular version of Panda, so should not be "
     "considered replacements for egg files, but they tend to be smaller and "
     "load much faster than the equivalent egg files.");

  // -f is always in effect for egg2bam.  It doesn't make sense to provide it
  // as an option to the user.
  remove_option("f");

  add_path_replace_options();
  add_path_store_options();

  add_option
    ("flatten", "flag", 0,
     "Specifies whether to flatten the egg hierarchy after it is loaded.  "
     "If flag is zero, the egg hierarchy will not be flattened, but will "
     "instead be written to the bam file exactly as it is.  If flag is "
     "non-zero, the hierarchy will be flattened so that unnecessary nodes "
     "(usually group nodes with only one child) are eliminated.  The default "
     "if this is not specified is taken from the egg-flatten Config.prc "
     "variable.",
     &EggToBam::dispatch_int, &_has_egg_flatten, &_egg_flatten);

  add_option
    ("combine-geoms", "flag", 0,
     "Specifies whether to combine sibling GeomNodes into a common GeomNode "
     "when possible.  This flag is only respected if flatten, above, is also "
     "enabled (or implicitly true from the Config.prc file).  The default if "
     "this is not specified is taken from the egg-combine-geoms Config.prc "
     "variable.",
     &EggToBam::dispatch_int, &_has_egg_combine_geoms, &_egg_combine_geoms);

  add_option
    ("suppress-hidden", "flag", 0,
     "Specifies whether to suppress hidden geometry.  If this is nonzero, "
     "egg geometry tagged as \"hidden\" will be removed from the final "
     "scene graph; otherwise, it will be preserved (but stashed).  The "
     "default is nonzero, to remove it.",
     &EggToBam::dispatch_int, nullptr, &_egg_suppress_hidden);

  add_option
    ("ls", "", 0,
     "Writes a scene graph listing to standard output after the egg "
     "file has been loaded, showing the nodes that will be written out.",
     &EggToBam::dispatch_none, &_ls);

  add_option
    ("C", "quality", 0,
     "Specify the quality level for lossy channel compression.  If this "
     "is specified, the animation channels will be compressed at this "
     "quality level, which is normally an integer value between 0 and 100, "
     "inclusive, where higher numbers produce larger files with greater "
     "quality.  Generally, 95 is the highest useful quality level.  Use "
     "-NC (described below) to disable channel compression.  If neither "
     "option is specified, the default comes from the Config.prc file.",
     &EggToBam::dispatch_int, &_has_compression_quality, &_compression_quality);

  add_option
    ("NC", "", 0,
     "Turn off lossy compression of animation channels.  Channels will be "
     "written exactly as they are, losslessly.",
     &EggToBam::dispatch_none, &_compression_off);

  add_option
    ("rawtex", "", 0,
     "Record texture data directly in the bam file, instead of storing "
     "a reference to the texture elsewhere on disk.  The textures are "
     "stored uncompressed, unless -ctex is also specified.  "
     "A particular texture that is encoded into "
     "multiple different bam files in this way cannot be unified into "
     "the same part of texture memory if the different bam files are loaded "
     "together.  That being said, this can sometimes be a convenient "
     "way to ensure the bam file is completely self-contained.",
     &EggToBam::dispatch_none, &_tex_rawdata);

  add_option
    ("txo", "", 0,
     "Rather than writing texture data directly into the bam file, as in "
     "-rawtex, create a texture object for each referenced texture.  A "
     "texture object is a kind of mini-bam file, with a .txo extension, "
     "that contains all of the data needed to recreate a texture, including "
     "its image contents, filter and wrap settings, and so on.  3-D textures "
     "and cube maps can also be represented in a single .txo file.  Texture "
     "object files, like bam files, are tied to a particular version of "
     "Panda.",
     &EggToBam::dispatch_none, &_tex_txo);

#ifdef HAVE_ZLIB
  add_option
    ("txopz", "", 0,
     "In addition to writing texture object files as above, compress each "
     "one using pzip to a .txo.pz file.  In many cases, this will yield a "
     "disk file size comparable to that achieved by png compression.  This "
     "is an on-disk compression only, and does not affect the amount of "
     "RAM or texture memory consumed by the texture when it is loaded.",
     &EggToBam::dispatch_none, &_tex_txopz);
#endif  // HAVE_ZLIB

  add_option
    ("ctex", "", 0,
#ifdef HAVE_SQUISH
     "Pre-compress the texture images using the libsquish library, when "
     "using -rawtex or -txo.  "
#else
     "Asks the graphics card to pre-compress the texture images when using "
     "-rawtex or -txo.  "
#endif  // HAVE_SQUISH
#ifdef HAVE_ZLIB
     "This is unrelated to the on-disk compression achieved "
     "via -txopz (and it may be used in conjunction with that parameter).  "
#endif  // HAVE_ZLIB
     "This will result in a smaller RAM and texture memory footprint for "
     "the texture images.  The same "
     "effect can be achieved at load time by setting compressed-textures in "
     "your Config.prc file; but -ctex pre-compresses the "
     "textures so that they do not need to be compressed at load time.  "
#ifndef HAVE_SQUISH
     "Note that, since your Panda is not compiled with the libsquish "
     "library, using -ctex will make .txo files that are only guaranteed "
     "to load on the particular graphics card that was used to "
     "generate them."
#endif  // HAVE_SQUISH
     ,
     &EggToBam::dispatch_none, &_tex_ctex);

  add_option
    ("mipmap", "", 0,
     "Records the pre-generated mipmap levels in the texture object file "
     "when using -rawtex or -txo, regardless of the texture filter mode.  This "
     "will increase the size of the texture object file by about 33%, but "
     "it prevents the need to compute the mipmaps at runtime.  The default "
     "is to record mipmap levels only when the texture uses a mipmap "
     "filter mode.",
     &EggToBam::dispatch_none, &_tex_mipmap);

  add_option
    ("ctexq", "quality", 0,
     "Specifies the compression quality to use when performing the "
     "texture compression requested by -ctex.  This may be one of "
     "'default', 'fastest', 'normal', or 'best'.  The default is 'best'.  "
     "Set it to 'default' to use whatever is specified by the Config.prc "
     "file.  This is a global setting only; individual texture quality "
     "settings appearing within the egg file will override this.",
     &EggToBam::dispatch_string, nullptr, &_ctex_quality);

  add_option
    ("load-display", "display name", 0,
     "Specifies the particular display module to load to perform the texture "
     "compression requested by -ctex.  If this is omitted, the default is "
     "taken from the Config.prc file."
#ifdef HAVE_SQUISH
     "  Since your Panda has libsquish compiled in, this is not necessary; "
     "Panda can compress textures without loading a display module."
#endif  // HAVE_SQUISH
     ,
     &EggToBam::dispatch_string, nullptr, &_load_display);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting " + _format_name +
     " file.  This may be "
     "one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is z-up.");

  _force_complete = true;
  _egg_flatten = 0;
  _egg_combine_geoms = 0;
  _egg_suppress_hidden = 1;
  _tex_txopz = false;
  _ctex_quality = "best";
}

/**
 *
 */
void EggToBam::
run() {
  if (_has_egg_flatten) {
    // If the user specified some -flatten, we need to set the corresponding
    // Config.prc variable.
    egg_flatten = (_egg_flatten != 0);
  }
  if (_has_egg_combine_geoms) {
    // Ditto with -combine_geoms.
    egg_combine_geoms = (_egg_combine_geoms != 0);
  }

  // We always set egg_suppress_hidden.
  egg_suppress_hidden = _egg_suppress_hidden;

  if (_compression_off) {
    // If the user specified -NC, turn off channel compression.
    compress_channels = false;

  } else if (_has_compression_quality) {
    // Otherwise, if the user specified a compression quality with -C, use
    // that quality level.
    compress_channels = true;
    compress_chan_quality = _compression_quality;
  }

  if (_ctex_quality != "default") {
    // Override the user's config file with the command-line parameter for
    // texture compression.
    std::string prc = "texture-quality-level " + _ctex_quality;
    load_prc_file_data("prc", prc);
  }

  if (!_got_coordinate_system) {
    // If the user didn't specify otherwise, ensure the coordinate system is
    // Z-up.
    _data->set_coordinate_system(CS_zup_right);
  }

  PT(PandaNode) root = load_egg_data(_data);
  if (root == nullptr) {
    nout << "Unable to build scene graph from egg file.\n";
    exit(1);
  }

  if (_tex_ctex) {
#ifndef HAVE_SQUISH
    if (!make_buffer()) {
      nout << "Unable to initialize graphics context; cannot compress textures.\n";
      exit(1);
    }
#endif  // HAVE_SQUISH
  }

  if (_tex_txo || _tex_txopz || (_tex_ctex && _tex_rawdata)) {
    collect_textures(root);
    Textures::iterator ti;
    for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
      Texture *tex = (*ti);
      tex->get_ram_image();
      bool want_mipmaps = (_tex_mipmap || tex->uses_mipmaps());
      if (want_mipmaps) {
  // Generate mipmap levels.
  tex->generate_ram_mipmap_images();
      }

      if (_tex_ctex) {
#ifdef HAVE_SQUISH
        if (!tex->compress_ram_image()) {
          nout << "  couldn't compress " << tex->get_name() << "\n";
        }
        tex->set_compression(Texture::CM_on);
#else  // HAVE_SQUISH
        tex->set_keep_ram_image(true);
  bool has_mipmap_levels = (tex->get_num_ram_mipmap_images() > 1);
        if (!_engine->extract_texture_data(tex, _gsg)) {
          nout << "  couldn't compress " << tex->get_name() << "\n";
        }
  if (!has_mipmap_levels && !want_mipmaps) {
    // Make sure we didn't accidentally introduce mipmap levels by
    // rendezvousing through the graphics card.
    tex->clear_ram_mipmap_images();
  }
        tex->set_keep_ram_image(false);
#endif  // HAVE_SQUISH
      }

      if (_tex_txo || _tex_txopz) {
        convert_txo(tex);
      }
    }
  }

  if (_ls) {
    root->ls(nout, 0);
  }

  // This should be guaranteed because we pass false to the constructor,
  // above.
  nassertv(has_output_filename());

  Filename filename = get_output_filename();
  filename.make_dir();
  nout << "Writing " << filename << "\n";
  BamFile bam_file;
  if (!bam_file.open_write(filename)) {
    nout << "Error in writing.\n";
    exit(1);
  }

  if (!bam_file.write_object(root)) {
    nout << "Error in writing.\n";
    exit(1);
  }
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool EggToBam::
handle_args(ProgramBase::Args &args) {
  // If the user specified a path store option, we need to set the bam-
  // texture-mode Config.prc variable directly to support this (otherwise the
  // bam code will do what it wants to do anyway).
  if (_tex_rawdata) {
    bam_texture_mode = BamFile::BTM_rawdata;

  } else if (_got_path_store) {
    bam_texture_mode = BamFile::BTM_unchanged;

  } else {
    // Otherwise, the default path store is absolute; then the bam-texture-
    // mode can do the appropriate thing to it.
    _path_replace->_path_store = PS_absolute;
  }

  return EggToSomething::handle_args(args);
}

/**
 * Recursively walks the scene graph, looking for Texture references.
 */
void EggToBam::
collect_textures(PandaNode *node) {
  collect_textures(node->get_state());
  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    int num_geoms = geom_node->get_num_geoms();
    for (int i = 0; i < num_geoms; ++i) {
      collect_textures(geom_node->get_geom_state(i));
    }
  }

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    collect_textures(children.get_child(i));
  }
}

/**
 * Recursively walks the scene graph, looking for Texture references.
 */
void EggToBam::
collect_textures(const RenderState *state) {
  const TextureAttrib *tex_attrib = DCAST(TextureAttrib, state->get_attrib(TextureAttrib::get_class_type()));
  if (tex_attrib != nullptr) {
    int num_on_stages = tex_attrib->get_num_on_stages();
    for (int i = 0; i < num_on_stages; ++i) {
      _textures.insert(tex_attrib->get_on_texture(tex_attrib->get_on_stage(i)));
    }
  }
}

/**
 * If the indicated Texture was not already loaded from a txo file, writes it
 * to a txo file and updates the Texture object to reference the new file.
 */
void EggToBam::
convert_txo(Texture *tex) {
  if (!tex->get_loaded_from_txo()) {
    Filename fullpath = tex->get_fullpath().get_filename_index(0);
    if (_tex_txopz) {
      fullpath.set_extension("txo.pz");
      // We use this clumsy syntax so that the new extension appears to be two
      // separate extensions, .txo followed by .pz, which is what
      // Texture::write() expects to find.
      fullpath = Filename(fullpath.get_fullpath());
    } else {
      fullpath.set_extension("txo");
    }

    if (tex->write(fullpath)) {
      nout << "  Writing " << fullpath;
      if (tex->get_ram_image_compression() != Texture::CM_off) {
        nout << " (compressed " << tex->get_ram_image_compression() << ")";
      }
      nout << "\n";
      tex->set_loaded_from_txo();
      tex->set_fullpath(fullpath);
      tex->clear_alpha_fullpath();

      Filename filename = tex->get_filename().get_filename_index(0);
      if (_tex_txopz) {
        filename.set_extension("txo.pz");
        filename = Filename(filename.get_fullpath());
      } else {
        filename.set_extension("txo");
      }

      tex->set_filename(filename);
      tex->clear_alpha_filename();
    }
  }
}

/**
 * Creates a GraphicsBuffer for communicating with the graphics card.
 */
bool EggToBam::
make_buffer() {
  if (!_load_display.empty()) {
    // Override the user's config file with the command-line parameter.
    std::string prc = "load-display " + _load_display;
    load_prc_file_data("prc", prc);
  }

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  _pipe = selection->make_default_pipe();
  if (_pipe == nullptr) {
    nout << "Unable to create graphics pipe.\n";
    return false;
  }

  _engine = new GraphicsEngine;

  FrameBufferProperties fbprops = FrameBufferProperties::get_default();

  // Some graphics drivers can only create single-buffered offscreen buffers.
  // So request that.
  fbprops.set_back_buffers(0);

  WindowProperties winprops;
  winprops.set_size(1, 1);
  winprops.set_origin(0, 0);
  winprops.set_undecorated(true);
  winprops.set_open(true);
  winprops.set_z_order(WindowProperties::Z_bottom);

  // We don't care how big the buffer is; we just need it to manifest the GSG.
  _buffer = _engine->make_output(_pipe, "buffer", 0,
                                 fbprops, winprops,
                                 GraphicsPipe::BF_fb_props_optional);
  _engine->open_windows();
  if (_buffer == nullptr || !_buffer->is_valid()) {
    nout << "Unable to create graphics window.\n";
    return false;
  }
  _gsg = _buffer->get_gsg();

  return true;
}


int main(int argc, char *argv[]) {
  EggToBam prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
