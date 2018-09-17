/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMakeFont.cxx
 * @author drose
 * @date 2001-02-16
 */

#include "eggMakeFont.h"
#include "rangeIterator.h"
#include "palettizer.h"
#include "filenameUnifier.h"
#include "eggFile.h"
#include "textureImage.h"
#include "sourceTextureImage.h"
#include "pnmTextMaker.h"
#include "pnmTextGlyph.h"
#include "eggData.h"
#include "eggGroup.h"
#include "eggPoint.h"
#include "eggPolygon.h"
#include "eggTexture.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "string_utils.h"
#include "dcast.h"

#include <ctype.h>

using std::string;

/**
 *
 */
EggMakeFont::
EggMakeFont() : EggWriter(true, false) {
  set_program_brief("generates .egg files with rasterized font glyphs");
  set_program_description
    ("egg-mkfont uses the FreeType library to generate an egg file "
     "and a series of texture images from a font file "
     "input, such as a TTF file.  The resulting egg file "
     "can be loaded in Panda as a font for rendering text, even "
     "if FreeType is not compiled into the executing Panda.\n\n"

     "egg-mkfont will normally run the generated egg file through "
     "egg-palettize automatically as part of the generation process.  "
     "This collects the individual glyph textures into a small number "
     "of texture maps.  If you intend to run the font through egg-palettize "
     "yourself later, you may choose to omit this step.");

  clear_runlines();
  add_runline("[opts] -o output.egg font");
  add_runline("[opts] font output.egg");

  add_option
    ("fg", "r,g,b[,a]", 0,
     "Specifies the foreground color of the generated texture map.  The "
     "default is white: 1,1,1,1, which leads to the most flexibility "
     "as the color can be modulated at runtime to any suitable color.",
     &EggMakeFont::dispatch_color, nullptr, &_fg[0]);

  add_option
    ("bg", "r,g,b[,a]", 0,
     "Specifies the background color of the generated texture map.  The "
     "default is transparent: 1,1,1,0, which allows the text to be "
     "visible against any color background by placing a polygon of a "
     "suitable color behind it.  If the alpha component of either -fg "
     "or -bg is not 1, the generated texture images will include an "
     "alpha component; if both colors specify an alpha component of 1 "
     "(or do not specify an alpha compenent), then the generated images "
     "will not include an alpha component.",
     &EggMakeFont::dispatch_color, nullptr, &_bg[0]);

  add_option
    ("interior", "r,g,b[,a]", 0,
     "Specifies the color to render the interior part of a hollow font.  "
     "This is a special effect that involves analysis of the bitmap after "
     "the font has been rendered, and so is more effective when the pixel "
     "size is large.  It also implies -noaa (but you can use a scale "
     "factor with -sf to achieve antialiasing).",
     &EggMakeFont::dispatch_color, &_got_interior, &_interior[0]);

  add_option
    ("chars", "range", 0,
     "Specifies the characters of the font that are used.  The range "
     "specification may include combinations of decimal or hex unicode "
     "values (where hex values are identified with a leading 0x), separated "
     "by commas and hyphens to indicate ranges, e.g. '32-126,0xfa0-0xfff'.  "
     "It also may specify ranges of ASCII characters by enclosing them "
     "within square brackets, e.g. '[A-Za-z0-9]'.  If this is not specified, "
     "the default set has all ASCII characters and an assorted set of "
     "latin-1 characters, diacritics and punctuation marks.",
     &EggMakeFont::dispatch_range, nullptr, &_range);

  add_option
    ("extra", "file.egg", 0,
     "Specifies additional externally-painted glyphs to mix into the "
     "generated egg file.  The named egg file is expected to contain one "
     "or more groups, each of which is named with the decimal unicode "
     "number of a character and should contain one polygon.  These groups "
     "are simply copied into the output egg file as if they were generated "
     "locally.  This option may be repeated.",
     &EggMakeFont::dispatch_vector_string, nullptr, &_extra_filenames);

  add_option
    ("ppu", "pixels", 0,
     "Specify the pixels per unit.  This is the number of pixels in the "
     "generated texture map that are used for each onscreen unit (or each "
     "10 points of font; see -ps).  Setting this number larger results in "
     "an easier-to-read font, but at the cost of more texture memory.  "
     "The default is 40.",
     &EggMakeFont::dispatch_double, nullptr, &_pixels_per_unit);

  add_option
    ("ps", "size", 0,
     "Specify the point size of the resulting font.  This controls the "
     "apparent size of the font when it is rendered onscreen.  By convention, "
     "a 10 point font is 1 screen unit high, so the default is 10.",
     &EggMakeFont::dispatch_double, nullptr, &_point_size);

  add_option
    ("sdf", "", 0,
     "If this is set, a signed distance field will be generated, which "
     "results in crisp text even when the text is enlarged or zoomed in.",
     &EggMakeFont::dispatch_true, nullptr, &_generate_distance_field);

  add_option
    ("pm", "n", 0,
     "The number of extra pixels around a single character in the "
     "generated polygon.  This may be a floating-point number.  The "
     "default is 1.",
     &EggMakeFont::dispatch_double, nullptr, &_poly_margin);

  add_option
    ("tm", "n", 0,
     "The number of extra pixels around each character in the texture map.  "
     "This may only be an integer.  The default is 2.  This is meaningful "
     "when -nopal is also used; in the normal case, use -pm to control "
     "both the polygon size and the texture map spacing.",
     &EggMakeFont::dispatch_int, nullptr, &_tex_margin);

  add_option
    ("rm", "n", 0,
     "The amount of padding in screen units to place around the glyph when "
     "rendered.  This differs from -pm in that it has no effect on the "
     "generated texture map, only on the generated egg.  Use this in order to "
     "space the characters out in case they appear to be too close together "
     "when rendered. The default is 0.",
     &EggMakeFont::dispatch_double, nullptr, &_render_margin);

  add_option
    ("sf", "factor", 0,
     "The scale factor of the generated image.  This is the factor by which "
     "the font image is generated oversized, then reduced to its final size, "
     "to improve antialiasing.  If the specified font contains one "
     "or more fixed-size fonts instead of a scalable font, the scale factor "
     "may be automatically adjusted as necessary to scale the closest-"
     "matching font to the desired pixel size.  The default is 2.",
     &EggMakeFont::dispatch_double, &_got_scale_factor, &_scale_factor);

  add_option
    ("noaa", "", 0,
     "Disable low-level antialiasing by the Freetype library.  "
     "This is unrelated to the antialiasing that is applied due to the "
     "scale factor specified by -sf; you may have either one, neither, or "
     "both kinds of antialiasing enabled.",
     &EggMakeFont::dispatch_none, &_no_native_aa);

  add_option
    ("nopal", "", 0,
     "Don't run egg-palettize automatically on the output file, but "
     "just output the raw egg file and all of its individual texture "
     "images, one for each glyph.",
     &EggMakeFont::dispatch_none, &_no_palettize);

  add_option
    ("nr", "", 0,
     "Don't actually reduce the images after applying the scale factor, but "
     "leave them at their inflated sizes.  Presumably you will reduce "
     "them later, for instance with egg-palettize.",
     &EggMakeFont::dispatch_none, &_no_reduce);

  add_option
    ("gp", "pattern", 0,
     "The pattern to be used to generate the glyph texture images.  This "
     "string will be passed to sprintf to generate the actual file name; it "
     "should contain the string %d or %x (or some variant such as %03d) "
     "which will be filled in with the Unicode number of each symbol.  "
     "If it is omitted, the default is based on the name of the egg file.  "
     "This is used only if -nopal is specified; in the normal case, "
     "without -nopal, use -pp instead.",
     &EggMakeFont::dispatch_string, nullptr, &_output_glyph_pattern);

  add_option
    ("pp", "pattern", 0,
     "The pattern to be used to generate the palette texture images.  This "
     "string is effectively passed to egg-palettize as the -tn option, and "
     "thus should contain %i for the palette index number.  This is used "
     "if -nopal is not specified.",
     &EggMakeFont::dispatch_string, nullptr, &_output_palette_pattern);

  add_option
    ("palsize", "xsize,ysize", 0,
     "Specify the size of the palette texture images.  This is used if "
     "-nopal is not specified.",
     &EggMakeFont::dispatch_int_pair, nullptr, _palette_size);

  add_option
    ("face", "index", 0,
     "Specify the face index of the particular face within the font file "
     "to use.  Some font files contain multiple faces, indexed beginning "
     "at 0.  The default is face 0.",
     &EggMakeFont::dispatch_int, nullptr, &_face_index);

  _fg.set(1.0, 1.0, 1.0, 1.0);
  _bg.set(1.0, 1.0, 1.0, 0.0);
  _interior.set(1.0, 1.0, 1.0, 1.0);
  _pixels_per_unit = 40.0;
  _point_size = 10.0;
  _poly_margin = 1.0;
  _tex_margin = 2;
  _render_margin = 0.0;
  _palette_size[0] = _palette_size[1] = 512;
  _face_index = 0;
  _generate_distance_field = false;

  _text_maker = nullptr;
  _vpool = nullptr;
  _group = nullptr;
}


/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool EggMakeFont::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "Must specify name of font file on command line.\n";
    return false;
  }

  _input_font_filename = args[0];
  args.pop_front();
  return EggWriter::handle_args(args);
}

/**
 *
 */
void EggMakeFont::
run() {
  if (has_output_filename() && !get_output_filename().get_dirname().empty()) {
    FilenameUnifier::set_rel_dirname(get_output_filename().get_dirname());
  } else {
    FilenameUnifier::set_rel_dirname(".");
  }

  _text_maker = new PNMTextMaker(_input_font_filename, _face_index);
  if (!_text_maker->is_valid()) {
    exit(1);
  }

  if (_got_interior) {
    _no_native_aa = true;
  }

  if (!_got_scale_factor) {
    // The default scale factor is 4 if we are not using FreeType's antialias,
    // or 2 if we are.
    if (_generate_distance_field) {
      _scale_factor = 1.0;
    } else if (_no_native_aa) {
      _scale_factor = 4.0;
    } else {
      _scale_factor = 2.0;
    }
  }

  _text_maker->set_point_size(_point_size);
  _text_maker->set_native_antialias(!_no_native_aa);
  _text_maker->set_interior_flag(_got_interior);
  _text_maker->set_pixels_per_unit(_pixels_per_unit);
  _text_maker->set_scale_factor(_scale_factor);

  // The text_maker may have had to adjust the pixels per unit and the scale
  // factor according to what the font supports.
  _pixels_per_unit = _text_maker->get_pixels_per_unit();
  _scale_factor = _text_maker->get_scale_factor();

  if (_text_maker->get_font_pixel_size() != 0) {
    nout << "Using " << _text_maker->get_font_pixel_size() << "-pixel font.\n";
  }

  // Now we may want to tweak the scale factor so that fonts will actually be
  // generated big.  We have to do this after we have already send the current
  // _scale_factor through the _text_maker for validation.
  _palettize_scale_factor = _scale_factor;
  if (_scale_factor != 1.0 && (_no_reduce || !_no_palettize)) {
    // If _no_reduce is true (-nr was specified), we want to keep the glyph
    // textures full-sized, because the user asked for that.

    // If _no_palettize is false (-nopal was not specified), we still want to
    // keep the glyph textures full-sized, because the palettizer will reduce
    // them later.

    _tex_margin = (int)(_tex_margin * _scale_factor);
    _poly_margin *= _scale_factor;
    _pixels_per_unit *= _scale_factor;
    _scale_factor = 1.0;
    _text_maker->set_pixels_per_unit(_pixels_per_unit);
    _text_maker->set_scale_factor(1.0);
  }

  if (_no_reduce) {
    // If -nr was specified, but we're still palettizing, we don't even want
    // to reduce the palette images.  Instead, we'll generate extra-large
    // palette images.
    _palette_size[0] = (int)(_palette_size[0] * _palettize_scale_factor);
    _palette_size[1] = (int)(_palette_size[1] * _palettize_scale_factor);
    _palettize_scale_factor = 1.0;
  }

  if (_range.is_empty()) {
    // If there's no specified range, the default is the entire ASCII set.
    _range.add_range(0x20, 0x7e);

    _range.add_singleton(0xa1); // Upside down exclamation mark
    _range.add_singleton(0xa9); // Copyright sign
    _range.add_singleton(0xab); // Left double angle quote
    // _range.add_singleton(0xae);  Registered sign
    _range.add_singleton(0xb0); // Degree symbol
    _range.add_singleton(0xb5); // Mu/micro
    _range.add_singleton(0xb8); // Cedilla
    _range.add_singleton(0xbb); // Right double angle quote
    _range.add_singleton(0xbf); // Upside down question mark

    _range.add_singleton(0xc6); // AE ligature
    _range.add_singleton(0xc7); // C cedilla
    // _range.add_singleton(0xd0);  Upper-case Eth _range.add_singleton(0xd8);
    // Upper-case O with line _range.add_singleton(0xde);  Upper-case Thorn
    _range.add_singleton(0xdf); // German Eszet
    _range.add_singleton(0xe6); // ae ligature
    _range.add_singleton(0xe7); // c cedilla
    _range.add_singleton(0xf0); // Lower-case Eth
    _range.add_singleton(0xf8); // Lower-case O with line
    _range.add_singleton(0xfe); // Lower-case Thorn

    // _range.add_singleton(0x03c0);  pi

    // Dotless i and j, for combining purposes.
    _range.add_singleton(0x0131);
    _range.add_singleton(0x0237);

    // And general punctuation.  These don't take up much space anyway.
    _range.add_range(0x2018, 0x201f);

    _range.add_singleton(0x2026); // Ellipses

    // Also add all the combining diacritic marks.
    _range.add_range(0x0300, 0x030f);
  }
  if (_output_glyph_pattern.empty()) {
    // Create a default texture filename pattern.
    _output_glyph_pattern = get_output_filename().get_fullpath_wo_extension() + "%03d.png";
  }
  if (_output_palette_pattern.empty()) {
    // Create a default texture filename pattern.
    _output_palette_pattern = get_output_filename().get_fullpath_wo_extension() + "_%i";
  }

  // Figure out how many channels we need based on the foreground and
  // background colors.
  bool needs_alpha = (_fg[3] != 1.0 || _bg[3] != 1.0 || _interior[3] != 1.0);
  bool needs_color = (_fg[0] != _fg[1] || _fg[1] != _fg[2] ||
                      _bg[0] != _bg[1] || _bg[1] != _bg[2] ||
                      _interior[0] != _interior[1] || _interior[1] != _interior[2]);

  if (needs_alpha) {
    if (needs_color) {
      _num_channels = 4;
      _format = EggTexture::F_rgba;
    } else {
      if (_fg[0] == 1.0 && _bg[0] == 1.0 && _interior[0] == 1.0) {
        // A special case: we only need an alpha channel.  Copy the alpha data
        // into the color channels so we can write out a one-channel image.
        _fg[0] = _fg[1] = _fg[2] = _fg[3];
        _bg[0] = _bg[1] = _bg[2] = _bg[3];
        _interior[0] = _interior[1] = _interior[2] = _interior[3];
        _num_channels = 1;
        _format = EggTexture::F_alpha;
      } else {
        _num_channels = 2;
        _format = EggTexture::F_luminance_alpha;
      }
    }
  } else {
    if (needs_color) {
      _num_channels = 3;
      _format = EggTexture::F_rgb;
    } else {
      _num_channels = 1;
      _format = EggTexture::F_luminance;
    }
  }

  // Create a global Palettizer object.  We'll use this even if the user
  // specified -nopal, if nothing else just to hold all of the TextureImage
  // pointers.
  pal = new Palettizer;
  pal->_generated_image_pattern = _output_palette_pattern;
  pal->_omit_solitary = false;
  pal->_round_uvs = false;

  // Generate a txa script for the palettizer.  We have the palettizer reduce
  // all of the texture images by the inverse of our scale factor.
  char buffer[1024];
  sprintf(buffer, ":margin 0;:coverage 1000;:background %f %f %f %f;:palette %d %d;*: %f%% keep-format",
          _bg[0], _bg[1], _bg[2], _bg[3],
          _palette_size[0], _palette_size[1],
          100.0 / _palettize_scale_factor);
  std::istringstream txa_script(buffer);
  pal->read_txa_file(txa_script, "default script");

  pal->all_params_set();

  // Now create all the egg structures.  We can't use _data, since we want to
  // pass this object to the palettizer, which will try to up its reference
  // count.
  PT(EggData) egg_data = new EggData;
  _group = new EggGroup();
  egg_data->add_child(_group);
  append_command_comment(egg_data);

  _vpool = new EggVertexPool("vpool");
  _group->add_child(_vpool);

  // Make the group a sequence, as a convenience.  If we view the egg file
  // directly we can see all the characters one at a time.
  _group->set_switch_flag(true);
  _group->set_switch_fps(2.0);

  double margin = _poly_margin;
  if (_generate_distance_field) {
    // Distance fields are always rendered with binary alpha.
    _group->set_alpha_mode(EggRenderMode::AM_binary);

    // Fudged to make most fonts fit on 512x256.
    if (_poly_margin >= 1) {
      margin += 3.5;
      _poly_margin -= 0.5;
    }

    _text_maker->set_distance_field_radius(4);
  }

  // Also create an egg group indicating the font's design size and poly
  // margin.
  EggGroup *ds_group = new EggGroup("ds");
  _group->add_child(ds_group);
  EggVertex *vtx = make_vertex(LPoint2d(margin / _pixels_per_unit, _text_maker->get_line_height()));
  EggPoint *point = new EggPoint;
  ds_group->add_child(point);
  point->add_vertex(vtx);

  // Finally, add the characters, one at a time.
  RangeIterator ri(_range);
  do {
    add_character(ri.get_code());
  } while (ri.next());

  // If there are extra glyphs, pick them up.
  if (!_extra_filenames.empty()) {
    vector_string::const_iterator si;
    for (si = _extra_filenames.begin(); si != _extra_filenames.end(); ++si) {
      add_extra_glyphs(*si);
    }
  }

  if (_no_palettize) {
    // Ok, no palettize step; just write out the egg file and all of the
    // textures.
    Textures::iterator ti;
    for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
      TextureImage *texture = (*ti);
      texture->write(texture->read_source_image());
    }

    egg_data->write_egg(get_output());

  } else {
    // Pass the generated egg structure through egg-palettize, without writing
    // it to disk first.
    string name = get_output_filename().get_basename();
    EggFile *egg_file = pal->get_egg_file(name);
    egg_file->from_command_line(egg_data, "", get_output_filename(),
                                get_exec_command());

    pal->add_command_line_egg(egg_file);
    pal->process_all(true, "");
    pal->optimal_resize();
    pal->generate_images(true);
    if (!pal->write_eggs()) {
      exit(1);
    }
    // pal->report_pi();
  }
}

/**
 *
 */
bool EggMakeFont::
dispatch_range(const string &, const string &arg, void *var) {
  RangeDescription *ip = (RangeDescription *)var;
  return ip->parse_parameter(arg);
}

/**
 * Allocates and returns a new vertex from the vertex pool representing the
 * indicated 2-d coordinates.
 */
EggVertex *EggMakeFont::
make_vertex(const LPoint2d &xy) {
  return
    _vpool->make_new_vertex(LPoint3d::origin(_coordinate_system) +
                            LVector3d::rfu(xy[0], 0.0, xy[1], _coordinate_system));
}

/**
 * Generates the indicated character and adds it to the font description.
 */
void EggMakeFont::
add_character(int code) {
  PNMTextGlyph *glyph = _text_maker->get_glyph(code);
  if (glyph == nullptr) {
    nout << "No definition in font for character " << code << ".\n";
    return;
  }

  make_geom(glyph, code);
}


/**
 * Creates the actual geometry for the glyph.
 */
void EggMakeFont::
make_geom(PNMTextGlyph *glyph, int character) {
  // Create an egg group to hold the polygon.
  string group_name = format_string(character);
  EggGroup *group = new EggGroup(group_name);
  _group->add_child(group);

  if (glyph->get_width() != 0 && glyph->get_height() != 0) {
    int bitmap_top = glyph->get_top();
    int bitmap_left = glyph->get_left();
    double tex_x_size = glyph->get_width();
    double tex_y_size = glyph->get_height();

    double poly_margin = _poly_margin;
    double x_origin = _tex_margin;
    double y_origin = _tex_margin;
    double page_y_size = tex_y_size + _tex_margin * 2;
    double page_x_size = tex_x_size + _tex_margin * 2;

    // Determine the corners of the rectangle in geometric units.
    double tex_poly_margin = poly_margin / _pixels_per_unit;
    double origin_y = bitmap_top / _pixels_per_unit;
    double origin_x = bitmap_left / _pixels_per_unit;
    double top = origin_y + tex_poly_margin;
    double left = origin_x - tex_poly_margin;
    double bottom = origin_y - tex_y_size / _pixels_per_unit - tex_poly_margin;
    double right = origin_x + tex_x_size / _pixels_per_unit + tex_poly_margin;

    // And the corresponding corners in UV units.
    double uv_top = 1.0f - (double)(y_origin - poly_margin) / page_y_size;
    double uv_left = (double)(x_origin - poly_margin) / page_x_size;
    double uv_bottom = 1.0f - (double)(y_origin + poly_margin + tex_y_size) / page_y_size;
    double uv_right = (double)(x_origin + poly_margin + tex_x_size) / page_x_size;

    // Create the vertices for the polygon.
    EggVertex *v1 = make_vertex(LPoint2d(left, bottom));
    EggVertex *v2 = make_vertex(LPoint2d(right, bottom));
    EggVertex *v3 = make_vertex(LPoint2d(right, top));
    EggVertex *v4 = make_vertex(LPoint2d(left, top));

    v1->set_uv(LTexCoordd(uv_left, uv_bottom));
    v2->set_uv(LTexCoordd(uv_right, uv_bottom));
    v3->set_uv(LTexCoordd(uv_right, uv_top));
    v4->set_uv(LTexCoordd(uv_left, uv_top));

    EggPolygon *poly = new EggPolygon();
    group->add_child(poly);
    poly->set_texture(get_tref(glyph, character));

    poly->add_vertex(v1);
    poly->add_vertex(v2);
    poly->add_vertex(v3);
    poly->add_vertex(v4);
  }

  // Now create a single point where the origin of the next character will be.

  EggVertex *v0 = make_vertex(LPoint2d(glyph->get_advance() / _pixels_per_unit + _render_margin, 0.0));
  EggPoint *point = new EggPoint;
  group->add_child(point);
  point->add_vertex(v0);
}

/**
 * Returns the egg texture reference for a particular glyph, creating it if it
 * has not already been created.
 */
EggTexture *EggMakeFont::
get_tref(PNMTextGlyph *glyph, int character) {
  TRefs::iterator ti = _trefs.find(glyph);
  if (ti != _trefs.end()) {
    return (*ti).second;
  }

  EggTexture *tref = make_tref(glyph, character);
  _trefs[glyph] = tref;
  return tref;
}

/**
 * Generates a texture image for the indicated glyph, and returns its egg
 * reference.
 */
EggTexture *EggMakeFont::
make_tref(PNMTextGlyph *glyph, int character) {
  char buffer[1024];
  sprintf(buffer, _output_glyph_pattern.c_str(), character);

  Filename texture_filename = buffer;
  PNMImage image(glyph->get_width() + _tex_margin * 2,
                 glyph->get_height() + _tex_margin * 2, _num_channels);
  image.fill(_bg[0], _bg[1], _bg[2]);
  if (image.has_alpha()) {
    image.alpha_fill(_bg[3]);
  }
  if (_got_interior) {
    glyph->place(image, -glyph->get_left() + _tex_margin,
                 glyph->get_top() + _tex_margin, _fg, _interior);
  } else {
    glyph->place(image, -glyph->get_left() + _tex_margin,
                 glyph->get_top() + _tex_margin, _fg);
  }

  // We don't write the image to disk immediately, since it might just get
  // palettized.  But we do record it in a TextureImage object within the
  // global Palettizer, so that it may be written out later.

  string name = texture_filename.get_basename_wo_extension();
  TextureImage *texture = pal->get_texture(name);
  _textures.push_back(texture);
  texture->set_filename("", texture_filename);
  SourceTextureImage *source = texture->get_source(texture_filename, "", 0);
  texture->set_source_image(image);
  source->set_header(image);

  EggTexture *tref = new EggTexture(name, texture_filename);
  tref->set_format(_format);
  tref->set_wrap_mode(EggTexture::WM_clamp);
  tref->set_minfilter(EggTexture::FT_linear_mipmap_linear);
  tref->set_magfilter(EggTexture::FT_linear);
  tref->set_quality_level(EggTexture::QL_best);

  return tref;
}

/**
 * Reads the indicated filename and adds any numbered groups into the current
 * egg file.
 */
void EggMakeFont::
add_extra_glyphs(const Filename &extra_filename) {
  PT(EggData) extra_data = new EggData;

  if (!extra_data->read(extra_filename)) {
    return;
  }

  _group->steal_children(*extra_data);
}

/**
 * Recursively searches for numbered groups in the indicated egg file, and
 * copies them to the current egg file.
 */
void EggMakeFont::
r_add_extra_glyphs(EggGroupNode *egg_group) {
  if (egg_group->is_of_type(EggGroup::get_class_type())) {
    EggGroup *group = DCAST(EggGroup, egg_group);
    if (is_numeric(group->get_name())) {
      EggGroup *new_group = new EggGroup(group->get_name());
      _group->add_child(new_group);
      new_group->steal_children(*group);
      return;
    }
  }

  EggGroupNode::iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    EggNode *child = (*ci);
    if (child->is_of_type(EggGroupNode::get_class_type())) {
      r_add_extra_glyphs(DCAST(EggGroupNode, child));
    }
  }
}

/**
 * Returns true if the indicated string is all numeric digits, false
 * otherwise.
 */
bool EggMakeFont::
is_numeric(const string &str) {
  if (str.empty()) {
    return false;
  }

  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    if (!isdigit(*si)) {
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  EggMakeFont prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
