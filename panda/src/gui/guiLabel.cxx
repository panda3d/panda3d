// Filename: guiLabel.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiLabel.h"

#include <textNode.h>
#include <transformTransition.h>
#include <colorTransition.h>

TypeHandle GuiLabel::_type_handle;

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
  case SIMPLE_CARD:
    {
      LMatrix4f mat = LMatrix4f::scale_mat(_scale) *
	LMatrix4f::translate_mat(_pos);
      _internal->set_transition(new TransformTransition(mat));
    }
    break;
  default:
    gui_cat->warning() << "recompute_transform on invalid label type ("
		       << (int)_type << ")" << endl;
  }
  set_properties();
}

void GuiLabel::set_properties(void) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_text_color(_foreground);
      n->clear_card();
      if ((_have_background) || (_tex == (Texture*)0L)) {
	if (_have_background)
	  n->set_card_color(_background);
	if (_tex != (Texture*)0L)
	  n->set_card_texture(_tex);
	if (_have_width || _have_height) {
	  n->set_card_as_margin(simple_text_margin_left,
				simple_text_margin_right,
				simple_text_margin_bottom,
				simple_text_margin_top);
	  LVecBase4f v = n->get_card_actual();
	  float w = v[1] - v[0];
	  float h = v[3] - v[2];
	  if (_have_width) {
	    w = _width - w;
	    w *= 0.5;
	    v[1] += w;
	    v[0] -= w;
	  } else {
	    v[0] -= simple_text_margin_left;
	    v[1] += simple_text_margin_right;
	  }
	  if (_have_height) {
	    h = _height - h;
	    h *= 0.5;
	    v[3] += h;
	    v[2] -= h;
	  } else {
	    v[2] -= simple_text_margin_bottom;
	    v[3] += simple_text_margin_top;
	  }
	  n->set_card_actual(v[0], v[1], v[2], v[3]);
	} else
	  n->set_card_as_margin(simple_text_margin_left,
				simple_text_margin_right,
				simple_text_margin_bottom,
				simple_text_margin_top);
      }
    }
    break;
  case SIMPLE_TEXTURE:
    _internal->set_transition(new ColorTransition(_foreground));
    break;
  case SIMPLE_CARD:
    _internal->set_transition(new ColorTransition(_foreground));
    {
      float w, h;
      w = _have_width?(_width * 0.5):0.5;
      h = _have_height?(_height * 0.5):0.5;
      PTA_Vertexf verts;
      verts.push_back(Vertexf::rfu(-w, 0., h));
      verts.push_back(Vertexf::rfu(-w, 0., -h));
      verts.push_back(Vertexf::rfu(w, 0., h));
      verts.push_back(Vertexf::rfu(w, 0., -h));
      _gset->set_coords(verts, G_PER_VERTEX);
    }
    break;
  default:
    gui_cat->warning() << "recompute_transform on invalid label type ("
		       << (int)_type << ")" << endl;
  }
}

GuiLabel::~GuiLabel(void) {
  if (gui_cat->is_debug())
    gui_cat->debug() << "deleting label (0x" << (void*)this << ")" << endl;
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
  ret->_gset = geoset;
  return ret;
}

GuiLabel* GuiLabel::make_simple_text_label(const string& text, Node* font,
					   Texture* tex) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = SIMPLE_TEXT;
  TextNode* n = new TextNode("GUI label");
  ret->_geom = n;
  ret->_tex = tex;

  // The GuiLabel is initially frozen at the time it is created.
  n->freeze();

  n->set_font(font);
  n->set_align(TM_ALIGN_CENTER);
  n->set_text_color(ret->get_foreground_color());
  n->set_text(text);
  if (tex != (Texture*)0L)
    n->set_card_texture(tex);
  ret->set_scale(1.);
  ret->set_pos(LVector3f(0., 0., 0.));
  ret->recompute_transform();
  return ret;
}

GuiLabel* GuiLabel::make_simple_card_label(void) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = SIMPLE_CARD;
  ret->_geom = new NamedNode("GUI label");
  GeomNode* n2 = new GeomNode();
  ret->_internal = new RenderRelation(ret->_geom, n2);
  ret->_internal->set_transition(
				new ColorTransition(Colorf(ret->_foreground)));
  GeomTristrip *geoset = new GeomTristrip;
  PTA_int lengths(0);
  lengths.push_back(4);
  PTA_Vertexf verts;
  verts.push_back(Vertexf::rfu(-0.5, 0., 0.5));
  verts.push_back(Vertexf::rfu(-0.5, 0., -0.5));
  verts.push_back(Vertexf::rfu(0.5, 0., 0.5));
  verts.push_back(Vertexf::rfu(0.5, 0., -0.5));
  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);
  geoset->set_coords(verts, G_PER_VERTEX);
  n2->add_geom(geoset);
  ret->_gset = geoset;
  return ret;
}

int GuiLabel::freeze() {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      gui_cat->spam() << "GuiLabel:: freezing text node (0x" << (void*)this
		      << ")" << endl;
      TextNode* n = DCAST(TextNode, _geom);
      return n->freeze();
    }

  default:
    return 0;
  }
}

int GuiLabel::thaw() {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      gui_cat->spam() << "GuiLabel:: thawing text node (0x" << (void*)this
		      << ")" << endl;
      TextNode* n = DCAST(TextNode, _geom);
      return n->thaw();
    }

  default:
    return 0;
  }
}

void GuiLabel::get_extents(float& l, float& r, float& b, float& t) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      if (n->has_card()) {
	LVecBase4f v = n->get_card_transformed();
	l = v[0];
	r = v[1];
	b = v[2];
	t = v[3];
      } else {
	LVector3f ul = n->get_upper_left_3d() - LPoint3f::origin();
	LVector3f lr = n->get_lower_right_3d() - LPoint3f::origin();
	LVector3f up = LVector3f::up();
	LVector3f right = LVector3f::right();
	l = ul.dot(right);
	r = lr.dot(right);
	b = lr.dot(up);
	t = ul.dot(up);
      }
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
  case SIMPLE_CARD:
    {
      l = _have_width?-(_width*0.5):-0.5;
      r = _have_width?(_width*0.5):0.5;
      b = _have_height?-(_height*0.5):-0.5;
      t = _have_height?(_height*0.5):0.5;
    }
    break;
  default:
    gui_cat->warning()
      << "trying to get extents from something I don't know how to" << endl;
    l = b = 0.;
    r = t = 1.;
  }
}

float GuiLabel::get_width(void) {
  float w;
  TextNode* n = DCAST(TextNode, _geom);
  if (n->has_card()) {
    LVecBase4f v = n->get_card_actual();
    w = v[1] - v[0];
  } else {
    w = n->get_width();
  }
  return w;
}

float GuiLabel::get_height(void) {
  float h;
  TextNode* n = DCAST(TextNode, _geom);
  if (n->has_card()) {
    LVecBase4f v = n->get_card_actual();
    h = v[3] - v[2];
  } else {
    h = n->get_width();
  }
  return h;
}

void GuiLabel::set_foreground_color(const Colorf& color) {
  _foreground = color;
  set_properties();
}

void GuiLabel::set_background_color(const Colorf& color) {
  static Colorf zero(0., 0., 0., 0.);

  _background = color;
  _have_background = (color != zero);
  set_properties();
}

void GuiLabel::set_text(const string& val) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_text(val);
    }
    break;
  case SIMPLE_TEXTURE:
    gui_cat->warning() << "tried to set text on a texture label" << endl;
    break;
  case SIMPLE_CARD:
    gui_cat->warning() << "tried to set text on a card label" << endl;
    break;
  default:
    gui_cat->warning() << "trying to set text on an unknown label type ("
		       << (int)_type << ")" << endl;
  }
  recompute();
}

bool GuiLabel::operator<(const GuiLabel& c) const {
  PriorityMap::const_iterator pi;
  pi = _priorities.find((GuiLabel*)(&c));
  if (pi != _priorities.end()) {
    if ((*pi).second == P_LOWER)
      return true;
    else
      return false;
  }
  return ((void*)this) < ((void*)&c);
}
