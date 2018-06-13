/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file staticTextFont.cxx
 * @author drose
 * @date 2001-05-03
 */

#include "staticTextFont.h"
#include "config_text.h"

#include "geom.h"
#include "geomNode.h"
#include "geomVertexReader.h"
#include "geomPoints.h"
#include "renderState.h"
#include "dcast.h"

// Temporary
#include "textureCollection.h"
#include "nodePath.h"

TypeHandle StaticTextFont::_type_handle;

/**
 * The constructor expects the root node to a model generated via egg-mkfont,
 * which consists of a set of models, one per each character in the font.
 *
 * If a CoordinateSystem value is specified, it informs the font of the
 * coordinate system in which this model was generated.  "up" in this
 * coordinate system will be the direction of the top of the letters.
 */
StaticTextFont::
StaticTextFont(PandaNode *font_def, CoordinateSystem cs) {
  nassertv(font_def != nullptr);
  _font = font_def;
  _cs = cs;
  _glyphs.clear();

  if (_cs == CS_default) {
    _cs = get_default_coordinate_system();
  }

  NodePath np(font_def);
  if (_cs != CS_zup_right) {
    // We have to convert the entire font to CS_zup_right before we can use
    // it, because the text subsystem assumes the glyphs are stored in
    // CS_zup_right.
    NodePath temp_root("root");
    NodePath temp_child = temp_root.attach_new_node("child");
    np = np.copy_to(temp_child);
    LMatrix4 mat = LMatrix4::convert_mat(_cs, CS_zup_right);
    temp_child.set_mat(mat);
    temp_root.clear_model_nodes();
    temp_root.flatten_light();
    np = temp_root.get_child(0).get_child(0);
    _font = np.node();
    _cs = CS_zup_right;
  }

  // If there is no explicit quality level or filter settings on the textures
  // in the static font, set the appropriate defaults for text.
  TextureCollection tc = np.find_all_textures();
  int num_textures = tc.get_num_textures();
  for (int i = 0; i < num_textures; ++i) {
    Texture *tex = tc.get_texture(i);

    // Don't compress font textures.  Though there's a relatively high bang-
    // for-the-buck in compressing them, there's an increased risk that broken
    // graphics drivers will fail to render the text properly, causing
    // troubles for a user who then won't be able to navigate the options
    // menus to disable texture compression.
    tex->set_compression(Texture::CM_off);

    if (tex->get_quality_level() == Texture::QL_default) {
      tex->set_quality_level(text_quality_level);
    }
    if (tex->get_minfilter() == SamplerState::FT_default) {
      tex->set_minfilter(text_minfilter);
    }
    if (tex->get_magfilter() == SamplerState::FT_default) {
      tex->set_magfilter(text_magfilter);
    }
  }

  find_characters(_font, RenderState::make_empty());
  _is_valid = !_glyphs.empty();

  // Check for an explicit space width.
  int character = 32;
  Glyphs::iterator gi = _glyphs.find(character);
  if (gi != _glyphs.end()) {
    TextGlyph *glyph = (*gi).second;
    _space_advance = glyph->get_advance();
  }

  set_name(font_def->get_name());
}

/**
 * Returns a new copy of the same font.
 */
PT(TextFont) StaticTextFont::
make_copy() const {
  return new StaticTextFont(_font);
}

/**
 *
 */
void StaticTextFont::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "StaticTextFont " << get_name() << "; "
    << _glyphs.size() << " characters available in font:\n";
  Glyphs::const_iterator gi;

  // Figure out which symbols we have.  We collect lowercase letters,
  // uppercase letters, and digits together for the user's convenience.
  static const int num_letters = 26;
  static const int num_digits = 10;
  bool lowercase[num_letters];
  bool uppercase[num_letters];
  bool digits[num_digits];

  memset(lowercase, 0, sizeof(bool) * num_letters);
  memset(uppercase, 0, sizeof(bool) * num_letters);
  memset(digits, 0, sizeof(bool) * num_digits);

  int count_lowercase = 0;
  int count_uppercase = 0;
  int count_digits = 0;

  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    int ch = (*gi).first;
    if (ch < 128) {
      if (islower(ch)) {
        count_lowercase++;
        lowercase[ch - 'a'] = true;

      } else if (isupper(ch)) {
        count_uppercase++;
        uppercase[ch - 'A'] = true;

      } else if (isdigit(ch)) {
        count_digits++;
        digits[ch - '0'] = true;
      }
    }
  }

  if (count_lowercase == num_letters) {
    indent(out, indent_level + 2)
      << "All lowercase letters\n";

  } else if (count_lowercase > 0) {
    indent(out, indent_level + 2)
      << "Some lowercase letters: ";
    for (int i = 0; i < num_letters; i++) {
      if (lowercase[i]) {
        out << (char)(i + 'a');
      }
    }
    out << "\n";
  }

  if (count_uppercase == num_letters) {
    indent(out, indent_level + 2)
      << "All uppercase letters\n";

  } else if (count_uppercase > 0) {
    indent(out, indent_level + 2)
      << "Some uppercase letters: ";
    for (int i = 0; i < num_letters; i++) {
      if (uppercase[i]) {
        out << (char)(i + 'A');
      }
    }
    out << "\n";
  }

  if (count_digits == num_digits) {
    indent(out, indent_level + 2)
      << "All digits\n";

  } else if (count_digits > 0) {
    indent(out, indent_level + 2)
      << "Some digits: ";
    for (int i = 0; i < num_digits; i++) {
      if (digits[i]) {
        out << (char)(i + '0');
      }
    }
    out << "\n";
  }

  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    int ch = (*gi).first;
    if (ch >= 128 || !isalnum(ch)) {
      indent(out, indent_level + 2)
        << ch;
      if (ch < isprint(ch)) {
        out << " = '" << (char)ch << "'\n";
      }
    }
  }
}

/**
 * Gets the glyph associated with the given character code, as well as an
 * optional scaling parameter that should be applied to the glyph's geometry
 * and advance parameters.  Returns true if the glyph exists, false if it does
 * not.  Even if the return value is false, the value for glyph might be
 * filled in with a printable glyph.
 */
bool StaticTextFont::
get_glyph(int character, CPT(TextGlyph) &glyph) {
  Glyphs::const_iterator gi = _glyphs.find(character);
  if (gi == _glyphs.end()) {
    // No definition for this character.
    glyph = get_invalid_glyph();
    return false;
  }

  glyph = (*gi).second;
  return true;
}

/**
 * Given that 'root' is a PandaNode containing at least a polygon and a point
 * which define the character's appearance and kern position, respectively,
 * recursively walk the hierarchy and root and locate those two Geoms.
 */
void StaticTextFont::
find_character_gsets(PandaNode *root, CPT(Geom) &ch, CPT(Geom) &dot,
                     const RenderState *&state, const RenderState *net_state) {
  CPT(RenderState) next_net_state = net_state->compose(root->get_state());

  if (root->is_geom_node()) {
    GeomNode *geode = DCAST(GeomNode, root);

    for (int i = 0; i < geode->get_num_geoms(); i++) {
      const Geom *geom = geode->get_geom(i);

      bool found_points = false;
      for (size_t j = 0; j < geom->get_num_primitives() && !found_points; ++j) {
        const GeomPrimitive *primitive = geom->get_primitive(j);
        if (primitive->is_of_type(GeomPoints::get_class_type())) {
          dot = geom;
          found_points = true;
        }
      }
      if (!found_points) {
        // If it doesn't have any points, it must be the regular letter.
        ch = geom;
        state = next_net_state->compose(geode->get_geom_state(i));
      }
    }

  } else {
    PandaNode::Children cr = root->get_children();
    int num_children = cr.get_num_children();
    for (int i = 0; i < num_children; i++) {
      find_character_gsets(cr.get_child(i), ch, dot, state,
                           next_net_state);
    }
  }
}

/**
 * Walk the hierarchy beginning at the indicated root and locate any nodes
 * whose names are just integers.  These are taken to be characters, and their
 * definitions and kern informations are retrieved.
 */
void StaticTextFont::
find_characters(PandaNode *root, const RenderState *net_state) {
  CPT(RenderState) next_net_state = net_state->compose(root->get_state());
  std::string name = root->get_name();

  bool all_digits = !name.empty();
  const char *p = name.c_str();
  while (all_digits && *p != '\0') {
    // VC++ complains if we treat an int as a bool, so we have to do this != 0
    // comparison on the int isdigit() function to shut it up.
    all_digits = (isdigit(*p) != 0);
    p++;
  }

  if (all_digits) {
    int character = atoi(name.c_str());
    CPT(Geom) ch;
    CPT(Geom) dot;
    const RenderState *state = nullptr;
    find_character_gsets(root, ch, dot, state, next_net_state);
    PN_stdfloat width = 0.0;
    if (dot != nullptr) {
      // Get the first vertex from the "dot" geoset.  This will be the origin
      // of the next character.
      GeomVertexReader reader(dot->get_vertex_data(), InternalName::get_vertex());
      width = reader.get_data1f();
    }

    _glyphs[character] = new TextGlyph(character, ch, state, width);

  } else if (name == "ds") {
    // The group "ds" is a special node that indicates the font's design size,
    // or line height.

    CPT(Geom) ch;
    CPT(Geom) dot;
    const RenderState *state = nullptr;
    find_character_gsets(root, ch, dot, state, next_net_state);
    if (dot != nullptr) {
      // Get the first vertex from the "dot" geoset.  This will be the design
      // size indicator.
      GeomVertexReader reader(dot->get_vertex_data(), InternalName::get_vertex());
      LVecBase3 data = reader.get_data3();
      _line_height = data[2];
      _total_poly_margin = data[0];
      _space_advance = 0.25f * _line_height;
    }

  } else {
    PandaNode::Children cr = root->get_children();
    int num_children = cr.get_num_children();
    for (int i = 0; i < num_children; i++) {
      find_characters(cr.get_child(i), next_net_state);
    }
  }
}
