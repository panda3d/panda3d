// Filename: textNode.cxx
// Created by:  drose (12May99)
// 
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "textNode.h"
#include "config_text.h"

#include <compose_matrix.h>
#include <transformTransition.h>
#include <colorTransition.h>
#include <geom.h>
#include <geomprimitives.h>
#include <renderRelation.h>
#include <billboardTransition.h>
#include <notify.h>
#include <sceneGraphReducer.h>

#include <stdio.h>
#include <ctype.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle TextNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: isblank
//  Description: An internal function, similar to isspace(), except it
//               does not consider newlines to be whitespace.
////////////////////////////////////////////////////////////////////
INLINE bool
isblank(char ch) {
  return (ch == ' ' || ch == '\t');
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::CharDef::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TextNode::CharDef::
CharDef(Geom *geom, float width, const AllTransitionsWrapper &trans) : 
  _geom(geom), _width(width), _trans(trans) { }

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextNode::
TextNode(const string &name) : NamedNode(name) {
  _font_height = 1.0;
  _slant = 0.0;

  _flags = 0;
  _align = TM_ALIGN_LEFT;
  _wordwrap_width = 1.0;

  _text_color.set(1.0, 1.0, 1.0, 1.0);
  _frame_color.set(1.0, 1.0, 1.0, 1.0);
  _card_color.set(1.0, 1.0, 1.0, 1.0);
  _shadow_color.set(1.0, 1.0, 1.0, 1.0);
  
  _frame_width = 1.0;

  _frame_ul.set(0.0, 0.0);
  _frame_lr.set(0.0, 0.0);
  _card_ul.set(0.0, 0.0);
  _card_lr.set(0.0, 0.0);
  _shadow_offset.set(0.0, 0.0);

  _draw_order = 1;

  _transform = LMatrix4f::ident_mat();
  _coordinate_system = CS_default;

  _ul2d.set(0.0, 0.0);
  _lr2d.set(0.0, 0.0);
  _ul3d.set(0.0, 0.0, 0.0);
  _lr3d.set(0.0, 0.0, 0.0);
  _num_rows = 0;

  _freeze_level = 0;
  _needs_rebuild = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextNode::
~TextNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Public
//  Description: Returns the width of a single character of the font,
//               or 0.0 if the character is not known.
////////////////////////////////////////////////////////////////////
float TextNode::
calc_width(char ch) const {
  if (ch == ' ') {
    // A space is a special case.
    return 0.25;
  }

  CharDefs::const_iterator cdi = _defs.find(ch);
  if (cdi == _defs.end()) {
    // Unknown character.
    return 0.0;
  }

  return (*cdi).second._width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::calc_width
//       Access: Public
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character.
////////////////////////////////////////////////////////////////////
float TextNode::
calc_width(const string &line) const {
  float width = 0.0;

  string::const_iterator si;
  for (si = line.begin(); si != line.end(); ++si) {
    width += calc_width(*si);
  }

  return width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::wordwrap_to
//       Access: Public
//  Description: Inserts newlines into the given text at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).  Returns the new string.
////////////////////////////////////////////////////////////////////
string TextNode::
wordwrap_to(const string &text, float wordwrap_width) const {
  string output_text;

  size_t p = 0;

  // Preserve any initial whitespace and newlines.
  while (p < text.length() && isspace(text[p])) {
    output_text += text[p];
    p++;
  }
  bool first_line = true;

  while (p < text.length()) {
    nassertr(!isspace(text[p]), "");

    // Scan the next n characters, until the end of the string or an
    // embedded newline character, or we exceed wordwrap_width.

    size_t q = p;
    bool any_spaces = false;

    float width = 0.0;
    while (q < text.length() && text[q] != '\n' && width <= wordwrap_width) {
      if (isspace(text[q])) {
	any_spaces = true;
      }

      width += calc_width(text[q]);
      q++;
    }

    if (q < text.length() && any_spaces) {
      // If we stopped because we exceeded the wordwrap width, then
      // back up to the end of the last complete word.

      while (q > p && !isspace(text[q])) {
	q--;
      }
    }

    // Skip additional whitespace between the lines.
    size_t next_start = q;
    while (next_start < text.length() && isblank(text[next_start])) {
      next_start++;
    }

    // Trim off any more blanks on the end.
    while (q > p && isspace(text[q - 1])) {
      q--;
    }

    if (next_start == p) {
      // No characters got in at all.  This could only happen if the
      // wordwrap width is narrower than a single character.
      q++;
      next_start++;
      while (next_start < text.length() && isblank(text[next_start])) {
	next_start++;
      }
    }

    if (!first_line) {
      output_text += '\n';
    }
    first_line = false;
    output_text += text.substr(p, q - p);

    // Now prepare to wrap the next line.

    if (next_start < text.length() && text[next_start] == '\n') {
      // Skip a single embedded newline.
      next_start++;
    }
    p = next_start;

    // Preserve any initial whitespace and newlines.
    while (p < text.length() && isspace(text[p])) {
      output_text += text[p];
      p++;
    }
  }

  return output_text;
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::print
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
void TextNode::
print() const {
  write(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::write
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
void TextNode::
write(ostream &out) const {
  out << "TextNode\n"
      << _defs.size() << " characters available in font.\n"
      << "line height is " << _font_height << " units.\n";
  if (has_text_color()) {
    out << "text color is " << _text_color << "\n";
  } else {
    out << "text color is unchanged from source\n";
  }
  out << "alignment is ";
  switch (_align) {
  case TM_ALIGN_LEFT:
    out << "TM_ALIGN_LEFT\n";
    break;
    
  case TM_ALIGN_RIGHT:
    out << "TM_ALIGN_RIGHT\n";
    break;

  case TM_ALIGN_CENTER:
    out << "TM_ALIGN_CENTER\n";
    break;
  }

  if (has_wordwrap()) {
    out << "Word-wrapping at " << _wordwrap_width << " units.\n";
  }

  if (has_frame()) {
    out << "frame of color " << _frame_color << " at " 
	<< get_frame_as_set() << " line width " << _frame_width << "\n";
    if (get_frame_corners()) {
      out << "frame corners are enabled\n";
    }
    if (is_frame_as_margin()) {
      out << "frame coordinates are specified as margin; actual frame is:\n"
	  << get_frame_actual() << "\n";
    } else {
      out << "frame coordinates are actual\n";
    }
  }
  if (has_card()) {
    out << "card of color " << _card_color << " at " 
	<< get_card_as_set() << "\n";
    if (is_card_as_margin()) {
      out << "card coordinates are specified as margin; actual card is:\n"
	  << get_card_actual() << "\n";
    } else {
      out << "card coordinates are actual\n";
    }
  }
  if (has_shadow()) {
    out << "shadow of color " << _shadow_color << " at " 
	<< _shadow_offset << "\n";
  }
  out << "draw order is " << _draw_order << ", "
      << _draw_order + 1 << ", " << _draw_order + 2 << "\n";

  LVecBase3f scale, hpr, trans;
  if (decompose_matrix(_transform, scale, hpr, trans, _coordinate_system)) {
    out << "transform is:\n"
	<< "  scale: " << scale << "\n"
	<< "    hpr: " << hpr << "\n"
	<< "  trans: " << hpr << "\n";
  } else {
    out << "transform is:\n" << _transform;
  }
  out << "in coordinate system " << _coordinate_system << "\n";

  out << "\ntext is " << _text << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::do_rebuild
//       Access: Private
//  Description: Removes any geometry previously defined in the geode,
//               and fills it with new geometry that represents the
//               current text string and all its accoutrements.
////////////////////////////////////////////////////////////////////
void TextNode::
do_rebuild() {
  _needs_rebuild = false;

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

  // First delete the arc below self (the TextNode).  This will eliminate
  // the entire sub-tree result of a prior call to make_text (if one
  // exists).

  if (_root_arc != (RenderRelation *)NULL) {
    remove_arc(_root_arc);
    _root_arc.clear();
  }

  _root.clear();
  _text_root.clear();
  _frame_root.clear();
  _card_root.clear();

  _ul2d.set(0.0, 0.0);
  _lr2d.set(0.0, 0.0);
  _ul3d.set(0.0, 0.0, 0.0);
  _lr3d.set(0.0, 0.0, 0.0);
  _num_rows = 0;

  if (_text.empty() || _defs.empty()) {
    return;
  }

  // Now build a new sub-tree for all the text components.
  _root = new NamedNode(_text);
  _root_arc = new RenderRelation(this, _root);

  // Compute the overall text transform matrix.  We build the text in
  // a Z-up coordinate system and then convert it to whatever the user
  // asked for.
  LMatrix4f mat = 
    LMatrix4f::convert_mat(CS_zup_right, _coordinate_system) * 
    _transform;

  _root_arc->set_transition(new TransformTransition(mat));

  if (get_billboard()) {
    _root_arc->set_transition(new BillboardTransition(BillboardTransition::axis(_coordinate_system)));
  }

  string text = _text;
  if (has_wordwrap()) {
    text = wordwrap_to(text, _wordwrap_width);
  }

  // Assemble the text.
  LVector2f ul, lr;
  int num_rows = 0;
  _text_root = assemble_text(text.c_str(), ul, lr, num_rows);
  RenderRelation *text_arc = 
    new RenderRelation(_root, _text_root, _draw_order + 2);

  if (has_text_color()) {
    text_arc->set_transition(new ColorTransition(_text_color));
    if (_text_color[3] != 1.0) {
      text_arc->set_transition
	(new TransparencyTransition(TransparencyProperty::M_alpha));
    }
  }

  // Save the bounding-box information about the text in a form
  // friendly to the user.
  _num_rows = num_rows;
  _ul2d = ul;
  _lr2d = lr;
  _ul3d.set(ul[0], 0.0, ul[1]);
  _lr3d.set(lr[0], 0.0, lr[1]);

  _ul3d = _ul3d * _transform;
  _lr3d = _lr3d * _transform;


  // Now deal with all the decorations.
  
  if (has_shadow()) {
    // Make a shadow by instancing the text behind itself.
    LMatrix4f offset =
      LMatrix4f::translate_mat(_shadow_offset[0], 0.01, -_shadow_offset[1]);
    RenderRelation *shadow_arc = 
      new RenderRelation(_root, _text_root, _draw_order + 1);
    shadow_arc->set_transition(new TransformTransition(offset));
    shadow_arc->set_transition(new ColorTransition(_shadow_color));

    if (_shadow_color[3] != 1.0) {
      shadow_arc->set_transition
	(new TransparencyTransition(TransparencyProperty::M_alpha));
    }
  }

  if (has_frame()) {
    _frame_root = make_frame();
    RenderRelation *frame_arc =
      new RenderRelation(_root, _frame_root, _draw_order + 1);
    frame_arc->set_transition(new ColorTransition(_frame_color));
    if (_frame_color[3] != 1.0) {
      frame_arc->set_transition
	(new TransparencyTransition(TransparencyProperty::M_alpha));
    }
  }

  if (has_card()) {
    if (has_card_border())
      _card_root = make_card_with_border();
    else
      _card_root = make_card();
    RenderRelation *card_arc = 
      new RenderRelation(_root, _card_root, _draw_order);
    card_arc->set_transition(new ColorTransition(_card_color));
    if (_card_color[3] != 1.0) {
      card_arc->set_transition
	(new TransparencyTransition(TransparencyProperty::M_alpha));
    }
    if (has_card_texture()) {
      card_arc->set_transition(new TextureTransition(_card_texture));
    }
  }
  
  // Now flatten our hierarchy to get rid of the transforms we put in,
  // applying them to the vertices.

  if (flatten_text) {
    SceneGraphReducer gr(RenderRelation::get_class_type());
    gr.apply_transitions(_root);
    gr.flatten(_root, true);
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
  _ul2d.set(0.0, 0.0);
  _lr2d.set(0.0, 0.0);
  _ul3d.set(0.0, 0.0, 0.0);
  _lr3d.set(0.0, 0.0, 0.0);
  _num_rows = 0;

  if (_text.empty() || _defs.empty()) {
    return;
  }

  string text = _text;
  if (has_wordwrap()) {
    text = wordwrap_to(text, _wordwrap_width);
  }

  LVector2f ul, lr;
  int num_rows;
  measure_text(text.c_str(), ul, lr, num_rows);

  _num_rows = num_rows;
  _ul2d = ul;
  _lr2d = lr;
  _ul3d.set(ul[0], 0.0, ul[1]);
  _lr3d.set(lr[0], 0.0, lr[1]);

  _ul3d = _ul3d * _transform;
  _lr3d = _lr3d * _transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::find_character_gsets
//       Access: Private
//  Description: Given that 'root' is a Node containing at least a
//               polygon and a point which define the character's
//               appearance and kern position, respectively,
//               recursively walk the hierarchy and root and locate
//               those two Geoms.
////////////////////////////////////////////////////////////////////
bool TextNode::
find_character_gsets(Node *root, Geom *&ch, GeomPoint *&dot,
		     AllTransitionsWrapper &trans) {
  if (root->is_of_type(GeomNode::get_class_type())) {
    GeomNode *geode = (GeomNode *)root;
    
    bool found = false;
    for (int i = 0; i < geode->get_num_geoms(); i++) {
      dDrawable *geom = geode->get_geom(i);
      if (geom->is_of_type(GeomPoint::get_class_type())) {
	dot = DCAST(GeomPoint, geom);
	
      } else if (geom->is_of_type(Geom::get_class_type())) {
	ch = DCAST(Geom, geom);
	found = true;
      }
    }
    return found;
    
  } else {
    DownRelations::const_iterator dri;
    dri = root->_children.find(RenderRelation::get_class_type());
    if (dri != root->_children.end()) {
      const DownRelationPointers &drp = (*dri).second;
      DownRelationPointers::const_iterator drpi;
      for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
        if (find_character_gsets((*drpi)->get_child(), ch, dot, trans)) {
	  trans.extract_from(*drpi);
	}
      }
    }
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::find_characters
//       Access: Private
//  Description: Walk the hierarchy beginning at the indicated root
//               and locate any nodes whose names are just integers.
//               These are taken to be characters, and their
//               definitions and kern informations are retrieved.
////////////////////////////////////////////////////////////////////
void TextNode::
find_characters(Node *root) {
  string name;
  if (root->is_of_type(NamedNode::get_class_type())) {
    name = DCAST(NamedNode, root)->get_name();
  }

  bool all_digits = !name.empty();
  const char *p = name.c_str();
  while (all_digits && *p != '\0') {
    // VC++ complains if we treat an int as a bool, so we have to do
    // this != 0 comparsion on the int isdigit() function to shut it
    // up.
    all_digits = (isdigit(*p) != 0);
    p++;
  }
  
  if (all_digits) {
    int character = atoi(name.c_str());
    Geom *ch = NULL;
    GeomPoint *dot = NULL;
    AllTransitionsWrapper trans;
    find_character_gsets(root, ch, dot, trans);
    if (dot != NULL) {
      // Get the first vertex from the "dot" geoset.  This will be the
      // origin of the next character.
      PTA_Vertexf alist;
      PTA_ushort ilist;
      GeomBindType bind;
      float width;
      dot->get_coords(alist, bind, ilist);
      if (ilist.empty()) {
	width = alist[0][0];
      } else {
	width = alist[ilist[0]][0];
      }

      _defs[character] = CharDef(ch, width, trans);
    }

  } else if (name == "ds") {
    // The group "ds" is a special node that indicate's the font's
    // design size, or line height.

    Geom *ch = NULL;
    GeomPoint *dot = NULL;
    AllTransitionsWrapper trans;
    find_character_gsets(root, ch, dot, trans);
    if (dot != NULL) {
      // Get the first vertex from the "dot" geoset.  This will be the
      // design size indicator.
      PTA_Vertexf alist;
      PTA_ushort ilist;
      GeomBindType bind;
      dot->get_coords(alist, bind, ilist);
      if (ilist.empty()) {
	_font_height = alist[0][2];
      } else {
	_font_height = alist[ilist[0]][2];
      }
    }

  } else {
    DownRelations::const_iterator dri;
    dri = root->_children.find(RenderRelation::get_class_type());
    if (dri != root->_children.end()) {
      const DownRelationPointers &drp = (*dri).second;
      DownRelationPointers::const_iterator drpi;
      for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
	Node *node = (*drpi)->get_child();
	find_characters(node);
      }
    }
  }
}

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
assemble_row(const char *&source, Node *dest) {
  float xpos = 0.0;
  while (*source != '\0' && *source != '\n') {
    int character = (unsigned char)*source;

    if (character == ' ') {
      // A space is a special case.
      xpos += 0.25;

    } else {
      // A printable character.

      CharDefs::const_iterator cdi = _defs.find(character);
      if (cdi == _defs.end()) {
	text_cat.warning()
	  << "No definition for character " << character << endl;
  
      } else {
	Geom *char_geom = (*cdi).second._geom;
	float char_width = (*cdi).second._width;
	const AllTransitionsWrapper &trans = (*cdi).second._trans;
	
	LMatrix4f mat = LMatrix4f::ident_mat();
        mat.set_row(3, LVector3f(xpos, 0, 0)); 
	if (char_geom != NULL) {
	  string ch(1, (char)character);
	  GeomNode *geode = new GeomNode(ch);
	  geode->add_geom(char_geom);
	  RenderRelation* rel = new RenderRelation(dest, geode);
	  rel->set_transition(new TransformTransition(mat));
	  trans.store_to(rel);
	}
	
	xpos += char_width;
      }
    }
    source++;
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
Node *TextNode::
assemble_text(const char *source, LVector2f &ul, LVector2f &lr,
	      int &num_rows) {
  ul.set(0.0, 0.8 * _font_height);
  lr.set(0.0, 0.0);

  // Make a group node to hold our formatted text geometry.
  Node *root_node = new NamedNode("text");

  float posy = 0.0;
  int row_index = 0;
  while (*source != '\0') {
    char numstr[20];
    sprintf(numstr, "row%d", row_index);
    nassertr(strlen(numstr) < 20, root_node);

    Node *row = new NamedNode(numstr);
    float row_width = assemble_row(source, row);
    if (*source != '\0') {
      // Skip past the newline.
      source++;
    }
    
    LMatrix4f mat = LMatrix4f::ident_mat();
    if (_align == TM_ALIGN_LEFT) {
      mat.set_row(3, LVector3f(0, 0, posy));  
      lr[0] = max(lr[0], row_width);

    } else if (_align == TM_ALIGN_RIGHT) {
      mat.set_row(3, LVector3f(-row_width, 0, posy));  
      ul[0] = min(ul[0], -row_width);

    } else {
      mat.set_row(3, LVector3f(-row_width / 2.0, 0, posy));  
      lr[0] = max(lr[0], row_width / 2);
      ul[0] = min(ul[0], -row_width / 2);
    }

    // Also apply whatever slant the user has asked for to the entire
    // row.  This is an X shear.
    if (_slant != 0.0) {
      LMatrix4f shear(1, 0, 0, 0,
		      0, 1, 0, 0,
		      _slant, 0, 1, 0,
		      0, 0, 0, 1);
      mat = shear * mat;
    }

    RenderRelation *arc = new RenderRelation(root_node, row);
    arc->set_transition(new TransformTransition(mat));

    posy -= _font_height;
    num_rows++;
  }

  lr[1] = posy + 0.8 * _font_height;

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
measure_row(const char *&source) {
  float xpos = 0.0;
  while (*source != '\0' && *source != '\n') {
    int character = (unsigned char)*source;

    if (character == ' ') {
      // A space is a special case.
      xpos += 0.25;

    } else {
      // A printable character.

      CharDefs::const_iterator cdi = _defs.find(character);
      if (cdi == _defs.end()) {
	text_cat.warning()
	  << "No definition for character " << character << endl;
  
      } else {
	float char_width = (*cdi).second._width;
	xpos += char_width;
      }
    }
    source++;
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
measure_text(const char *source, LVector2f &ul, LVector2f &lr,
	     int &num_rows) {
  ul.set(0.0, 0.8 * _font_height);
  lr.set(0.0, 0.0);

  float posy = 0.0;
  while (*source != '\0') {
    float row_width = measure_row(source);
    if (*source != '\0') {
      // Skip past the newline.
      source++;
    }
    
    if (_align == TM_ALIGN_LEFT) {
      lr[0] = max(lr[0], row_width);

    } else if (_align == TM_ALIGN_RIGHT) {
      ul[0] = min(ul[0], -row_width);

    } else {
      lr[0] = max(lr[0], row_width / 2);
      ul[0] = min(ul[0], -row_width / 2);
    }

    posy -= _font_height;
    num_rows++;
  }

  lr[1] = posy + 0.8 * _font_height;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_frame
//       Access: Private
//  Description: Creates a frame around the text.
////////////////////////////////////////////////////////////////////
Node *TextNode::
make_frame() {
  GeomNode *frame_geode = new GeomNode("frame");

  LVector4f dimensions = get_frame_actual();
  float left = dimensions[0];
  float right = dimensions[1];
  float bottom = dimensions[2];
  float top = dimensions[3];
  
  GeomLinestrip *geoset = new GeomLinestrip;
  PTA_int lengths(0);
  PTA_Vertexf verts;
  lengths.push_back(5);
  verts.push_back(Vertexf(left, 0.0, top));
  verts.push_back(Vertexf(left, 0.0, bottom));
  verts.push_back(Vertexf(right, 0.0, bottom));
  verts.push_back(Vertexf(right, 0.0, top));
  verts.push_back(Vertexf(left, 0.0, top));
  
  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);

  geoset->set_coords(verts, G_PER_VERTEX);
  geoset->set_width(_frame_width);
  frame_geode->add_geom(geoset);

  if (get_frame_corners()) {
    GeomPoint *geoset = new GeomPoint;
    
    geoset->set_num_prims(4);
    geoset->set_coords(verts, G_PER_VERTEX);
    geoset->set_size(_frame_width);
    frame_geode->add_geom(geoset);
  }

  return frame_geode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_card
//       Access: Private
//  Description: Creates a card behind the text.
////////////////////////////////////////////////////////////////////
Node *TextNode::
make_card() {
  GeomNode *card_geode = new GeomNode("card");

  LVector4f dimensions = get_card_actual();
  float left = dimensions[0];
  float right = dimensions[1];
  float bottom = dimensions[2];
  float top = dimensions[3];

  GeomTristrip *geoset = new GeomTristrip;
  PTA_int lengths(0);
  lengths.push_back(4);

  PTA_Vertexf verts;
  verts.push_back(Vertexf::rfu(left, 0.02, top));
  verts.push_back(Vertexf::rfu(left, 0.02, bottom));
  verts.push_back(Vertexf::rfu(right, 0.02, top));
  verts.push_back(Vertexf::rfu(right, 0.02, bottom));
  
  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);

  geoset->set_coords(verts, G_PER_VERTEX);
 
  if (has_card_texture()) {
    PTA_TexCoordf uvs;
    uvs.push_back(TexCoordf(0.0, 1.0));
    uvs.push_back(TexCoordf(0.0, 0.0));
    uvs.push_back(TexCoordf(1.0, 1.0));
    uvs.push_back(TexCoordf(1.0, 0.0));

    geoset->set_texcoords(uvs, G_PER_VERTEX);
  }
 
  card_geode->add_geom(geoset);

  return card_geode;
}


////////////////////////////////////////////////////////////////////
//     Function: TextNode::make_card_with_border
//       Access: Private
//  Description: Creates a card behind the text with a specified border
//               for button edge or what have you.
////////////////////////////////////////////////////////////////////
Node *TextNode::
make_card_with_border() {
  GeomNode *card_geode = new GeomNode("card");

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
  verts.push_back(Vertexf::rfu(left, 0.02, top));
  verts.push_back(Vertexf::rfu(left, 0.02, top - _card_border_size));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02, top));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02, 
			       top - _card_border_size));
  // verts 5,6,7,8 
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02, top));
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02, 
			       top - _card_border_size));
  verts.push_back(Vertexf::rfu(right, 0.02, top));
  verts.push_back(Vertexf::rfu(right, 0.02, top - _card_border_size));
  // verts 9,10,11,12 
  verts.push_back(Vertexf::rfu(left, 0.02, bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(left, 0.02, bottom));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02, 
			       bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(left + _card_border_size, 0.02, bottom));
  // verts 13,14,15,16
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02, 
			       bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(right - _card_border_size, 0.02, bottom));
  verts.push_back(Vertexf::rfu(right, 0.02, bottom + _card_border_size));
  verts.push_back(Vertexf::rfu(right, 0.02, bottom));
  
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

  geoset->set_coords(verts, G_PER_VERTEX, indices);
 
  if (has_card_texture()) {
    PTA_TexCoordf uvs;
    uvs.push_back(TexCoordf(0.0, 1.0)); //1
    uvs.push_back(TexCoordf(0.0, 1.0 - _card_border_uv_portion)); //2
    uvs.push_back(TexCoordf(0.0 + _card_border_uv_portion, 1.0)); //3
    uvs.push_back(TexCoordf(0.0 + _card_border_uv_portion, 
      1.0 - _card_border_uv_portion)); //4
    uvs.push_back(TexCoordf( 1.0 -_card_border_uv_portion, 1.0)); //5
    uvs.push_back(TexCoordf( 1.0 -_card_border_uv_portion, 
      1.0 - _card_border_uv_portion)); //6
    uvs.push_back(TexCoordf(1.0, 1.0)); //7
    uvs.push_back(TexCoordf(1.0, 1.0 - _card_border_uv_portion)); //8

    uvs.push_back(TexCoordf(0.0, _card_border_uv_portion)); //9
    uvs.push_back(TexCoordf(0.0, 0.0)); //10
    uvs.push_back(TexCoordf(_card_border_uv_portion, _card_border_uv_portion)); //11
    uvs.push_back(TexCoordf(_card_border_uv_portion, 0.0)); //12

    uvs.push_back(TexCoordf(1.0 - _card_border_uv_portion, _card_border_uv_portion));//13
    uvs.push_back(TexCoordf(1.0 - _card_border_uv_portion, 0.0));//14
    uvs.push_back(TexCoordf(1.0, _card_border_uv_portion));//15
    uvs.push_back(TexCoordf(1.0, 0.0));//16

    // we can use same ref's as before (same order)
    geoset->set_texcoords(uvs, G_PER_VERTEX, indices);

  }
 
  card_geode->add_geom(geoset);

  return card_geode;
}
