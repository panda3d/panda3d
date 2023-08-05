/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textNode.cxx
 * @author drose
 * @date 2002-03-13
 */

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

using std::string;

TypeHandle TextNode::_type_handle;

PStatCollector TextNode::_text_generate_pcollector("*:Generate Text");

/**
 *
 */
TextNode::
TextNode(const string &name) : PandaNode(name) {
  set_cull_callback();
  set_renderable();

  if (text_small_caps) {
    TextProperties::set_small_caps(true);
  }
}

/**
 * It's sort of a copy constructor: it copies the indicated TextProperties,
 * without copying a complete TextNode.
 */
TextNode::
TextNode(const string &name, const TextProperties &copy) :
  PandaNode(name), TextProperties(copy)
{
}

/**
 * OK, this is a true copy constructor.
 */
TextNode::
TextNode(const TextNode &copy) :
  PandaNode(copy),
  TextEncoder(copy),
  TextProperties(copy),
  _cycler(copy._cycler)
{
  mark_internal_bounds_stale();
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *TextNode::
make_copy() const {
  return new TextNode(*this);
}

/**
 *
 */
TextNode::
~TextNode() {
}

/**
 * Returns the actual dimensions of the frame around the text.  If the frame
 * was set via set_frame_as_margin(), the result returned by this function
 * reflects the size of the current text; if the frame was set via
 * set_frame_actual(), this returns the values actually set.
 *
 * If the text has no frame at all, this returns the dimensions of the text
 * itself, as if the frame were set with a margin of 0, 0, 0, 0.
 */
LVecBase4 TextNode::
get_frame_actual() const {
  CDLockedReader cdata(_cycler);
  if ((cdata->_flags & (F_has_frame | F_frame_as_margin)) == F_has_frame) {
    return LVecBase4(cdata->_frame_ul[0],
                     cdata->_frame_lr[0],
                     cdata->_frame_lr[1],
                     cdata->_frame_ul[1]);
  }

  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);

    LVecBase4 frame(cdataw->_text_ul[0], cdataw->_text_lr[0], cdataw->_text_lr[1], cdataw->_text_ul[1]);
    if (cdataw->_flags & F_has_frame) {
      frame += LVecBase4(-cdataw->_frame_ul[0], cdataw->_frame_lr[0], -cdataw->_frame_lr[1], cdataw->_frame_ul[1]);
    }
    return frame;
  }
  else {
    LVecBase4 frame(cdata->_text_ul[0], cdata->_text_lr[0], cdata->_text_lr[1], cdata->_text_ul[1]);
    if (cdata->_flags & F_has_frame) {
      frame += LVecBase4(-cdata->_frame_ul[0], cdata->_frame_lr[0], -cdata->_frame_lr[1], cdata->_frame_ul[1]);
    }
    return frame;
  }
}

/**
 * Returns the actual dimensions of the card around the text.  If the card was
 * set via set_card_as_margin(), the result returned by this function reflects
 * the size of the current text; if the card was set via set_card_actual(),
 * this returns the values actually set.
 *
 * If the text has no card at all, this returns the dimensions of the text
 * itself, as if the card were set with a margin of 0, 0, 0, 0.
 */
LVecBase4 TextNode::
get_card_actual() const {
  CDLockedReader cdata(_cycler);
  if ((cdata->_flags & (F_has_card | F_card_as_margin)) == F_has_card) {
    return LVecBase4(cdata->_card_ul[0],
                     cdata->_card_lr[0],
                     cdata->_card_lr[1],
                     cdata->_card_ul[1]);
  }

  if (do_needs_measure(cdata)) {
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_measure(cdataw);

    LVecBase4 card(cdataw->_text_ul[0], cdataw->_text_lr[0], cdataw->_text_lr[1], cdataw->_text_ul[1]);
    if (cdataw->_flags & F_has_card) {
      card += LVecBase4(-cdataw->_card_ul[0], cdataw->_card_lr[0], -cdataw->_card_lr[1], cdataw->_card_ul[1]);
    }
    return card;
  }
  else {
    LVecBase4 card(cdata->_text_ul[0], cdata->_text_lr[0], cdata->_text_lr[1], cdata->_text_ul[1]);
    if (cdata->_flags & F_has_card) {
      card += LVecBase4(-cdata->_card_ul[0], cdata->_card_lr[0], -cdata->_card_lr[1], cdata->_card_ul[1]);
    }
    return card;
  }
}

/**
 * Returns the width of a single character of the font, or 0.0 if the
 * character is not known.  This may be a wide character (greater than 255).
 */
PN_stdfloat TextNode::
calc_width(wchar_t character) const {
  TextFont *font = get_font();
  if (font == nullptr) {
    return 0.0f;
  }

  return TextAssembler::calc_width(character, *this);
}

/**
 * Returns true if the named character exists in the font exactly as named,
 * false otherwise.  Note that because Panda can assemble glyphs together
 * automatically using cheesy accent marks, this is not a reliable indicator
 * of whether a suitable glyph can be rendered for the character.  For that,
 * use has_character() instead.
 *
 * This returns true for whitespace and Unicode whitespace characters (if they
 * exist in the font), but returns false for characters that would render with
 * the "invalid glyph".  It also returns false for characters that would be
 * synthesized within Panda, but see has_character().
 */
bool TextNode::
has_exact_character(wchar_t character) const {
  TextFont *font = get_font();
  if (font == nullptr) {
    return false;
  }

  return TextAssembler::has_exact_character(character, *this);
}

/**
 * Returns true if the named character exists in the font or can be
 * synthesized by Panda, false otherwise.  (Panda can synthesize some accented
 * characters by combining similar-looking glyphs from the font.)
 *
 * This returns true for whitespace and Unicode whitespace characters (if they
 * exist in the font), but returns false for characters that would render with
 * the "invalid glyph".
 */
bool TextNode::
has_character(wchar_t character) const {
  TextFont *font = get_font();
  if (font == nullptr) {
    return false;
  }

  return TextAssembler::has_character(character, *this);
}

/**
 * Returns true if the indicated character represents whitespace in the font,
 * or false if anything visible will be rendered for it.
 *
 * This returns true for whitespace and Unicode whitespace characters (if they
 * exist in the font), and returns false for any other characters, including
 * characters that do not exist in the font (these would be rendered with the
 * "invalid glyph", which is visible).
 *
 * Note that this function can be reliably used to identify Unicode whitespace
 * characters only if the font has all of the whitespace characters defined.
 * It will return false for any character not in the font, even if it is an
 * official Unicode whitespace character.
 */
bool TextNode::
is_whitespace(wchar_t character) const {
  TextFont *font = get_font();
  if (font == nullptr) {
    return false;
  }

  return TextAssembler::is_whitespace(character, *this);
}

/**
 * Returns the width of a line of text of arbitrary characters.  The line
 * should not include the newline character or any embedded control characters
 * like \1 or \3.
 */
PN_stdfloat TextNode::
calc_width(const std::wstring &line) const {
  TextFont *font = get_font();
  if (font == nullptr) {
    return 0.0f;
  }

  PN_stdfloat width = 0.0f;

  std::wstring::const_iterator si;
  for (si = line.begin(); si != line.end(); ++si) {
    width += TextAssembler::calc_width(*si, *this);
  }

  return width;
}

/**
 *
 */
void TextNode::
output(std::ostream &out) const {
  PandaNode::output(out);

  PT(PandaNode) internal_geom = do_get_internal_geom();
  int geom_count = 0;
  if (internal_geom != nullptr) {
    geom_count = count_geoms(internal_geom);
  }

  out << " (" << geom_count << " geoms)";
}

/**
 *
 */
void TextNode::
write(std::ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);

  CDReader cdata(_cycler);
  TextProperties::write(out, indent_level + 2);
  indent(out, indent_level + 2)
    << "transform is: " << *TransformState::make_mat(cdata->_transform) << "\n";
  indent(out, indent_level + 2)
    << "in coordinate system " << cdata->_coordinate_system << "\n";
  indent(out, indent_level + 2)
    << "text is " << cdata->_text << "\n";
}

/**
 * Returns the actual node that is used internally to render the text, if the
 * TextNode is parented within the scene graph.
 *
 * In general, you should not call this method.  Call generate() instead if
 * you want to get a handle to geometry that represents the text.  This method
 * is provided as a debugging aid only.
 */
PT(PandaNode) TextNode::
get_internal_geom() const {
  // Output a nuisance warning to discourage the naive from calling this
  // method accidentally.
  text_cat.info()
    << "TextNode::get_internal_geom() called.\n";
  return do_get_internal_geom();
}

/**
 * Called whenever the text has been changed.
 */
void TextNode::
text_changed() {
  // Copy the text to this class, since TextEncoder doesn't store it in a
  // pipeline-cycled manner.
  CDWriter cdata(_cycler, true);
  cdata->_text = TextEncoder::get_text();
  cdata->_wtext = TextEncoder::get_wtext();
  invalidate_with_measure(cdata);
}

/**
 * Returns the union of all attributes from SceneGraphReducer::AttribTypes
 * that may not safely be applied to the vertices of this node.  If this is
 * nonzero, these attributes must be dropped at this node as a state change.
 *
 * This is a generalization of safe_to_transform().
 */
int TextNode::
get_unsafe_to_apply_attribs() const {
  // We have no way to apply these kinds of attributes to our TextNode, so
  // insist they get dropped into the PandaNode's basic state.
  return
    SceneGraphReducer::TT_tex_matrix |
    SceneGraphReducer::TT_other;
}

/**
 * Applies whatever attributes are specified in the AccumulatedAttribs object
 * (and by the attrib_types bitmask) to the vertices on this node, if
 * appropriate.  If this node uses geom arrays like a GeomNode, the supplied
 * GeomTransformer may be used to unify shared arrays across multiple
 * different nodes.
 *
 * This is a generalization of xform().
 */
void TextNode::
apply_attribs_to_vertices(const AccumulatedAttribs &attribs, int attrib_types,
                          GeomTransformer &transformer) {

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);

    if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
      const LMatrix4 &mat = attribs._transform->get_mat();
      cdata->_transform *= mat;

      if ((cdata->_flags & F_needs_measure) == 0) {
        // If we already have a measure, transform it too.  We don't need to
        // invalidate the 2-d parts, since that's not affected by the transform
        // anyway.
        cdata->_ul3d = cdata->_ul3d * mat;
        cdata->_lr3d = cdata->_lr3d * mat;
      }
    }
    if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
      if (attribs._color != nullptr) {
        const ColorAttrib *ca = DCAST(ColorAttrib, attribs._color);
        if (ca->get_color_type() == ColorAttrib::T_flat) {
          const LColor &c = ca->get_color();
          TextProperties::set_text_color(c);
          TextProperties::set_shadow_color(c);
          cdata->_frame_color = c;
          cdata->_card_color = c;
          invalidate_no_measure(cdata);
        }
      }
    }
    if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
      if (attribs._color_scale != nullptr) {
        const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attribs._color_scale);
        const LVecBase4 &s = csa->get_scale();
        if (s != LVecBase4(1.0f, 1.0f, 1.0f, 1.0f)) {
          LVecBase4 tc = get_text_color();
          tc.componentwise_mult(s);
          TextProperties::set_text_color(tc);

          LVecBase4 sc = get_shadow_color();
          sc.componentwise_mult(s);
          TextProperties::set_shadow_color(sc);

          cdata->_frame_color.componentwise_mult(s);
          cdata->_card_color.componentwise_mult(s);

          invalidate_no_measure(cdata);
        }
      }
    }

    // Now propagate the attributes down to our already-generated geometry, if
    // we have any.
    if ((cdata->_flags & F_needs_rebuild) == 0 && cdata->_internal_geom != nullptr) {
      SceneGraphReducer gr;
      gr.apply_attribs(cdata->_internal_geom, attribs, attrib_types, transformer);
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
}

/**
 * This is used to support NodePath::calc_tight_bounds().  It is not intended
 * to be called directly, and it has nothing to do with the normal Panda
 * bounding-volume computation.
 *
 * If the node contains any geometry, this updates min_point and max_point to
 * enclose its bounding box.  found_any is to be set true if the node has any
 * geometry at all, or left alone if it has none.  This method may be called
 * over several nodes, so it may enter with min_point, max_point, and
 * found_any already set.
 */
CPT(TransformState) TextNode::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  CPT(TransformState) next_transform =
    PandaNode::calc_tight_bounds(min_point, max_point, found_any, transform,
                                 current_thread);

  PT(PandaNode) geom = do_get_internal_geom();
  if (geom != nullptr) {
    geom->calc_tight_bounds(min_point, max_point,
                            found_any, next_transform, current_thread);
  }

  return next_transform;
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool TextNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {

  PT(PandaNode) internal_geom = do_get_internal_geom();
  if (internal_geom != nullptr) {
    // Render the text with this node.
    trav->traverse_down(data, internal_geom);
  }

  // Now continue to render everything else below this node.
  return true;
}

/**
 * Called when needed to recompute the node's _internal_bound object.  Nodes
 * that contain anything of substance should redefine this to do the right
 * thing.
 */
void TextNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  // First, get ourselves a fresh, empty bounding volume.
  PT(BoundingVolume) bound = new BoundingSphere;

  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now enclose the bounding box around the text.  We can do this without
  // actually generating the text, if we have at least measured it.
  LPoint3 vertices[8];
  {
    LPoint3 ul3d, lr3d;
    CDLockedReader cdata(_cycler);
    if (do_needs_measure(cdata)) {
      CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
      ((TextNode *)this)->do_measure(cdataw);

      ul3d = cdataw->_ul3d;
      lr3d = cdataw->_lr3d;
    } else {
      ul3d = cdata->_ul3d;
      lr3d = cdata->_lr3d;
    }

    vertices[0].set(ul3d[0], ul3d[1], ul3d[2]);
    vertices[1].set(ul3d[0], ul3d[1], lr3d[2]);
    vertices[2].set(ul3d[0], lr3d[1], ul3d[2]);
    vertices[3].set(ul3d[0], lr3d[1], lr3d[2]);
    vertices[4].set(lr3d[0], ul3d[1], ul3d[2]);
    vertices[5].set(lr3d[0], ul3d[1], lr3d[2]);
    vertices[6].set(lr3d[0], lr3d[1], ul3d[2]);
    vertices[7].set(lr3d[0], lr3d[1], lr3d[2]);
  }

  gbv->around(vertices, vertices + 8);

  internal_bounds = bound;
  internal_vertices = 0;  // TODO: estimate this better.
}

/**
 * The recursive implementation of prepare_scene(). Don't call this directly;
 * call PandaNode::prepare_scene() or NodePath::prepare_scene() instead.
 */
void TextNode::
r_prepare_scene(GraphicsStateGuardianBase *gsg, const RenderState *node_state,
                GeomTransformer &transformer, Thread *current_thread) {

  PT(PandaNode) child = do_get_internal_geom();
  if (child != nullptr) {
    CPT(RenderState) child_state = node_state->compose(child->get_state());
    child->r_prepare_scene(gsg, child_state, transformer, current_thread);
  }

  PandaNode::r_prepare_scene(gsg, node_state, transformer, current_thread);
}

/**
 * Removes any existing children of the TextNode, and adds the newly generated
 * text instead.
 */
void TextNode::
do_rebuild(CData *cdata) {
  cdata->_flags &= ~(F_needs_rebuild | F_needs_measure);
  cdata->_internal_geom = do_generate(cdata);
}

/**
 * Can be called in lieu of do_rebuild() to measure the text and set up the
 * bounding boxes properly without actually assembling it.
 */
void TextNode::
do_measure(CData *cdata) {
  // We no longer make this a special case.
  do_rebuild(cdata);
}

/**
 * Generates the text, according to the parameters indicated within the
 * TextNode, and returns a Node that may be parented within the tree to
 * represent it.
 */
PT(PandaNode) TextNode::
do_generate(CData *cdata) {
  PStatTimer timer(_text_generate_pcollector);
  if (text_cat.is_debug()) {
    text_cat.debug()
      << "Rebuilding " << get_type() << " " << get_name()
      << " with '" << cdata->_text << "'\n";
  }

  // The strategy here will be to assemble together a bunch of letters,
  // instanced from the letter hierarchy of font_def, into our own little
  // hierarchy.

  // There will be one root over the whole text block, that contains the
  // transform passed in.  Under this root there will be another node for each
  // row, that moves the row into the right place horizontally and vertically,
  // and for each row, there is another node for each character.

  cdata->_ul3d.set(0.0f, 0.0f, 0.0f);
  cdata->_lr3d.set(0.0f, 0.0f, 0.0f);

  // Now build a new sub-tree for all the text components.
  string name = cdata->_text;
  size_t newline = name.find('\n');
  if (newline != string::npos) {
    name = name.substr(0, newline);
  }
  PT(PandaNode) root = new PandaNode(name);

  if (cdata->_wtext.empty()) {
    return root;
  }

  TextFont *font = get_font();
  if (font == nullptr) {
    return root;
  }

  // Compute the overall text transform matrix.  We build the text in a Z-up
  // coordinate system and then convert it to whatever the user asked for.
  LMatrix4 mat =
    LMatrix4::convert_mat(CS_zup_right, cdata->_coordinate_system) *
    cdata->_transform;

  CPT(TransformState) transform = TransformState::make_mat(mat);
  root->set_transform(transform);

  // Assemble the text.
  TextAssembler assembler(this);
  assembler.set_properties(*this);
  assembler.set_max_rows(cdata->_max_rows);
  assembler.set_usage_hint(cdata->_usage_hint);
  assembler.set_dynamic_merge((cdata->_flatten_flags & FF_dynamic_merge) != 0);
  bool all_set = assembler.set_wtext(cdata->_wtext);
  if (all_set) {
    // No overflow.
    cdata->_flags &= ~F_has_overflow;
  } else {
    // Overflow.
    cdata->_flags |= F_has_overflow;
  }

  PT(PandaNode) text_root = assembler.assemble_text();
  cdata->_text_ul = assembler.get_ul();
  cdata->_text_lr = assembler.get_lr();
  cdata->_num_rows = assembler.get_num_rows();
  cdata->_wordwrapped_wtext = assembler.get_wordwrapped_wtext();

  // Parent the text in.
  PT(PandaNode) text = new PandaNode("text");
  root->add_child(text, get_draw_order() + 2);
  text->add_child(text_root);

  // Save the bounding-box information about the text in a form friendly to
  // the user.
  const LVector2 &ul = assembler.get_ul();
  const LVector2 &lr = assembler.get_lr();
  cdata->_ul3d.set(ul[0], 0.0f, ul[1]);
  cdata->_lr3d.set(lr[0], 0.0f, lr[1]);

  cdata->_ul3d = cdata->_ul3d * cdata->_transform;
  cdata->_lr3d = cdata->_lr3d * cdata->_transform;

  // Incidentally, that means we don't need to measure the text now.
  cdata->_flags &= ~F_needs_measure;

  // Now flatten our hierarchy to get rid of the transforms we put in,
  // applying them to the vertices.

  NodePath root_np(root);
  if (cdata->_flatten_flags & FF_strong) {
    root_np.flatten_strong();
  }
  else if (cdata->_flatten_flags & FF_medium) {
    root_np.flatten_medium();
  }
  else if (cdata->_flatten_flags & FF_light) {
    root_np.flatten_light();
  }

  // Now deal with the decorations.

  if (cdata->_flags & F_has_card) {
    PT(PandaNode) card_root;
    if (cdata->_flags & F_has_card_border) {
      card_root = do_make_card_with_border(cdata);
    } else {
      card_root = do_make_card(cdata);
    }
    card_root->set_transform(transform);
    card_root->set_attrib(ColorAttrib::make_flat(cdata->_card_color));
    if (cdata->_card_color[3] != 1.0f) {
      card_root->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }
    if (cdata->_flags & F_has_card_texture) {
      card_root->set_attrib(TextureAttrib::make(cdata->_card_texture));
    }

    if (has_bin()) {
      card_root->set_attrib(CullBinAttrib::make(get_bin(), get_draw_order()));
    }

    // We always apply attribs down to the card vertices.
    SceneGraphReducer gr;
    gr.apply_attribs(card_root);

    // In order to decal the text onto the card, the card must become the
    // parent of the text.
    card_root->add_child(root);
    root = card_root;

    if (cdata->_flags & F_card_decal) {
      card_root->set_effect(DecalEffect::make());
    }
  }

  if (cdata->_flags & F_has_frame) {
    PT(PandaNode) frame_root = do_make_frame(cdata);
    frame_root->set_transform(transform);
    root->add_child(frame_root, get_draw_order() + 1);
    frame_root->set_attrib(ColorAttrib::make_flat(cdata->_frame_color));
    if (cdata->_frame_color[3] != 1.0f) {
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

/**
 * Returns the actual node that is used internally to render the text, if the
 * TextNode is parented within the scene graph.
 */
PT(PandaNode) TextNode::
do_get_internal_geom() const {
  CDLockedReader cdata(_cycler);
  if ((cdata->_flags & F_needs_rebuild) != 0) {
    // Propagate the generated text upstream if the upstream stages have no
    // changes to the text.
    CDWriter cdataw(((TextNode *)this)->_cycler, cdata, false);
    ((TextNode *)this)->do_rebuild(cdataw);

    return cdataw->_internal_geom;
  }
  else {
    return cdata->_internal_geom;
  }
}

/**
 * Creates a frame around the text.
 */
PT(PandaNode) TextNode::
do_make_frame(const CData *cdata) {
  nassertr((cdata->_flags & F_needs_measure) == 0, nullptr);

  PT(GeomNode) frame_node = new GeomNode("frame");

  PN_stdfloat left = cdata->_frame_ul[0];
  PN_stdfloat right = cdata->_frame_lr[0];
  PN_stdfloat bottom = cdata->_frame_lr[1];
  PN_stdfloat top = cdata->_frame_ul[1];

  if (cdata->_flags & F_frame_as_margin) {
    left = cdata->_text_ul[0] - left;
    right = cdata->_text_lr[0] + right;
    bottom = cdata->_text_lr[1] - bottom;
    top = cdata->_text_ul[1] + top;
  }

  CPT(RenderAttrib) thick = RenderModeAttrib::make(RenderModeAttrib::M_unchanged, cdata->_frame_width);
  CPT(RenderState) state = RenderState::make(thick);

  PT(GeomVertexData) vdata = new GeomVertexData
    ("text", GeomVertexFormat::get_v3(), cdata->_usage_hint);
  vdata->unclean_set_num_rows(4);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  vertex.set_data3(left, 0.0f, top);
  vertex.set_data3(left, 0.0f, bottom);
  vertex.set_data3(right, 0.0f, bottom);
  vertex.set_data3(right, 0.0f, top);

  PT(GeomLinestrips) frame = new GeomLinestrips(cdata->_usage_hint);
  frame->add_consecutive_vertices(0, 4);
  frame->add_vertex(0);
  frame->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(frame);
  frame_node->add_geom(geom, state);

  if (cdata->_flags & F_frame_corners) {
    PT(GeomPoints) corners = new GeomPoints(cdata->_usage_hint);
    corners->add_consecutive_vertices(0, 4);
    PT(Geom) geom2 = new Geom(vdata);
    geom2->add_primitive(corners);
    frame_node->add_geom(geom2, state);
  }

  return frame_node;
}

/**
 * Creates a card behind the text.
 */
PT(PandaNode) TextNode::
do_make_card(const CData *cdata) {
  nassertr((cdata->_flags & F_needs_measure) == 0, nullptr);

  PT(GeomNode) card_node = new GeomNode("card");

  PN_stdfloat left = cdata->_card_ul[0];
  PN_stdfloat right = cdata->_card_lr[0];
  PN_stdfloat bottom = cdata->_card_lr[1];
  PN_stdfloat top = cdata->_card_ul[1];

  if (cdata->_flags & F_card_as_margin) {
    left = cdata->_text_ul[0] - left;
    right = cdata->_text_lr[0] + right;
    bottom = cdata->_text_lr[1] - bottom;
    top = cdata->_text_ul[1] + top;
  }

  PT(GeomVertexData) vdata = new GeomVertexData
    ("text", GeomVertexFormat::get_v3t2(), cdata->_usage_hint);
  vdata->unclean_set_num_rows(4);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  vertex.set_data3(left, 0.0f, top);
  vertex.set_data3(left, 0.0f, bottom);
  vertex.set_data3(right, 0.0f, top);
  vertex.set_data3(right, 0.0f, bottom);

  texcoord.set_data2(0.0f, 1.0f);
  texcoord.set_data2(0.0f, 0.0f);
  texcoord.set_data2(1.0f, 1.0f);
  texcoord.set_data2(1.0f, 0.0f);

  PT(GeomTristrips) card = new GeomTristrips(cdata->_usage_hint);
  card->add_consecutive_vertices(0, 4);
  card->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(card);

  card_node->add_geom(geom);

  return card_node;
}


/**
 * Creates a card behind the text with a specified border for button edge or
 * what have you.
 */
PT(PandaNode) TextNode::
do_make_card_with_border(const CData *cdata) {
  nassertr((cdata->_flags & F_needs_measure) == 0, nullptr);

  PT(GeomNode) card_node = new GeomNode("card");

  PN_stdfloat left = cdata->_card_ul[0];
  PN_stdfloat right = cdata->_card_lr[0];
  PN_stdfloat bottom = cdata->_card_lr[1];
  PN_stdfloat top = cdata->_card_ul[1];

  if (cdata->_flags & F_card_as_margin) {
    left = cdata->_text_ul[0] - left;
    right = cdata->_text_lr[0] + right;
    bottom = cdata->_text_lr[1] - bottom;
    top = cdata->_text_ul[1] + top;
  }

  PN_stdfloat border_size = cdata->_card_border_size;
  PN_stdfloat border_uv_portion = cdata->_card_border_uv_portion;

  // we now create three tri-strips instead of one
  // with vertices arranged as follows:
  //
  //  1 3            5 7  - one
  //  2 4            6 8  /  \ two
  //  9 11          13 15 \  /
  // 10 12          14 16 - three
  //

  PT(GeomVertexData) vdata = new GeomVertexData
    ("text", GeomVertexFormat::get_v3t2(), cdata->_usage_hint);
  vdata->unclean_set_num_rows(16);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  // verts 1,2,3,4
  vertex.set_data3(left, 0.02, top);
  vertex.set_data3(left, 0.02, top - border_size);
  vertex.set_data3(left + border_size, 0.02, top);
  vertex.set_data3(left + border_size, 0.02,
                    top - border_size);
  // verts 5,6,7,8
  vertex.set_data3(right - border_size, 0.02, top);
  vertex.set_data3(right - border_size, 0.02,
                    top - border_size);
  vertex.set_data3(right, 0.02, top);
  vertex.set_data3(right, 0.02, top - border_size);
  // verts 9,10,11,12
  vertex.set_data3(left, 0.02, bottom + border_size);
  vertex.set_data3(left, 0.02, bottom);
  vertex.set_data3(left + border_size, 0.02,
                    bottom + border_size);
  vertex.set_data3(left + border_size, 0.02, bottom);
  // verts 13,14,15,16
  vertex.set_data3(right - border_size, 0.02,
                    bottom + border_size);
  vertex.set_data3(right - border_size, 0.02, bottom);
  vertex.set_data3(right, 0.02, bottom + border_size);
  vertex.set_data3(right, 0.02, bottom);

  texcoord.set_data2(0.0f, 1.0f); //1
  texcoord.set_data2(0.0f, 1.0f - border_uv_portion); //2
  texcoord.set_data2(0.0f + border_uv_portion, 1.0f); //3
  texcoord.set_data2(0.0f + border_uv_portion,
                      1.0f - border_uv_portion); //4
  texcoord.set_data2(1.0f -border_uv_portion, 1.0f); //5
  texcoord.set_data2(1.0f -border_uv_portion,
                      1.0f - border_uv_portion); //6
  texcoord.set_data2(1.0f, 1.0f); //7
  texcoord.set_data2(1.0f, 1.0f - border_uv_portion); //8

  texcoord.set_data2(0.0f, border_uv_portion); //9
  texcoord.set_data2(0.0f, 0.0f); //10
  texcoord.set_data2(border_uv_portion, border_uv_portion); //11
  texcoord.set_data2(border_uv_portion, 0.0f); //12

  texcoord.set_data2(1.0f - border_uv_portion, border_uv_portion);//13
  texcoord.set_data2(1.0f - border_uv_portion, 0.0f);//14
  texcoord.set_data2(1.0f, border_uv_portion);//15
  texcoord.set_data2(1.0f, 0.0f);//16

  PT(GeomTristrips) card = new GeomTristrips(cdata->_usage_hint);
  card->reserve_num_vertices(24);

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

  return card_node;
}

/**
 * Recursively counts the number of Geoms at the indicated node and below.
 * Strictly for reporting this count on output.
 */
int TextNode::
count_geoms(PandaNode *node) {
  int num_geoms = 0;

  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    num_geoms += geom_node->get_num_geoms();
  }

  Children children = node->get_children();
  for (size_t i = 0; i < children.get_num_children(); ++i) {
    num_geoms += count_geoms(children.get_child(i));
  }

  return num_geoms;
}

/**
 *
 */
TextNode::CData::
CData() {
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

/**
 *
 */
TextNode::CData::
CData(const CData &copy) :
  _text(copy._text),
  _wtext(copy._wtext),
  _internal_geom(copy._internal_geom),
  _card_texture(copy._card_texture),
  _frame_color(copy._frame_color),
  _card_color(copy._card_color),
  _flags(copy._flags | F_needs_rebuild | F_needs_measure),
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
}

/**
 *
 */
CycleData *TextNode::CData::
make_copy() const {
  return new CData(*this);
}
