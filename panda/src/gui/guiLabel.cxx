// Filename: guiLabel.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiLabel.h"

#include <textNode.h>
#include <transformTransition.h>
#include <colorTransition.h>

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
  case SIMPLE_TEXTURE:
    {
      LMatrix4f mat = LMatrix4f::scale_mat(_scale) *
	LMatrix4f::translate_mat(_pos);
      _internal->set_transition(new TransformTransition(mat));
    }
    break;
  default:
    gui_cat->warning() << "recompute_transform on invalid label type ("
		       << _type << ")" << endl;
  }
  set_properties();
}

void GuiLabel::set_properties(void) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_text_color(_foreground);
      if (!_have_background) {
	n->clear_card();
	if (gui_cat->is_debug())
	  gui_cat->debug() << "cleared card" << endl;
      } else {
	n->set_card_color(_background);
	n->set_card_as_margin(0., 0., 0., 0.);
	if (gui_cat->is_debug()) {
	  gui_cat->debug() << "set card color" << endl;
	  if (n->has_card())
	    gui_cat->debug() << ".. and a card was made" << endl;
	  else
	    gui_cat->debug() << ".. but there is no card" << endl;
	}
      }
    }
    break;
  case SIMPLE_TEXTURE:
    _internal->set_transition(new ColorTransition(_foreground));
    break;
  default:
    gui_cat->warning() << "recompute_transform on invalid label type ("
		       << _type << ")" << endl;
  }
}

GuiLabel::~GuiLabel(void) {
}

#include <textureTransition.h>
#include <geomTristrip.h>

GuiLabel* GuiLabel::make_simple_texture_label(Texture* texture) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = SIMPLE_TEXTURE;
  ret->_tex = texture;
  ret->_geom = new NamedNode("GUI label");
  GeomNode* n2 = new GeomNode();
  ret->_internal = new RenderRelation(ret->_geom, n2);
  ret->_internal->set_transition(new TextureTransition(texture));
  GeomTristrip *geoset = new GeomTristrip;
  PTA_int lengths(0);
  lengths.push_back(4);
  PTA_Vertexf verts;
  float l, r, b, t;
  {
    // compute {l, r, b, t}
    int xs = texture->_pbuffer->get_xsize();
    int ys = texture->_pbuffer->get_ysize();
    float ratio;

    if (xs > ys) {
      // horizontally dominant
      ratio = ((float)ys) / ((float)xs);
      ratio *= 0.5;
      l = -0.5;
      r = 0.5;
      b = -ratio;
      t = ratio;
    } else {
      // vertically dominant
      ratio = ((float)xs) / ((float)ys);
      ratio *= 0.5;
      l = -ratio;
      r = ratio;
      b = -0.5;
      t = 0.5;
    }
  }
  verts.push_back(Vertexf::rfu(l, 0., t));
  verts.push_back(Vertexf::rfu(l, 0., b));
  verts.push_back(Vertexf::rfu(r, 0., t));
  verts.push_back(Vertexf::rfu(r, 0., b));
  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);
  geoset->set_coords(verts, G_PER_VERTEX);
  PTA_TexCoordf uvs;
  uvs.push_back(TexCoordf(0., 1.));
  uvs.push_back(TexCoordf(0., 0.));
  uvs.push_back(TexCoordf(1., 1.));
  uvs.push_back(TexCoordf(1., 0.));
  geoset->set_texcoords(uvs, G_PER_VERTEX);
  n2->add_geom(geoset);
  return ret;
}

GuiLabel* GuiLabel::make_simple_text_label(const string& text, Node* font) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = SIMPLE_TEXT;
  TextNode* n = new TextNode("GUI label");
  ret->_geom = n;
  n->set_font(font);
  n->set_align(TM_ALIGN_CENTER);
  n->set_text_color(ret->get_foreground_color());
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
  case SIMPLE_TEXTURE:
    {
      float xs = _tex->_pbuffer->get_xsize();
      float ys = _tex->_pbuffer->get_ysize();
      float ratio;
      LVector3f ul, lr;

      if (xs > ys) {
	// horizontally dominant
	ratio = ((float)ys) / ((float)xs);
	ratio *= 0.5;
	ul = LVector3f::rfu(-0.5, 0., ratio);
	lr = LVector3f::rfu(0.5, 0., -ratio);
      } else {
	// vertically dominant
	ratio = ((float)xs) / ((float)ys);
	ratio *= 0.5;
	ul = LVector3f::rfu(-ratio, 0., 0.5);
	lr = LVector3f::rfu(ratio, 0., -0.5);
      }
      LMatrix4f mat = LMatrix4f::scale_mat(_scale) *
	LMatrix4f::translate_mat(_pos);
      ul = mat * ul;
      lr = mat * lr;
      l = ul.dot(ul.right());
      r = lr.dot(lr.right());
      b = lr.dot(lr.up());
      t = ul.dot(ul.up());
    }
    break;
  default:
    gui_cat->warning()
      << "trying to get extents from something I don't know how to" << endl;
    l = b = 0.;
    r = t = 1.;
  }
}

void GuiLabel::set_foreground_color(const Colorf& color) {
  _foreground = color;
  set_properties();
}

void GuiLabel::set_background_color(const Colorf& color) {
  static Colorf zero(0., 0., 0., 0.);

  _background = color;
  _have_background = (color != zero);
  if (gui_cat->is_debug()) {
    if (_have_background)
      gui_cat->debug() << "setting background" << endl;
    else
      gui_cat->debug() << "setting no background" << endl;
  }
  set_properties();
}
