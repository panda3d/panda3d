// Filename: textNode.cxx
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "textNode.h"
#include "textGlyph.h"
#include "stringDecoder.h"
#include "config_text.h"
#include "textAssembler.h"

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
#include "renderState.h"
#include "dcast.h"
#include "bamFile.h"
#include "zStream.h"

#include <stdio.h>

TypeHandle TextNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextNode::
TextNode(const string &name) : PandaNode(name), _assembler(this) {
  _flags = 0;
  _max_rows = 0;

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

  _transform = LMatrix4f::ident_mat();
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
  PandaNode(name), TextProperties(copy),
  _assembler(this) 
{
  _flags = 0;
  _max_rows = 0;

  _frame_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _card_color.set(1.0f, 1.0f, 1.0f, 1.0f);

  _frame_width = 1.0f;

  _frame_ul.set(0.0f, 0.0f);
  _frame_lr.set(0.0f, 0.0f);
  _card_ul.set(0.0f, 0.0f);
  _card_lr.set(0.0f, 0.0f);

  _transform = LMatrix4f::ident_mat();
  _coordinate_system = CS_default;

  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);
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
float TextNode::
calc_width(int character) const {
  TextFont *font = get_font();
  if (font == (TextFont *)NULL) {
    return 0.0f;
  }

  return _assembler.calc_width(character, *this);
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
  TextProperties::write(out, indent_level + 2);

  out << "\n";
  LVecBase3f scale, shear, hpr, trans;
  if (decompose_matrix(_transform, scale, shear, hpr, trans, _coordinate_system)) {
  indent(out, indent_level + 2)
    << "transform is:\n"
    << "  scale: " << scale << "\n"
    << "  shear: " << shear << "\n"
    << "    hpr: " << hpr << "\n"
    << "  trans: " << hpr << "\n";
  } else {
    indent(out, indent_level + 2)
      << "transform is:\n" << _transform;
  }
  indent(out, indent_level + 2)
    << "in coordinate system " << _coordinate_system << "\n";

  out << "\n";
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

  _ul3d.set(0.0f, 0.0f, 0.0f);
  _lr3d.set(0.0f, 0.0f, 0.0f);

  // Now build a new sub-tree for all the text components.
  PT(PandaNode) root = new PandaNode(get_text());

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
  LMatrix4f mat =
    LMatrix4f::convert_mat(CS_zup_right, _coordinate_system) *
    _transform;

  root->set_transform(TransformState::make_mat(mat));

  wstring wtext = get_wtext();

  // Assemble the text.
  bool all_set = _assembler.set_wtext(wtext, *this, _max_rows);
  if (all_set) {
    // No overflow.
    _flags &= ~F_has_overflow;
  } else {
    // Overflow.
    _flags |= F_has_overflow;
  }

  PT(PandaNode) text_root = _assembler.assemble_text();

  // Parent the text in.  We create an intermediate node so we can
  // choose to reinstance the text_root as the shadow, below.
  PT(PandaNode) text = new PandaNode("text");
  root->add_child(text, get_draw_order() + 2);
  text->add_child(text_root);

  // Save the bounding-box information about the text in a form
  // friendly to the user.
  const LVector2f &ul = _assembler.get_ul();
  const LVector2f &lr = _assembler.get_lr();
  _ul3d.set(ul[0], 0.0f, ul[1]);
  _lr3d.set(lr[0], 0.0f, lr[1]);

  _ul3d = _ul3d * _transform;
  _lr3d = _lr3d * _transform;

  // Incidentally, that means we don't need to measure the text now.
  _flags &= ~F_needs_measure;


  // Now deal with the decorations.

  if (has_frame()) {
    PT(PandaNode) frame_root = make_frame();
    root->add_child(frame_root, get_draw_order() + 1);
    frame_root->set_attrib(ColorAttrib::make_flat(get_frame_color()));
    if (get_frame_color()[3] != 1.0f) {
      frame_root->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }

    if (has_bin()) {
      frame_root->set_attrib(CullBinAttrib::make(get_bin(), get_draw_order() + 1));
    }
  }

  if (has_card()) {
    PT(PandaNode) card_root;
    if (has_card_border())
      card_root = make_card_with_border();
    else
      card_root = make_card();
    root->add_child(card_root, get_draw_order());
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
  }

  // Now flatten our hierarchy to get rid of the transforms we put in,
  // applying them to the vertices.

  if (text_flatten) {
    SceneGraphReducer gr;
    gr.apply_attribs(root);
    gr.flatten(root, ~0);
  }

  return root;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Public
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character or any embedded control characters like \1
//               or \3.
////////////////////////////////////////////////////////////////////
float TextNode::
calc_width(const wstring &line) const {
  float width = 0.0f;

  wstring::const_iterator si;
  for (si = line.begin(); si != line.end(); ++si) {
    width += calc_width(*si);
  }

  return width;
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
      const LVecBase4f &s = csa->get_scale();
      if (s != LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f)) {
        LVecBase4f tc = get_text_color();
        tc[0] *= s[0];
        tc[1] *= s[1];
        tc[2] *= s[2];
        tc[3] *= s[3];
        set_text_color(tc);
        LVecBase4f sc = get_shadow_color();
        sc[0] *= s[0];
        sc[1] *= s[1];
        sc[2] *= s[2];
        sc[3] *= s[3];
        set_shadow_color(sc);
        LVecBase4f fc = get_frame_color();
        fc[0] *= s[0];
        fc[1] *= s[1];
        fc[2] *= s[2];
        fc[3] *= s[3];
        set_frame_color(fc);
        LVecBase4f cc = get_card_color();
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
