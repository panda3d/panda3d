// Filename: guiLabel.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiLabel.h"

#include <textNode.h>

void GuiLabel::recompute_transform(void) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      LMatrix4f mat = LMatrix4f::scale_mat(_scale) *
	LMatrix4f::translate_mat(_pos);
      TextNode* n = DCAST(TextNode, _geom);
      n->set_transform(mat);
    }
    break;
  default:
    gui_cat->warning() << "recompute_transform on invalid label type ("
		       << _type << ")" << endl;
  }
}

GuiLabel::~GuiLabel(void) {
}

GuiLabel* GuiLabel::make_simple_texture_label(void) {
  return new GuiLabel();
}

GuiLabel* GuiLabel::make_simple_text_label(const string& text, Node* font) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = SIMPLE_TEXT;
  TextNode* n = new TextNode("GUI label");
  ret->_geom = n;
  n->set_font(font);
  n->set_align(TM_ALIGN_CENTER);
  n->set_text_color(1., 1., 1., 1.);
  n->set_text(text);
  ret->set_scale(1.);
  ret->set_pos(LVector3f(0., 0., 0.));
  ret->recompute_transform();
  return ret;
}

void GuiLabel::get_extents(float& l, float& r, float& b, float& t) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      LVector3f ul = n->get_upper_left_3d() - LPoint3f::origin();
      LVector3f lr = n->get_lower_right_3d() - LPoint3f::origin();
      LVector3f up = LVector3f::up();
      LVector3f right = LVector3f::right();
      l = ul.dot(right);
      r = lr.dot(right);
      b = lr.dot(up);
      t = ul.dot(up);
    }
    break;
  default:
    gui_cat->warning()
      << "trying to get extents from something I don't know how to" << endl;
    l = b = 0.;
    r = t = 1.;
  }
}
