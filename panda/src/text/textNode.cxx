// Filename: textNode.cxx
// Created by:  drose (13Mar02)
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

#include "textNode.h"
#include "textGlyph.h"
#include "stringDecoder.h"
#include "config_text.h"
#include "fontPool.h"
#include "default_font.h"
#include "dynamicTextFont.h"

#include "compose_matrix.h"
#include "geom.h"
#include "geomTristrip.h"
#include "geomLinestrip.h"
#include "geomPoint.h"
#include "geomNode.h"
#include "notify.h"
#include "transformState.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "cullBinAttrib.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"
#include "sceneGraphReducer.h"
#include "indent.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "geometricBoundingVolume.h"
#include "accumulatedAttribs.h"
#include "dcast.h"

#include <stdio.h>
#include <ctype.h>

TypeHandle TextNode::_type_handle;

PT(TextFont) TextNode::_default_font;
bool TextNode::_loaded_default_font = false;
TextNode::Encoding TextNode::_default_encoding;

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextNode::
TextNode(const string &name) : PandaNode(name) {
  _encoding = _default_encoding;
  _slant = 0.0f;
  
  // Initially, since the text string is empty, we know that both
  // _text and _wtext accurately reflect the empty state; so we "got"
  // both of them.
  _flags = (F_got_text | F_got_wtext);
  _align = A_left;
  _wordwrap_width = 1.0f;

  _text_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _frame_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _card_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _shadow_color.set(1.0f, 1.0f, 1.0f, 1.0f);

  _frame_width = 1.0f;

  _frame_ul.set(0.0f, 0.0f);
  _frame_lr.set(0.0f, 0.0f);
  _card_ul.set(0.0f, 0.0f);
  _card_lr.set(0.0f, 0.0f);
  _shadow_offset.set(0.0f, 0.0f);

  _draw_order = 1;

  _transform = LMatrix4f::ident_mat();
  _coordinate_system = CS_default;

  _ul2d.set(0.0f, 0.0f);
  _lr2d.set(0.0f, 0.0f);
  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);
  _num_rows = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextNode::
~TextNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::wordwrap_to
//       Access: Published
//  Description: Inserts newlines into the given text at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).  Returns the new string.
////////////////////////////////////////////////////////////////////
string TextNode::
wordwrap_to(const string &text, float wordwrap_width,
            bool preserve_trailing_whitespace) const {
  nassertr(_font != (TextFont *)NULL, text);
  wstring decoded = decode_text(text);
  wstring wrapped = 
    _font->wordwrap_to(decoded, wordwrap_width, preserve_trailing_whitespace);
  return encode_wtext(wrapped);
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TextNode::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "TextNode " << get_name() << "\n";
  if (_font != (TextFont *)NULL) {
    indent(out, indent_level + 2)
      << "with font " << _font->get_name() << "\n";
  }
  if (has_text_color()) {
    indent(out, indent_level + 2)
      << "text color is " << _text_color << "\n";
  } else {
    indent(out, indent_level + 2)
      << "text color is unchanged from source\n";
  }
  indent(out, indent_level + 2)
    << "alignment is ";
  switch (_align) {
  case A_left:
    out << "A_left\n";
    break;

  case A_right:
    out << "A_right\n";
    break;

  case A_center:
    out << "A_center\n";
    break;
  }

  if (has_wordwrap()) {
    indent(out, indent_level + 2)
      << "Word-wrapping at " << _wordwrap_width << " units.\n";
  }

  if (has_frame()) {
    indent(out, indent_level + 2)
      << "frame of color " << _frame_color << " at "
      << get_frame_as_set() << " line width " << _frame_width << "\n";
    if (get_frame_corners()) {
      indent(out, indent_level + 2)
        << "frame corners are enabled\n";
    }
    if (is_frame_as_margin()) {
      indent(out, indent_level + 2)
        << "frame coordinates are specified as margin; actual frame is:\n"
        << get_frame_actual() << "\n";
    } else {
      indent(out, indent_level + 2)
        << "frame coordinates are actual\n";
    }
  }
  if (has_card()) {
    indent(out, indent_level + 2)
      << "card of color " << _card_color << " at "
      << get_card_as_set() << "\n";
    if (is_card_as_margin()) {
      indent(out, indent_level + 2)
        << "card coordinates are specified as margin; actual card is:\n"
        << get_card_actual() << "\n";
    } else {
      indent(out, indent_level + 2)
        << "card coordinates are actual\n";
    }
  }
  if (has_shadow()) {
    indent(out, indent_level + 2)
      << "shadow of color " << _shadow_color << " at "
      << _shadow_offset << "\n";
  }
  if (has_bin()) {
    indent(out, indent_level + 2)
      << "bin is " << _bin << "\n";
  }
  indent(out, indent_level + 2)
    << "draw order is " << _draw_order << ", "
    << _draw_order + 1 << ", " << _draw_order + 2 << "\n";

  LVecBase3f scale, hpr, trans;
  if (decompose_matrix(_transform, scale, hpr, trans, _coordinate_system)) {
  indent(out, indent_level + 2)
    << "transform is:\n"
    << "  scale: " << scale << "\n"
    << "    hpr: " << hpr << "\n"
    << "  trans: " << hpr << "\n";
  } else {
    indent(out, indent_level + 2)
      << "transform is:\n" << _transform;
  }
  indent(out, indent_level + 2)
    << "in coordinate system " << _coordinate_system << "\n";

  indent(out, indent_level + 2)
    << "\ntext is " << get_text() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::generate
//       Access: Published
//  Description: Generates the text, according to the parameters
//               indicated within the TextNode, and returns a Node
//               that may be parented within the tree to represent it.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
generate() {
  if (text_cat.is_debug()) {
    text_cat.debug()
      << "Rebuilding " << *this << " with '" << get_text() << "'\n";
  }

  // The strategy here will be to assemble together a bunch of
  // letters, instanced from the letter hierarchy of font_def, into
  // our own little hierarchy.

  // There will be one root over the whole text block, that
  // contains the transform passed in.  Under this root there will be
  // another node for each row, that moves the row into the right place
  // horizontally and vertically, and for each row, there is another
  // node for each character.

  _ul2d.set(0.0f, 0.0f);
  _lr2d.set(0.0f, 0.0f);
  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);
  _num_rows = 0;

  // Now build a new sub-tree for all the text components.
  PT(PandaNode) root = new PandaNode(get_text());

  if (!has_text()) {
    return root;
  }

  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    font = get_default_font();
  }

  if (font == (TextFont *)NULL) {
    return root;
  }

  // Compute the overall text transform matrix.  We build the text in
  // a Z-up coordinate system and then convert it to whatever the user
  // asked for.
  LMatrix4f mat =
    LMatrix4f::convert_mat(CS_zup_right, _coordinate_system) *
    _transform;

  root->set_transform(TransformState::make_mat(mat));

  wstring wtext = get_wtext();
  if (has_wordwrap()) {
    wtext = font->wordwrap_to(wtext, _wordwrap_width, false);
  }

  // Assemble the text.
  LVector2f ul, lr;
  int num_rows = 0;
  PT(PandaNode) text_root = 
    assemble_text(wtext.begin(), wtext.end(), font, 
                  ul, lr, num_rows);

  // Parent the text in.  We create an intermediate node so we can
  // choose to reinstance the text_root as the shadow, below.
  PT(PandaNode) text = new PandaNode("text");
  root->add_child(text, _draw_order + 2);
  text->add_child(text_root);

  if (has_text_color()) {
    text->set_attrib(ColorAttrib::make_flat(_text_color));
    if (_text_color[3] != 1.0) {
      text->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }
  }

  if (has_bin()) {
    text->set_attrib(CullBinAttrib::make(_bin, _draw_order + 2));
  }

  // Save the bounding-box information about the text in a form
  // friendly to the user.
  _num_rows = num_rows;
  _ul2d = ul;
  _lr2d = lr;
  _ul3d.set(ul[0], 0.0f, ul[1]);
  _lr3d.set(lr[0], 0.0f, lr[1]);

  _ul3d = _ul3d * _transform;
  _lr3d = _lr3d * _transform;

  // Incidentally, that means we don't need to measure the text now.
  _flags &= ~F_needs_measure;


  // Now deal with all the decorations.

  if (has_shadow()) {
    // Make a shadow by instancing the text behind itself.

    // For now, the depth offset is 0.0 because we don't expect to see
    // text with shadows in the 3-d world that aren't decals.  Maybe
    // this will need to be addressed in the future.

    LMatrix4f offset =
      LMatrix4f::translate_mat(_shadow_offset[0], 0.0f, -_shadow_offset[1]);
    PT(PandaNode) shadow = new PandaNode("shadow");
    root->add_child(shadow, _draw_order + 1);
    shadow->add_child(text_root);
    shadow->set_transform(TransformState::make_mat(offset));
    shadow->set_attrib(ColorAttrib::make_flat(_shadow_color));

    if (_shadow_color[3] != 1.0f) {
      shadow->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }

    if (has_bin()) {
      shadow->set_attrib(CullBinAttrib::make(_bin, _draw_order + 1));
    }
  }

  if (has_frame()) {
    PT(PandaNode) frame_root = make_frame();
    root->add_child(frame_root, _draw_order + 1);
    frame_root->set_attrib(ColorAttrib::make_flat(_frame_color));
    if (_frame_color[3] != 1.0f) {
      frame_root->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }

    if (has_bin()) {
      frame_root->set_attrib(CullBinAttrib::make(_bin, _draw_order + 1));
    }
  }

  if (has_card()) {
    PT(PandaNode) card_root;
    if (has_card_border())
      card_root = make_card_with_border();
    else
      card_root = make_card();
    root->add_child(card_root, _draw_order);
    card_root->set_attrib(ColorAttrib::make_flat(_card_color));
    if (_card_color[3] != 1.0f) {
      card_root->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }
    if (has_card_texture()) {
      card_root->set_attrib(TextureAttrib::make(_card_texture));
    }

    if (has_bin()) {
      card_root->set_attrib(CullBinAttrib::make(_bin, _draw_order));
    }
  }

  // Now flatten our hierarchy to get rid of the transforms we put in,
  // applying them to the vertices.

  if (text_flatten) {
    SceneGraphReducer gr;
    gr.apply_attribs(root);
    gr.flatten(root, true);
  }

  return root;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::encode_wchar
//       Access: Public
//  Description: Encodes a single wide char into a one-, two-, or
//               three-byte string, according to the current encoding
//               system in effect.
////////////////////////////////////////////////////////////////////
string TextNode::
encode_wchar(wchar_t ch) const {
  switch (_encoding) {
  case E_iso8859:
    if (isascii((unsigned int)ch)) {
      return string(1, (char)ch);
    } else {
      return ".";
    }

  case E_utf8:
    if (ch < 0x80) {
      return string(1, (char)ch);
    } else if (ch < 0x800) {
      return 
        string(1, (char)((ch >> 6) | 0xc0)) +
        string(1, (char)((ch & 0x3f) | 0x80));
    } else {
      return 
        string(1, (char)((ch >> 12) | 0xe0)) +
        string(1, (char)(((ch >> 6) & 0x3f) | 0x80)) +
        string(1, (char)((ch & 0x3f) | 0x80));
    }

  case E_unicode:
    return
      string(1, (char)(ch >> 8)) + 
      string(1, (char)(ch & 0xff));
  }

  return "";
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::encode_wtext
//       Access: Public
//  Description: Encodes a wide-text string into a single-char string,
//               accoding to the current encoding.
////////////////////////////////////////////////////////////////////
string TextNode::
encode_wtext(const wstring &wtext) const {
  string result;

  for (wstring::const_iterator pi = wtext.begin(); pi != wtext.end(); ++pi) {
    result += encode_wchar(*pi);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::decode_text
//       Access: Public
//  Description: Returns the given wstring decoded to a single-byte
//               string, via the current encoding system.
////////////////////////////////////////////////////////////////////
wstring TextNode::
decode_text(const string &text) const {
  switch (_encoding) {
  case E_utf8:
    {
      StringUtf8Decoder decoder(text);
      return decode_text_impl(decoder);
    }

  case E_unicode:
    {
      StringUnicodeDecoder decoder(text);
      return decode_text_impl(decoder);
    }

  case E_iso8859:
  default:
    {
      StringDecoder decoder(text);
      return decode_text_impl(decoder);
    }
  };
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_unsafe_to_apply_attribs
//       Access: Public, Virtual
//  Description: Returns the union of all attributes from
//               SceneGraphReducer::AttribTypes that may not safely be
//               applied to the vertices of this node.  If this is
//               nonzero, these attributes must be dropped at this
//               node as a state change.
//
//               This is a generalization of safe_to_transform().
////////////////////////////////////////////////////////////////////
int TextNode::
get_unsafe_to_apply_attribs() const {
  // We have no way to apply these kinds of attributes to our
  // TextNode, so insist they get dropped into the PandaNode's basic
  // state.
  return 
    SceneGraphReducer::TT_tex_matrix | 
    SceneGraphReducer::TT_other;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::apply_attribs_to_vertices
//       Access: Public, Virtual
//  Description: Applies whatever attributes are specified in the
//               AccumulatedAttribs object (and by the attrib_types
//               bitmask) to the vertices on this node, if
//               appropriate.  If this node uses geom arrays like a
//               GeomNode, the supplied GeomTransformer may be used to
//               unify shared arrays across multiple different nodes.
//
//               This is a generalization of xform().
////////////////////////////////////////////////////////////////////
void TextNode::
apply_attribs_to_vertices(const AccumulatedAttribs &attribs, int attrib_types,
                          GeomTransformer &transformer) {
  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    const LMatrix4f &mat = attribs._transform->get_mat();
    _transform *= mat;

    if ((_flags & F_needs_measure) == 0) {
      // If we already have a measure, transform it too.  We don't
      // need to invalidate the 2-d parts, since that's not affected
      // by the transform anyway.
      _ul3d = _ul3d * mat;
      _lr3d = _lr3d * mat;
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
    if (attribs._color != (const RenderAttrib *)NULL) {
      const ColorAttrib *ca = DCAST(ColorAttrib, attribs._color);
      if (ca->get_color_type() == ColorAttrib::T_flat) {
        const Colorf &c = ca->get_color();
        _text_color = c;
        _frame_color = c;
        _card_color = c;
        _shadow_color = c;
        _flags |= F_has_text_color;
      }
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
    if (attribs._color_scale != (const RenderAttrib *)NULL) {
      const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attribs._color_scale);
      const LVecBase4f &s = csa->get_scale();
      if (s != LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f)) {
        _text_color[0] *= s[0];
        _text_color[1] *= s[1];
        _text_color[2] *= s[2];
        _text_color[3] *= s[3];
        _frame_color[0] *= s[0];
        _frame_color[1] *= s[1];
        _frame_color[2] *= s[2];
        _frame_color[3] *= s[3];
        _card_color[0] *= s[0];
        _card_color[1] *= s[1];
        _card_color[2] *= s[2];
        _card_color[3] *= s[3];
        _shadow_color[0] *= s[0];
        _shadow_color[1] *= s[1];
        _shadow_color[2] *= s[2];
        _shadow_color[3] *= s[3];
      }
    }
  }

  // Now propagate the attributes down to our already-generated
  // geometry, if we have any.
  if ((_flags & F_needs_rebuild) == 0 && 
      _internal_geom != (PandaNode *)NULL) {
    SceneGraphReducer gr;
    gr.apply_attribs(_internal_geom, attribs, attrib_types, transformer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_tight_bounds
//       Access: Public, Virtual
//  Description: This is used to support
//               NodePath::calc_tight_bounds().  It is not intended to
//               be called directly, and it has nothing to do with the
//               normal Panda bounding-volume computation.
//
//               If the node contains any geometry, this updates
//               min_point and max_point to enclose its bounding box.
//               found_any is to be set true if the node has any
//               geometry at all, or left alone if it has none.  This
//               method may be called over several nodes, so it may
//               enter with min_point, max_point, and found_any
//               already set.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TextNode::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point, bool &found_any,
                  const TransformState *transform) const {
  CPT(TransformState) next_transform = 
    PandaNode::calc_tight_bounds(min_point, max_point, found_any, transform);

  check_rebuild();

  if (_internal_geom != (PandaNode *)NULL) {
    _internal_geom->calc_tight_bounds(min_point, max_point, 
                                      found_any, next_transform);
  }

  return next_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_cull_callback
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool TextNode::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::cull_callback
//       Access: Protected, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool TextNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  check_rebuild();
  if (_internal_geom != (PandaNode *)NULL) {
    // Render the text with this node.
    CullTraverserData next_data(data, _internal_geom);
    trav->traverse(next_data);
  }

  // Now continue to render everything else below this node.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::recompute_internal_bound
//       Access: Protected, Virtual
//  Description: Called when needed to recompute the node's
//               _internal_bound object.  Nodes that contain anything
//               of substance should redefine this to do the right
//               thing.
////////////////////////////////////////////////////////////////////
BoundingVolume *TextNode::
recompute_internal_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = PandaNode::recompute_internal_bound();
  nassertr(bound != (BoundingVolume *)NULL, bound);

  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now enclose the bounding box around the text.  We can do this
  // without actually generating the text, if we have at least
  // measured it.
  check_measure();

  LPoint3f vertices[8];
  vertices[0].set(_ul3d[0], _ul3d[1], _ul3d[2]);
  vertices[1].set(_ul3d[0], _ul3d[1], _lr3d[2]);
  vertices[2].set(_ul3d[0], _lr3d[1], _ul3d[2]);
  vertices[3].set(_ul3d[0], _lr3d[1], _lr3d[2]);
  vertices[4].set(_lr3d[0], _ul3d[1], _ul3d[2]);
  vertices[5].set(_lr3d[0], _ul3d[1], _lr3d[2]);
  vertices[6].set(_lr3d[0], _lr3d[1], _ul3d[2]);
  vertices[7].set(_lr3d[0], _lr3d[1], _lr3d[2]);

  gbv->around(vertices, vertices + 8);

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::decode_text_impl
//       Access: Private
//  Description: Decodes the eight-bit stream from the indicated
//               decoder, returning the decoded wide-char string.
////////////////////////////////////////////////////////////////////
wstring TextNode::
decode_text_impl(StringDecoder &decoder) const {
  wstring result;
  bool expand_amp = get_expand_amp();

  wchar_t character = decoder.get_next_character();
  while (!decoder.is_eof()) {
    if (character == '&' && expand_amp) {
      // An ampersand in expand_amp mode is treated as an escape
      // character.
      character = expand_amp_sequence(decoder);
    }
    result += character;
    character = decoder.get_next_character();
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::expand_amp_sequence
//       Access: Private
//  Description: Given that we have just read an ampersand from the
//               StringDecoder, and that we have expand_amp in effect
//               and are therefore expected to expand the sequence
//               that this ampersand begins into a single unicode
//               character, do the expansion and return the character.
////////////////////////////////////////////////////////////////////
int TextNode::
expand_amp_sequence(StringDecoder &decoder) const {
  int result = 0;

  int character = decoder.get_next_character();
  if (!decoder.is_eof() && character == '#') {
    // An explicit numeric sequence: &#nnn;
    result = 0;
    character = decoder.get_next_character();
    while (!decoder.is_eof() && character < 128 && isdigit((unsigned int)character)) {
      result = (result * 10) + (character - '0');
      character = decoder.get_next_character();
    }
    if (character != ';') {
      // Invalid sequence.
      return 0;
    }

    return result;
  }

  string sequence;
  
  // Some non-numeric sequence.
  while (!decoder.is_eof() && character < 128 && isalpha((unsigned int)character)) {
    sequence += character;
    character = decoder.get_next_character();
  }
  if (character != ';') {
    // Invalid sequence.
    return 0;
  }

  static const struct {
    const char *name;
    int code;
  } tokens[] = {
    { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' },
    { "nbsp", ' ' /* 160 */ },

    { "iexcl", 161 }, { "cent", 162 }, { "pound", 163 }, { "curren", 164 },
    { "yen", 165 }, { "brvbar", 166 }, { "brkbar", 166 }, { "sect", 167 },
    { "uml", 168 }, { "die", 168 }, { "copy", 169 }, { "ordf", 170 },
    { "laquo", 171 }, { "not", 172 }, { "shy", 173 }, { "reg", 174 },
    { "macr", 175 }, { "hibar", 175 }, { "deg", 176 }, { "plusmn", 177 },
    { "sup2", 178 }, { "sup3", 179 }, { "acute", 180 }, { "micro", 181 },
    { "para", 182 }, { "middot", 183 }, { "cedil", 184 }, { "sup1", 185 },
    { "ordm", 186 }, { "raquo", 187 }, { "frac14", 188 }, { "frac12", 189 },
    { "frac34", 190 }, { "iquest", 191 }, { "Agrave", 192 }, { "Aacute", 193 },
    { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 }, { "Aring", 197 },
    { "AElig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 }, { "Eacute", 201 },
    { "Ecirc", 202 }, { "Euml", 203 }, { "Igrave", 204 }, { "Iacute", 205 },
    { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 }, { "Dstrok", 208 },
    { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 },
    { "Otilde", 213 }, { "Ouml", 214 }, { "times", 215 }, { "Oslash", 216 },
    { "Ugrave", 217 }, { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 },
    { "Yacute", 221 }, { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 },
    { "aacute", 225 }, { "acirc", 226 }, { "atilde", 227 }, { "auml", 228 },
    { "aring", 229 }, { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 },
    { "eacute", 233 }, { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 },
    { "iacute", 237 }, { "icirc", 238 }, { "iuml", 239 }, { "eth", 240 },
    { "ntilde", 241 }, { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 },
    { "otilde", 245 }, { "ouml", 246 }, { "divide", 247 }, { "oslash", 248 },
    { "ugrave", 249 }, { "uacute", 250 }, { "ucirc", 251 }, { "uuml", 252 },
    { "yacute", 253 }, { "thorn", 254 }, { "yuml", 255 },

    { NULL, 0 },
  };

  for (int i = 0; tokens[i].name != NULL; i++) {
    if (sequence == tokens[i].name) {
      // Here's a match.
      return tokens[i].code;
    }
  }

  // Some unrecognized sequence.
  return 0;
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::do_rebuild
//       Access: Private
//  Description: Removes any existing children of the TextNode, and
//               adds the newly generated text instead.
////////////////////////////////////////////////////////////////////
void TextNode::
do_rebuild() {
  _flags &= ~(F_needs_rebuild | F_needs_measure);
  _internal_geom = generate();
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::do_measure
//       Access: Private
//  Description: Can be called in lieu of do_rebuild() to measure the
//               text and set up the bounding boxes properly without
//               actually assembling it.
////////////////////////////////////////////////////////////////////
void TextNode::
do_measure() {
  _flags &= ~F_needs_measure;

  _ul2d.set(0.0f, 0.0f);
  _lr2d.set(0.0f, 0.0f);
  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);
  _num_rows = 0;

  if (!has_text()) {
    return;
  }

  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    font = get_default_font();
  }

  if (font == (TextFont *)NULL) {
    return;
  }

  wstring wtext = get_wtext();
  if (has_wordwrap()) {
    wtext = font->wordwrap_to(wtext, _wordwrap_width, false);
  }

  LVector2f ul, lr;
  int num_rows = 0;
  measure_text(wtext.begin(), wtext.end(), font,
               ul, lr, num_rows);

  _num_rows = num_rows;
  _ul2d = ul;
  _lr2d = lr;
  _ul3d.set(ul[0], 0.0f, ul[1]);
  _lr3d.set(lr[0], 0.0f, lr[1]);

  _ul3d = _ul3d * _transform;
  _lr3d = _lr3d * _transform;
}

#ifndef CPPPARSER  // interrogate has a bit of trouble with wstring.

////////////////////////////////////////////////////////////////////
//     Function: TextNode::assemble_row
//       Access: Private
//  Description: Assembles the letters in the source string, up until
//               the first newline or the end of the string into a
//               single row (which is parented to 'dest'), and returns
//               the length of the row.  The source pointer is moved
//               to the terminating character.
////////////////////////////////////////////////////////////////////
float TextNode::
assemble_row(wstring::iterator &si, const wstring::iterator &send, 
             TextFont *font, GeomNode *dest, const LMatrix4f &mat) {
  float xpos = 0.0f;
  while (si != send && (*si) != '\n') {
    wchar_t character = *si;

    if (character == ' ') {
      // A space is a special case.
      xpos += font->get_space_advance();

    } else {
      // A printable character.

      const TextGlyph *glyph;
      float glyph_scale;
      if (!font->get_glyph(character, glyph, glyph_scale)) {
        text_cat.warning()
          << "No definition in " << font->get_name() 
          << " for character " << character;
        if (character < 128 && isprint((unsigned int)character)) {
          text_cat.warning(false)
            << " ('" << (char)character << "')";
        }
        text_cat.warning(false)
          << "\n";
      }

      if (glyph != (TextGlyph *)NULL) {
        PT(Geom) char_geom = glyph->get_geom();
        const RenderState *state = glyph->get_state();

        if (char_geom != (Geom *)NULL) {
          LMatrix4f mat2 = LMatrix4f::scale_mat(glyph_scale);
          mat2.set_row(3, LVector3f(xpos, 0.0f, 0.0f));
          LMatrix4f xform = mat2 * mat;

          // Transform the vertices of the geom appropriately.  We
          // assume the geom is non-indexed.
          PTA_Vertexf coords;
          PTA_ushort index;
          char_geom->get_coords(coords, index);
          PTA_Vertexf new_coords;
          new_coords.reserve(coords.size());
          PTA_Vertexf::const_iterator vi;
          for (vi = coords.begin(); vi != coords.end(); ++vi) {
            new_coords.push_back((*vi) * xform);
          }
          nassertr(new_coords.size() == coords.size(), false);
          char_geom->set_coords(new_coords);

          // Now add the geom to the destination node.
          dest->add_geom(char_geom, state);
        }

        xpos += glyph->get_advance() * glyph_scale;
      }
    }
    ++si;
  }

  return xpos;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::assemble_text
//       Access: Private
//  Description: Constructs a hierarchy of nodes that contain the
//               geometry representing the indicated source text, and
//               returns it.  Also sets the ul, lr corners.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
assemble_text(wstring::iterator si, const wstring::iterator &send,
              TextFont *font, LVector2f &ul, LVector2f &lr, int &num_rows) {
  float line_height = font->get_line_height();

  ul.set(0.0f, 0.8f * line_height);
  lr.set(0.0f, 0.0f);

  // Make a geom node to hold our formatted text geometry.
  PT(GeomNode) root_node = new GeomNode("text");

  float posy = 0.0f;
  while (si != send) {
    // First, just measure the row, so we know how wide it is.
    // (Centered or right-justified text will require us to know this
    // up front.)
    wstring::iterator tsi = si;
    float row_width = measure_row(tsi, send, font);

    LMatrix4f mat = LMatrix4f::ident_mat();
    if (_align == A_left) {
      mat.set_row(3, LVector3f(0.0f, 0.0f, posy));
      lr[0] = max(lr[0], row_width);

    } else if (_align == A_right) {
      mat.set_row(3, LVector3f(-row_width, 0.0f, posy));
      ul[0] = min(ul[0], -row_width);

    } else {
      float half_row_width=0.5f*row_width;
      mat.set_row(3, LVector3f(-half_row_width, 0.0f, posy));
      lr[0] = max(lr[0], half_row_width);
      ul[0] = min(ul[0], -half_row_width);
    }

    // Also apply whatever slant the user has asked for to the entire
    // row.  This is an X shear.
    if (_slant != 0.0f) {
      LMatrix4f shear(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      _slant, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
      mat = shear * mat;
    }

    // Now that we've computed the row's transform matrix, generate
    // the actual geoms for the row.
    assemble_row(si, send, font, root_node, mat);
    if (si != send) {
      // Skip past the newline.
      ++si;
    }

    posy -= line_height;
    num_rows++;
  }

  lr[1] = posy + 0.8f * line_height;

  return root_node.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::measure_row
//       Access: Private
//  Description: Returns the length of the row in units, as it would
//               be if it were assembled, without actually assembling
//               it.
////////////////////////////////////////////////////////////////////
float TextNode::
measure_row(wstring::iterator &si, const wstring::iterator &send,
            TextFont *font) {
  float xpos = 0.0f;
  while (si != send && *si != '\n') {
    wchar_t character = *si;

    if (character == ' ') {
      // A space is a special case.
      xpos += font->get_space_advance();

    } else {
      // A printable character.

      const TextGlyph *glyph;
      float glyph_scale;
      font->get_glyph(character, glyph, glyph_scale);
      if (glyph != (TextGlyph *)NULL) {
        xpos += glyph->get_advance() * glyph_scale;
      }
    }
    ++si;
  }

  return xpos;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::measure_text
//       Access: Private
//  Description: Sets the ul, lr corners to fit the text, without
//               actually assembling it.
////////////////////////////////////////////////////////////////////
void TextNode::
measure_text(wstring::iterator si, const wstring::iterator &send,
             TextFont *font, LVector2f &ul, LVector2f &lr, int &num_rows) {
  float line_height = font->get_line_height();

  ul.set(0.0f, 0.8f * line_height);
  lr.set(0.0f, 0.0f);

  float posy = 0.0f;
  while (si != send) {
    float row_width = measure_row(si, send, font);
    if (si != send) {
      // Skip past the newline.
      ++si;
    }

    if (_align == A_left) {
      lr[0] = max(lr[0], row_width);

    } else if (_align == A_right) {
      ul[0] = min(ul[0], -row_width);

    } else {
      float half_row_width=0.5f*row_width;

      lr[0] = max(lr[0], half_row_width);
      ul[0] = min(ul[0], -half_row_width);
    }

    posy -= line_height;
    num_rows++;
  }

  lr[1] = posy + 0.8f * line_height;
}
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_frame
//       Access: Private
//  Description: Creates a frame around the text.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
make_frame() {
  PT(GeomNode) frame_geode = new GeomNode("frame");

  LVector4f dimensions = get_frame_actual();
  float left = dimensions[0];
  float right = dimensions[1];
  float bottom = dimensions[2];
  float top = dimensions[3];

  GeomLinestrip *geoset = new GeomLinestrip;
  PTA_int lengths=PTA_int::empty_array(0);
  PTA_Vertexf verts;
  lengths.push_back(5);
  verts.push_back(Vertexf(left, 0.0f, top));
  verts.push_back(Vertexf(left, 0.0f, bottom));
  verts.push_back(Vertexf(right, 0.0f, bottom));
  verts.push_back(Vertexf(right, 0.0f, top));
  verts.push_back(Vertexf(left, 0.0f, top));

  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);

  geoset->set_coords(verts);
  geoset->set_width(_frame_width);
  frame_geode->add_geom(geoset);

  if (get_frame_corners()) {
    GeomPoint *geoset = new GeomPoint;

    geoset->set_num_prims(4);
    geoset->set_coords(verts);
    geoset->set_size(_frame_width);
    frame_geode->add_geom(geoset);
  }

  return frame_geode.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_card
//       Access: Private
//  Description: Creates a card behind the text.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
make_card() {
  PT(GeomNode) card_geode = new GeomNode("card");

  LVector4f dimensions = get_card_actual();
  float left = dimensions[0];
  float right = dimensions[1];
  float bottom = dimensions[2];
  float top = dimensions[3];

  GeomTristrip *geoset = new GeomTristrip;
  PTA_int lengths=PTA_int::empty_array(0);
  lengths.push_back(4);

  PTA_Vertexf verts;
  verts.push_back(Vertexf::rfu(left, 0.02f, top));
  verts.push_back(Vertexf::rfu(left, 0.02f, bottom));
  verts.push_back(Vertexf::rfu(right, 0.02f, top));
  verts.push_back(Vertexf::rfu(right, 0.02f, bottom));

  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);

  geoset->set_coords(verts);

  if (has_card_texture()) {
    PTA_TexCoordf uvs;
    uvs.push_back(TexCoordf(0.0f, 1.0f));
    uvs.push_back(TexCoordf(0.0f, 0.0f));
    uvs.push_back(TexCoordf(1.0f, 1.0f));
    uvs.push_back(TexCoordf(1.0f, 0.0f));

    geoset->set_texcoords(uvs, G_PER_VERTEX);
  }

  card_geode->add_geom(geoset);

  return card_geode.p();
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_card_with_border
//       Access: Private
//  Description: Creates a card behind the text with a specified border
//               for button edge or what have you.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
make_card_with_border() {
  PT(GeomNode) card_geode = new GeomNode("card");

  LVector4f dimensions = get_card_actual();
  float left = dimensions[0];
  float right = dimensions[1];
  float bottom = dimensions[2];
  float top = dimensions[3];

  // we now create three tri-strips instead of one
  // with vertices arranged as follows:
  //
  //  1 3            5 7  - one
  //  2 4            6 8  /  \ two
  //  9 11          13 15 \  /
  // 10 12          14 16 - three
  //
  GeomTristrip *geoset = new GeomTristrip;
  PTA_int lengths;
  lengths.push_back(8);
  lengths.push_back(8);
  lengths.push_back(8);

  PTA_Vertexf verts;
  // verts 1,2,3,4
  verts.push_back(Vertexf::rfu(left, 0.02f, top));
  verts.push_back(Vertexf::rfu(left, 0.02f, top - _card_border_size));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02f, top));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02f,
                               top - _card_border_size));
  // verts 5,6,7,8
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02f, top));
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02f,
                               top - _card_border_size));
  verts.push_back(Vertexf::rfu(right, 0.02f, top));
  verts.push_back(Vertexf::rfu(right, 0.02f, top - _card_border_size));
  // verts 9,10,11,12
  verts.push_back(Vertexf::rfu(left, 0.02f, bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(left, 0.02f, bottom));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02f,
                               bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02f, bottom));
  // verts 13,14,15,16
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02f,
                               bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02f, bottom));
  verts.push_back(Vertexf::rfu(right, 0.02f, bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(right, 0.02f, bottom));

  PTA_ushort indices;
  // tristrip #1
  indices.push_back(0);
  indices.push_back(1);
  indices.push_back(2);
  indices.push_back(3);
  indices.push_back(4);
  indices.push_back(5);
  indices.push_back(6);
  indices.push_back(7);
  // tristrip #2
  indices.push_back(1);
  indices.push_back(8);
  indices.push_back(3);
  indices.push_back(10);
  indices.push_back(5);
  indices.push_back(12);
  indices.push_back(7);
  indices.push_back(14);
  // tristrip #3
  indices.push_back(8);
  indices.push_back(9);
  indices.push_back(10);
  indices.push_back(11);
  indices.push_back(12);
  indices.push_back(13);
  indices.push_back(14);
  indices.push_back(15);

  geoset->set_num_prims(3);
  geoset->set_lengths(lengths);

  geoset->set_coords(verts,indices);

  if (has_card_texture()) {
    PTA_TexCoordf uvs;
    uvs.push_back(TexCoordf(0.0f, 1.0f)); //1
    uvs.push_back(TexCoordf(0.0f, 1.0f - _card_border_uv_portion)); //2
    uvs.push_back(TexCoordf(0.0f + _card_border_uv_portion, 1.0f)); //3
    uvs.push_back(TexCoordf(0.0f + _card_border_uv_portion,
      1.0f - _card_border_uv_portion)); //4
    uvs.push_back(TexCoordf( 1.0f -_card_border_uv_portion, 1.0f)); //5
    uvs.push_back(TexCoordf( 1.0f -_card_border_uv_portion,
      1.0f - _card_border_uv_portion)); //6
    uvs.push_back(TexCoordf(1.0f, 1.0f)); //7
    uvs.push_back(TexCoordf(1.0f, 1.0f - _card_border_uv_portion)); //8

    uvs.push_back(TexCoordf(0.0f, _card_border_uv_portion)); //9
    uvs.push_back(TexCoordf(0.0f, 0.0f)); //10
    uvs.push_back(TexCoordf(_card_border_uv_portion, _card_border_uv_portion)); //11
    uvs.push_back(TexCoordf(_card_border_uv_portion, 0.0f)); //12

    uvs.push_back(TexCoordf(1.0f - _card_border_uv_portion, _card_border_uv_portion));//13
    uvs.push_back(TexCoordf(1.0f - _card_border_uv_portion, 0.0f));//14
    uvs.push_back(TexCoordf(1.0f, _card_border_uv_portion));//15
    uvs.push_back(TexCoordf(1.0f, 0.0f));//16

    // we can use same ref's as before (same order)
    geoset->set_texcoords(uvs, G_PER_VERTEX, indices);

  }

  card_geode->add_geom(geoset);

  return card_geode.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::load_default_font
//       Access: Private, Static
//  Description: This functin is called once (or never), the first
//               time someone attempts to render a TextNode using the
//               default font.  It should attempt to load the default
//               font, using the compiled-in version if it is
//               available, or whatever system file may be named in
//               Configrc.
////////////////////////////////////////////////////////////////////
void TextNode::
load_default_font() {
  _loaded_default_font = true;

  if (!text_default_font.empty()) {
    // First, attempt to load the user-specified filename.
    _default_font = FontPool::load_font(text_default_font);
    if (_default_font->is_valid()) {
      return;
    }
  }

  // Then, attempt to load the compiled-in font, if we have one.
#if defined(HAVE_FREETYPE) && defined(COMPILE_IN_DEFAULT_FONT)
  _default_font = new DynamicTextFont((const char *)default_font_data, 
                                      default_font_size, 0);
  if (_default_font->is_valid()) {
    return;
  }
#endif

  // Finally, fall back to a hardcoded font file, which we hope is on
  // the model path.  (Use text_default_font, above, if you don't want
  // to use this file and would prefer to specify a different font
  // file instead.)
  _default_font = FontPool::load_font("cmss12");
}
