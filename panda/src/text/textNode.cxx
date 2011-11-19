// Filename: textNode.cxx
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "textNode.h"
#include "textGlyph.h"
#include "stringDecoder.h"
#include "config_text.h"
#include "textAssembler.h"

#include "compose_matrix.h"
#include "geom.h"
#include "geomLinestrips.h"
#include "geomPoints.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "geomNode.h"
#include "pnotify.h"
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
#include "renderState.h"
#include "renderModeAttrib.h"
#include "decalEffect.h"
#include "dcast.h"
#include "bamFile.h"
#include "zStream.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "boundingSphere.h"

#include <stdio.h>

TypeHandle TextNode::_type_handle;

PStatCollector TextNode::_text_generate_pcollector("*:Generate Text");

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextNode::
TextNode(const string &name) : PandaNode(name) {
  set_cull_callback();

  _flags = 0;
  _max_rows = 0;
  _usage_hint = GeomEnums::UH_static;
  _flatten_flags = 0;
  if (text_flatten) {
    _flatten_flags |= FF_strong;
  }
  if (text_dynamic_merge) {
    _flatten_flags |= FF_dynamic_merge;
  }

  if (text_small_caps) {
    set_small_caps(true);
  }

  _frame_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _card_color.set(1.0f, 1.0f, 1.0f, 1.0f);

  _frame_width = 1.0f;

  _frame_ul.set(0.0f, 0.0f);
  _frame_lr.set(0.0f, 0.0f);
  _card_ul.set(0.0f, 0.0f);
  _card_lr.set(0.0f, 0.0f);

  _transform = LMatrix4::ident_mat();
  _coordinate_system = CS_default;

  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Copy Constructor
//       Access: Published
//  Description: It's sort of a copy constructor: it copies the
//               indicated TextProperties, without copying a complete
//               TextNode.
////////////////////////////////////////////////////////////////////
TextNode::
TextNode(const string &name, const TextProperties &copy) : 
  PandaNode(name), TextProperties(copy) 
{
  _flags = 0;
  _max_rows = 0;
  _usage_hint = GeomEnums::UH_static;

  _frame_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _card_color.set(1.0f, 1.0f, 1.0f, 1.0f);

  _frame_width = 1.0f;

  _frame_ul.set(0.0f, 0.0f);
  _frame_lr.set(0.0f, 0.0f);
  _card_ul.set(0.0f, 0.0f);
  _card_lr.set(0.0f, 0.0f);

  _transform = LMatrix4::ident_mat();
  _coordinate_system = CS_default;

  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Copy Constructor
//       Access: Published
//  Description: OK, this is a true copy constructor.
////////////////////////////////////////////////////////////////////
TextNode::
TextNode(const TextNode &copy) : 
  PandaNode(copy), 
  TextEncoder(copy),
  TextProperties(copy),
  _card_texture(copy._card_texture),
  _frame_color(copy._frame_color),
  _card_color(copy._card_color),
  _flags(copy._flags),
  _max_rows(copy._max_rows),
  _usage_hint(GeomEnums::UH_static),
  _frame_width(copy._frame_width),
  _card_border_size(copy._card_border_size),
  _card_border_uv_portion(copy._card_border_uv_portion),
  _frame_ul(copy._frame_ul),
  _frame_lr(copy._frame_lr),
  _card_ul(copy._card_ul),
  _card_lr(copy._card_lr),
  _transform(copy._transform),
  _coordinate_system(copy._coordinate_system),
  _ul3d(copy._ul3d),
  _lr3d(copy._lr3d)
{
  invalidate_with_measure();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_copy
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *TextNode::
make_copy() const {
  return new TextNode(*this);
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
//     Function: TextNode::calc_width
//       Access: Published
//  Description: Returns the width of a single character of the font,
//               or 0.0 if the character is not known.  This may be a
//               wide character (greater than 255).
////////////////////////////////////////////////////////////////////
PN_stdfloat TextNode::
calc_width(wchar_t character) const {
  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    return 0.0f;
  }

  return TextAssembler::calc_width(character, *this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_exact_character
//       Access: Published
//  Description: Returns true if the named character exists in the
//               font exactly as named, false otherwise.  Note that
//               because Panda can assemble glyphs together
//               automatically using cheesy accent marks, this is not
//               a reliable indicator of whether a suitable glyph can
//               be rendered for the character.  For that, use
//               has_character() instead.
//
//               This returns true for whitespace and Unicode
//               whitespace characters (if they exist in the font),
//               but returns false for characters that would render
//               with the "invalid glyph".  It also returns false for
//               characters that would be synthesized within Panda,
//               but see has_character().
////////////////////////////////////////////////////////////////////
bool TextNode::
has_exact_character(wchar_t character) const {
  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    return false;
  }

  return TextAssembler::has_exact_character(character, *this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::has_character
//       Access: Published
//  Description: Returns true if the named character exists in the
//               font or can be synthesized by Panda, false otherwise.
//               (Panda can synthesize some accented characters by
//               combining similar-looking glyphs from the font.)
//
//               This returns true for whitespace and Unicode
//               whitespace characters (if they exist in the font),
//               but returns false for characters that would render
//               with the "invalid glyph".
////////////////////////////////////////////////////////////////////
bool TextNode::
has_character(wchar_t character) const {
  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    return false;
  }

  return TextAssembler::has_character(character, *this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::is_whitespace
//       Access: Published
//  Description: Returns true if the indicated character represents
//               whitespace in the font, or false if anything visible
//               will be rendered for it.
//
//               This returns true for whitespace and Unicode
//               whitespace characters (if they exist in the font),
//               and returns false for any other characters, including
//               characters that do not exist in the font (these would
//               be rendered with the "invalid glyph", which is
//               visible).
//
//               Note that this function can be reliably used to
//               identify Unicode whitespace characters only if the
//               font has all of the whitespace characters defined.
//               It will return false for any character not in the
//               font, even if it is an official Unicode whitespace
//               character.
////////////////////////////////////////////////////////////////////
bool TextNode::
is_whitespace(wchar_t character) const {
  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    return false;
  }

  return TextAssembler::is_whitespace(character, *this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Published
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character or any embedded control characters like \1
//               or \3.
////////////////////////////////////////////////////////////////////
PN_stdfloat TextNode::
calc_width(const wstring &line) const {
  PN_stdfloat width = 0.0f;

  wstring::const_iterator si;
  for (si = line.begin(); si != line.end(); ++si) {
    width += calc_width(*si);
  }

  return width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TextNode::
output(ostream &out) const {
  PandaNode::output(out);

  check_rebuild();
  int geom_count = 0;
  if (_internal_geom != (PandaNode *)NULL) {
    geom_count = count_geoms(_internal_geom);
  }

  out << " (" << geom_count << " geoms)";
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TextNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  TextProperties::write(out, indent_level + 2);
  indent(out, indent_level + 2)
    << "transform is: " << *TransformState::make_mat(_transform) << "\n";
  indent(out, indent_level + 2)
    << "in coordinate system " << _coordinate_system << "\n";
  indent(out, indent_level + 2)
    << "text is " << get_text() << "\n";
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
  PStatTimer timer(_text_generate_pcollector);
  if (text_cat.is_debug()) {
    text_cat.debug()
      << "Rebuilding " << get_type() << " " << get_name()
      << " with '" << get_text() << "'\n";
  }

  // The strategy here will be to assemble together a bunch of
  // letters, instanced from the letter hierarchy of font_def, into
  // our own little hierarchy.

  // There will be one root over the whole text block, that
  // contains the transform passed in.  Under this root there will be
  // another node for each row, that moves the row into the right place
  // horizontally and vertically, and for each row, there is another
  // node for each character.

  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);

  // Now build a new sub-tree for all the text components.
  string name = get_text();
  size_t newline = name.find('\n');
  if (newline != string::npos) {
    name = name.substr(0, newline);
  }
  PT(PandaNode) root = new PandaNode(name);

  if (!has_text()) {
    return root;
  }

  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    return root;
  }

  // Compute the overall text transform matrix.  We build the text in
  // a Z-up coordinate system and then convert it to whatever the user
  // asked for.
  LMatrix4 mat =
    LMatrix4::convert_mat(CS_zup_right, _coordinate_system) *
    _transform;

  CPT(TransformState) transform = TransformState::make_mat(mat);
  root->set_transform(transform);

  wstring wtext = get_wtext();

  // Assemble the text.
  TextAssembler assembler(this);
  assembler.set_properties(*this);
  assembler.set_max_rows(_max_rows);
  assembler.set_usage_hint(_usage_hint);
  assembler.set_dynamic_merge((_flatten_flags & FF_dynamic_merge) != 0);
  bool all_set = assembler.set_wtext(wtext);
  if (all_set) {
    // No overflow.
    _flags &= ~F_has_overflow;
  } else {
    // Overflow.
    _flags |= F_has_overflow;
  }

  PT(PandaNode) text_root = assembler.assemble_text();
  _text_ul = assembler.get_ul();
  _text_lr = assembler.get_lr();
  _num_rows = assembler.get_num_rows();
  _wordwrapped_wtext = assembler.get_wordwrapped_wtext();

  // Parent the text in.
  PT(PandaNode) text = new PandaNode("text");
  root->add_child(text, get_draw_order() + 2);
  text->add_child(text_root);

  // Save the bounding-box information about the text in a form
  // friendly to the user.
  const LVector2 &ul = assembler.get_ul();
  const LVector2 &lr = assembler.get_lr();
  _ul3d.set(ul[0], 0.0f, ul[1]);
  _lr3d.set(lr[0], 0.0f, lr[1]);

  _ul3d = _ul3d * _transform;
  _lr3d = _lr3d * _transform;

  // Incidentally, that means we don't need to measure the text now.
  _flags &= ~F_needs_measure;

  // Now flatten our hierarchy to get rid of the transforms we put in,
  // applying them to the vertices.

  NodePath root_np(root);
  if (_flatten_flags & FF_strong) {
    root_np.flatten_strong();
  } else if (_flatten_flags & FF_medium) {
    root_np.flatten_medium();
  } else if (_flatten_flags & FF_light) {
    root_np.flatten_light();
  }

  // Now deal with the decorations.

  if (has_card()) {
    PT(PandaNode) card_root;
    if (has_card_border()) {
      card_root = make_card_with_border();
    } else {
      card_root = make_card();
    }
    card_root->set_transform(transform);
    card_root->set_attrib(ColorAttrib::make_flat(get_card_color()));
    if (get_card_color()[3] != 1.0f) {
      card_root->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }
    if (has_card_texture()) {
      card_root->set_attrib(TextureAttrib::make(get_card_texture()));
    }

    if (has_bin()) {
      card_root->set_attrib(CullBinAttrib::make(get_bin(), get_draw_order()));
    }

    // We always apply attribs down to the card vertices.
    SceneGraphReducer gr;
    gr.apply_attribs(card_root);

    // In order to decal the text onto the card, the card must
    // become the parent of the text.
    card_root->add_child(root);
    root = card_root;

    if (get_card_decal()) {
      card_root->set_effect(DecalEffect::make());
    }
  }

  if (has_frame()) {
    PT(PandaNode) frame_root = make_frame();
    frame_root->set_transform(transform);
    root->add_child(frame_root, get_draw_order() + 1);
    frame_root->set_attrib(ColorAttrib::make_flat(get_frame_color()));
    if (get_frame_color()[3] != 1.0f) {
      frame_root->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }

    if (has_bin()) {
      frame_root->set_attrib(CullBinAttrib::make(get_bin(), get_draw_order() + 1));
    }

    SceneGraphReducer gr;
    gr.apply_attribs(frame_root);
  }

  return root;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::get_internal_geom
//       Access: Published
//  Description: Returns the actual node that is used internally to
//               render the text, if the TextNode is parented within
//               the scene graph.
//
//               In general, you should not call this method.  Call
//               generate() instead if you want to get a handle to
//               geometry that represents the text.  This method is
//               provided as a debugging aid only.
////////////////////////////////////////////////////////////////////
PandaNode *TextNode::
get_internal_geom() const {
  // Output a nuisance warning to discourage the naive from calling
  // this method accidentally.
  text_cat.info()
    << "TextNode::get_internal_geom() called.\n";
  check_rebuild();
  return _internal_geom;
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
    const LMatrix4 &mat = attribs._transform->get_mat();
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
        const LColor &c = ca->get_color();
        set_text_color(c);
        set_frame_color(c);
        set_card_color(c);
        set_shadow_color(c);
      }
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
    if (attribs._color_scale != (const RenderAttrib *)NULL) {
      const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attribs._color_scale);
      const LVecBase4 &s = csa->get_scale();
      if (s != LVecBase4(1.0f, 1.0f, 1.0f, 1.0f)) {
        LVecBase4 tc = get_text_color();
        tc[0] *= s[0];
        tc[1] *= s[1];
        tc[2] *= s[2];
        tc[3] *= s[3];
        set_text_color(tc);
        LVecBase4 sc = get_shadow_color();
        sc[0] *= s[0];
        sc[1] *= s[1];
        sc[2] *= s[2];
        sc[3] *= s[3];
        set_shadow_color(sc);
        LVecBase4 fc = get_frame_color();
        fc[0] *= s[0];
        fc[1] *= s[1];
        fc[2] *= s[2];
        fc[3] *= s[3];
        set_frame_color(fc);
        LVecBase4 cc = get_card_color();
        cc[0] *= s[0];
        cc[1] *= s[1];
        cc[2] *= s[2];
        cc[3] *= s[3];
        set_card_color(cc);
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
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  CPT(TransformState) next_transform = 
    PandaNode::calc_tight_bounds(min_point, max_point, found_any, transform,
                                 current_thread);

  check_rebuild();

  if (_internal_geom != (PandaNode *)NULL) {
    _internal_geom->calc_tight_bounds(min_point, max_point, 
                                      found_any, next_transform, current_thread);
  }

  return next_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::cull_callback
//       Access: Protected, Virtual
//  Description: This function will be called during the cull
//               traversal to perform any additional operations that
//               should be performed at cull time.  This may include
//               additional manipulation of render state or additional
//               visible/invisible decisions, or any other arbitrary
//               operation.
//
//               Note that this function will *not* be called unless
//               set_cull_callback() is called in the constructor of
//               the derived class.  It is necessary to call
//               set_cull_callback() to indicated that we require
//               cull_callback() to be called.
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
//     Function: TextNode::is_renderable
//       Access: Public, Virtual
//  Description: Returns true if there is some value to visiting this
//               particular node during the cull traversal for any
//               camera, false otherwise.  This will be used to
//               optimize the result of get_net_draw_show_mask(), so
//               that any subtrees that contain only nodes for which
//               is_renderable() is false need not be visited.
////////////////////////////////////////////////////////////////////
bool TextNode::
is_renderable() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::compute_internal_bounds
//       Access: Protected, Virtual
//  Description: Called when needed to recompute the node's
//               _internal_bound object.  Nodes that contain anything
//               of substance should redefine this to do the right
//               thing.
////////////////////////////////////////////////////////////////////
void TextNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  // First, get ourselves a fresh, empty bounding volume.
  PT(BoundingVolume) bound = new BoundingSphere;

  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now enclose the bounding box around the text.  We can do this
  // without actually generating the text, if we have at least
  // measured it.
  check_measure();

  LPoint3 vertices[8];
  vertices[0].set(_ul3d[0], _ul3d[1], _ul3d[2]);
  vertices[1].set(_ul3d[0], _ul3d[1], _lr3d[2]);
  vertices[2].set(_ul3d[0], _lr3d[1], _ul3d[2]);
  vertices[3].set(_ul3d[0], _lr3d[1], _lr3d[2]);
  vertices[4].set(_lr3d[0], _ul3d[1], _ul3d[2]);
  vertices[5].set(_lr3d[0], _ul3d[1], _lr3d[2]);
  vertices[6].set(_lr3d[0], _lr3d[1], _ul3d[2]);
  vertices[7].set(_lr3d[0], _lr3d[1], _lr3d[2]);

  gbv->around(vertices, vertices + 8);

  internal_bounds = bound;
  internal_vertices = 0;  // TODO: estimate this better.
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::r_prepare_scene
//       Access: Protected, Virtual
//  Description: The recursive implementation of prepare_scene().
//               Don't call this directly; call
//               PandaNode::prepare_scene() or
//               NodePath::prepare_scene() instead.
////////////////////////////////////////////////////////////////////
void TextNode::
r_prepare_scene(GraphicsStateGuardianBase *gsg, const RenderState *node_state,
                GeomTransformer &transformer, Thread *current_thread) {
  check_rebuild();

  PandaNode *child = _internal_geom;
  if (child != (PandaNode *)NULL) {
    CPT(RenderState) child_state = node_state->compose(child->get_state());
    child->r_prepare_scene(gsg, child_state, transformer, current_thread);
  }
  
  PandaNode::r_prepare_scene(gsg, node_state, transformer, current_thread);
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
  // We no longer make this a special case.
  do_rebuild();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_frame
//       Access: Private
//  Description: Creates a frame around the text.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
make_frame() {
  PT(GeomNode) frame_node = new GeomNode("frame");

  LVector4 dimensions = get_frame_actual();
  PN_stdfloat left = dimensions[0];
  PN_stdfloat right = dimensions[1];
  PN_stdfloat bottom = dimensions[2];
  PN_stdfloat top = dimensions[3];
    
  CPT(RenderAttrib) thick = RenderModeAttrib::make(RenderModeAttrib::M_unchanged, _frame_width);
  CPT(RenderState) state = RenderState::make(thick);

  PT(GeomVertexData) vdata = new GeomVertexData
    ("text", GeomVertexFormat::get_v3(), get_usage_hint());
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  
  vertex.add_data3(left, 0.0f, top);
  vertex.add_data3(left, 0.0f, bottom);
  vertex.add_data3(right, 0.0f, bottom);
  vertex.add_data3(right, 0.0f, top);
  
  PT(GeomLinestrips) frame = new GeomLinestrips(get_usage_hint());
  frame->add_consecutive_vertices(0, 4);
  frame->add_vertex(0);
  frame->close_primitive();
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(frame);
  frame_node->add_geom(geom, state);
  
  if (get_frame_corners()) {
    PT(GeomPoints) corners = new GeomPoints(get_usage_hint());
    corners->add_consecutive_vertices(0, 4);
    PT(Geom) geom2 = new Geom(vdata);
    geom2->add_primitive(corners);
    frame_node->add_geom(geom2, state);
  }

  return frame_node.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_card
//       Access: Private
//  Description: Creates a card behind the text.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
make_card() {
  PT(GeomNode) card_node = new GeomNode("card");

  LVector4 dimensions = get_card_actual();
  PN_stdfloat left = dimensions[0];
  PN_stdfloat right = dimensions[1];
  PN_stdfloat bottom = dimensions[2];
  PN_stdfloat top = dimensions[3];

  PT(GeomVertexData) vdata = new GeomVertexData
    ("text", GeomVertexFormat::get_v3t2(), get_usage_hint());
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
  
  vertex.add_data3(left, 0.0f, top);
  vertex.add_data3(left, 0.0f, bottom);
  vertex.add_data3(right, 0.0f, top);
  vertex.add_data3(right, 0.0f, bottom);
  
  texcoord.add_data2(0.0f, 1.0f);
  texcoord.add_data2(0.0f, 0.0f);
  texcoord.add_data2(1.0f, 1.0f);
  texcoord.add_data2(1.0f, 0.0f);
  
  PT(GeomTristrips) card = new GeomTristrips(get_usage_hint());
  card->add_consecutive_vertices(0, 4);
  card->close_primitive();
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(card);
  
  card_node->add_geom(geom);

  return card_node.p();
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_card_with_border
//       Access: Private
//  Description: Creates a card behind the text with a specified border
//               for button edge or what have you.
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextNode::
make_card_with_border() {
  PT(GeomNode) card_node = new GeomNode("card");

  LVector4 dimensions = get_card_actual();
  PN_stdfloat left = dimensions[0];
  PN_stdfloat right = dimensions[1];
  PN_stdfloat bottom = dimensions[2];
  PN_stdfloat top = dimensions[3];

  // we now create three tri-strips instead of one
  // with vertices arranged as follows:
  //
  //  1 3            5 7  - one
  //  2 4            6 8  /  \ two
  //  9 11          13 15 \  /
  // 10 12          14 16 - three
  //

  PT(GeomVertexData) vdata = new GeomVertexData
    ("text", GeomVertexFormat::get_v3t2(), get_usage_hint());
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
  
  // verts 1,2,3,4
  vertex.add_data3(left, 0.02, top);
  vertex.add_data3(left, 0.02, top - _card_border_size);
  vertex.add_data3(left + _card_border_size, 0.02, top);
  vertex.add_data3(left + _card_border_size, 0.02,
                    top - _card_border_size);
  // verts 5,6,7,8
  vertex.add_data3(right - _card_border_size, 0.02, top);
  vertex.add_data3(right - _card_border_size, 0.02,
                    top - _card_border_size);
  vertex.add_data3(right, 0.02, top);
  vertex.add_data3(right, 0.02, top - _card_border_size);
  // verts 9,10,11,12
  vertex.add_data3(left, 0.02, bottom + _card_border_size);
  vertex.add_data3(left, 0.02, bottom);
  vertex.add_data3(left + _card_border_size, 0.02,
                    bottom + _card_border_size);
  vertex.add_data3(left + _card_border_size, 0.02, bottom);
  // verts 13,14,15,16
  vertex.add_data3(right - _card_border_size, 0.02,
                    bottom + _card_border_size);
  vertex.add_data3(right - _card_border_size, 0.02, bottom);
  vertex.add_data3(right, 0.02, bottom + _card_border_size);
  vertex.add_data3(right, 0.02, bottom);
  
  texcoord.add_data2(0.0f, 1.0f); //1
  texcoord.add_data2(0.0f, 1.0f - _card_border_uv_portion); //2
  texcoord.add_data2(0.0f + _card_border_uv_portion, 1.0f); //3
  texcoord.add_data2(0.0f + _card_border_uv_portion,
                      1.0f - _card_border_uv_portion); //4
  texcoord.add_data2(1.0f -_card_border_uv_portion, 1.0f); //5
  texcoord.add_data2(1.0f -_card_border_uv_portion,
                      1.0f - _card_border_uv_portion); //6
  texcoord.add_data2(1.0f, 1.0f); //7
  texcoord.add_data2(1.0f, 1.0f - _card_border_uv_portion); //8
  
  texcoord.add_data2(0.0f, _card_border_uv_portion); //9
  texcoord.add_data2(0.0f, 0.0f); //10
  texcoord.add_data2(_card_border_uv_portion, _card_border_uv_portion); //11
  texcoord.add_data2(_card_border_uv_portion, 0.0f); //12

  texcoord.add_data2(1.0f - _card_border_uv_portion, _card_border_uv_portion);//13
  texcoord.add_data2(1.0f - _card_border_uv_portion, 0.0f);//14
  texcoord.add_data2(1.0f, _card_border_uv_portion);//15
  texcoord.add_data2(1.0f, 0.0f);//16
  
  PT(GeomTristrips) card = new GeomTristrips(get_usage_hint());
  
  // tristrip #1
  card->add_consecutive_vertices(0, 8);
  card->close_primitive();
  
  // tristrip #2
  card->add_vertex(1);
  card->add_vertex(8);
  card->add_vertex(3);
  card->add_vertex(10);
  card->add_vertex(5);
  card->add_vertex(12);
  card->add_vertex(7);
  card->add_vertex(14);
  card->close_primitive();
  
  // tristrip #3
  card->add_consecutive_vertices(8, 8);
  card->close_primitive();
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(card);
  
  card_node->add_geom(geom);
  
  return card_node.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::count_geoms
//       Access: Private, Static
//  Description: Recursively counts the number of Geoms at the
//               indicated node and below.  Strictly for reporting
//               this count on output.
////////////////////////////////////////////////////////////////////
int TextNode::
count_geoms(PandaNode *node) {
  int num_geoms = 0;

  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    num_geoms += geom_node->get_num_geoms();
  }

  Children children = node->get_children();
  for (int i = 0; i < children.get_num_children(); ++i) {
    num_geoms += count_geoms(children.get_child(i));
  }

  return num_geoms;
}
