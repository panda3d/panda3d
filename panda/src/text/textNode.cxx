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

#include "compose_matrix.h"
#include "geom.h"
#include "geomTristrip.h"
#include "geomLinestrip.h"
#include "geomPoint.h"
#include "geomNode.h"
#include "notify.h"
#include "transformState.h"
#include "colorAttrib.h"
#include "cullBinAttrib.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"
#include "sceneGraphReducer.h"
#include "indent.h"

#include <stdio.h>
#include <ctype.h>

TypeHandle TextNode::_type_handle;

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

  _flags = 0;
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

  _freeze_level = 0;
 _needs_rebuild = false;
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
    << "\ntext is " << _text << "\n";
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
      << "Rebuilding " << *this << " with '" << _text << "'\n";
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
  PT(PandaNode) root = new PandaNode(_text);

  if (_text.empty() || _font.is_null()) {
    return root;
  }

  // Compute the overall text transform matrix.  We build the text in
  // a Z-up coordinate system and then convert it to whatever the user
  // asked for.
  LMatrix4f mat =
    LMatrix4f::convert_mat(CS_zup_right, _coordinate_system) *
    _transform;

  root->set_transform(TransformState::make_mat(mat));

  wstring wtext = _wtext;
  if (has_wordwrap()) {
    wtext = _font->wordwrap_to(wtext, _wordwrap_width, false);
  }

  // Assemble the text.
  LVector2f ul, lr;
  int num_rows = 0;
  PT(PandaNode) text_root = assemble_text(wtext.begin(), wtext.end(), ul, lr, num_rows);

  // Parent the text in.
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
//     Function: TextNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void TextNode::
xform(const LMatrix4f &mat) {
  _transform *= mat;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::decode_text_impl
//       Access: Private
//  Description: Decodes the eight-bit stream from the indicated
//               decoder, storing the decoded unicode characters in
//               _wtext.
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
  _needs_rebuild = false;

  remove_all_children();

  PT(PandaNode) new_text = generate();
  if (new_text != (PandaNode *)NULL) {
    add_child(new_text);

    /*
    // And we flatten one more time, to remove the new node itself if
    // possible (it might be an unneeded node above multiple
    // children).  This flatten operation should be fairly
    // lightweight; it's already pretty flat.
    SceneGraphReducer gr(RenderRelation::get_class_type());
    gr.flatten(this, false);
    */
  }
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
  _ul2d.set(0.0f, 0.0f);
  _lr2d.set(0.0f, 0.0f);
  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);
  _num_rows = 0;

  if (_text.empty() || _font.is_null()) {
    return;
  }

  wstring wtext = _wtext;
  if (has_wordwrap()) {
    wtext = _font->wordwrap_to(wtext, _wordwrap_width, false);
  }

  LVector2f ul, lr;
  int num_rows = 0;
  measure_text(wtext.begin(), wtext.end(), ul, lr, num_rows);

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
             PandaNode *dest) {
  nassertr(_font != (TextFont *)NULL, 0.0f);

  float xpos = 0.0f;
  while (si != send && (*si) != '\n') {
    wchar_t character = *si;

    if (character == ' ') {
      // A space is a special case.
      xpos += _font->get_space_advance();

    } else {
      // A printable character.

      const TextGlyph *glyph;
      float glyph_scale;
      if (!_font->get_glyph(character, glyph, glyph_scale)) {
        text_cat.warning()
          << "No definition in " << _font->get_name() 
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
          LMatrix4f mat = LMatrix4f::scale_mat(glyph_scale);
          mat.set_row(3, LVector3f(xpos, 0.0f, 0.0f));

          string ch(1, (char)character);
          PT(GeomNode) geode = new GeomNode(ch);
          geode->add_geom(char_geom, state);
          dest->add_child(geode);
          geode->set_transform(TransformState::make_mat(mat));
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
              LVector2f &ul, LVector2f &lr, int &num_rows) {
  nassertr(_font != (TextFont *)NULL, (PandaNode *)NULL);
  float line_height = get_line_height();

  ul.set(0.0f, 0.8f * line_height);
  lr.set(0.0f, 0.0f);

  // Make a group node to hold our formatted text geometry.
  PT(PandaNode) root_node = new PandaNode("text");

  float posy = 0.0f;
  int row_index = 0;
  while (si != send) {
    char numstr[20];
    sprintf(numstr, "row%d", row_index);
    nassertr(strlen(numstr) < 20, root_node);

    PT(PandaNode) row = new PandaNode(numstr);
    float row_width = assemble_row(si, send, row);
    if (si != send) {
      // Skip past the newline.
      ++si;
    }

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

    row->set_transform(TransformState::make_mat(mat));
    root_node->add_child(row);

    posy -= line_height;
    num_rows++;
  }

  lr[1] = posy + 0.8f * line_height;

  return root_node;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::measure_row
//       Access: Private
//  Description: Returns the length of the row in units, as it would
//               be if it were assembled, without actually assembling
//               it.
////////////////////////////////////////////////////////////////////
float TextNode::
measure_row(wstring::iterator &si, const wstring::iterator &send) {
  float xpos = 0.0f;
  while (si != send && *si != '\n') {
    wchar_t character = *si;

    if (character == ' ') {
      // A space is a special case.
      xpos += _font->get_space_advance();

    } else {
      // A printable character.

      const TextGlyph *glyph;
      float glyph_scale;
      if (_font->get_glyph(character, glyph, glyph_scale)) {
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
             LVector2f &ul, LVector2f &lr, int &num_rows) {
  nassertv(_font != (TextFont *)NULL);
  float line_height = get_line_height();

  ul.set(0.0f, 0.8f * line_height);
  lr.set(0.0f, 0.0f);

  float posy = 0.0f;
  while (si != send) {
    float row_width = measure_row(si, send);
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
