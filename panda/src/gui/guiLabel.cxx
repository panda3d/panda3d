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
  this->freeze();

/*
      LMatrix4f mat = LMatrix4f::scale_mat(LVector3f::rfu(_scale_x, _scale_y,
							  _scale_z)) *
	LMatrix4f::scale_mat(_scale) *
	  LMatrix4f::scale_mat(LVector3f::rfu((_mirror_x?-1.0f:1.0f), 1.0f,
						  (_mirror_y?-1.0f:1.0f))) *
	LMatrix4f::translate_mat(_pos);
*/	
  // optimize the above calculation
  LVector3f scalevec1 = LVector3f::rfu(_scale_x*_scale, _scale_y*_scale, _scale_z*_scale);
  LVector3f scalevec2 =  LVector3f::rfu((_mirror_x?-1.0f:1.0f), 
										1.0f,
										(_mirror_y?-1.0f:1.0f));

  scalevec1._v.v._0 *= scalevec2._v.v._0;
  scalevec1._v.v._1 *= scalevec2._v.v._1;
  scalevec1._v.v._2 *= scalevec2._v.v._2;

  LMatrix4f mat(scalevec1._v.v._0, 0.0f, 0.0f, 0.0f,
				0.0f, scalevec1._v.v._1, 0.0f, 0.0f, 
				0.0f, 0.0f, scalevec1._v.v._2, 0.0f,
				_pos._v.v._0, _pos._v.v._1, _pos._v.v._2, 1.0f);

  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_transform(mat);
    }
    break;
  case SIMPLE_TEXTURE:
  case SIMPLE_CARD:
  case L_NULL:
    {
      _internal->set_transition(new TransformTransition(mat));
    }
    break;
  case MODEL:
    {
      float w=_have_width?_scale*_width:_scale;
      float h=_have_height?_scale*_height:_scale;

/*
      LMatrix4f mat = LMatrix4f::scale_mat(LVector3f::rfu(_scale_x, _scale_y,
							  _scale_z)) *
	LMatrix4f::scale_mat(LVector3f::rfu(w, 1.0f, h)) *
	LMatrix4f::scale_mat(LVector3f::rfu((_mirror_x?-1.0f:1.0f), 1.0f,
					    (_mirror_y?-1.0f:1.0f))) *
	LMatrix4f::translate_mat(_pos + _model_pos);
*/	
      // optimize above calculation
	  LVector3f scalevec3 = LVector3f::rfu(w, 1.0f, h);

	  mat._m.m._00 *= scalevec3._v.v._0;
	  mat._m.m._11 *= scalevec3._v.v._1;
	  mat._m.m._22 *= scalevec3._v.v._2;

	  mat._m.m._30 += _model_pos._v.v._0;
	  mat._m.m._31 += _model_pos._v.v._1;
	  mat._m.m._32 += _model_pos._v.v._2;

      _internal->set_transition(new TransformTransition(mat));
    }
    break;
  default:
    gui_cat->warning() << "recompute_transform on invalid label type ("
		       << (int)_type << ")" << endl;
  }
  set_properties();
  this->thaw();
}

void GuiLabel::set_properties(void) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      if (_have_foreground)
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
	    w *= 0.5f;
	    v[1] += w;
	    v[0] -= w;
	  } else {
	    v[0] -= simple_text_margin_left;
	    v[1] += simple_text_margin_right;
	  }
	  if (_have_height) {
	    h = _height - h;
	    h *= 0.5f;
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
  case MODEL:
  case L_NULL:
    if (_have_foreground)
      _internal->set_transition(new ColorTransition(_foreground));
    break;
  case SIMPLE_CARD:
    if (_have_foreground) {
      _internal->set_transition(new ColorTransition(_foreground));
      TransparencyProperty::Mode mode;
      if (_foreground[3] != 1.0f)
	mode = TransparencyProperty::M_alpha;
      else
	mode = TransparencyProperty::M_none;
      _internal->set_transition(new TransparencyTransition(mode));
    }
    {
      float w, h;
      w = _have_width?(_width * 0.5f):0.5f;
      h = _have_height?(_height * 0.5f):0.5f;
      PTA_Vertexf verts;
      verts.push_back(Vertexf::rfu(-w, 0.0f, h));
      verts.push_back(Vertexf::rfu(-w, 0.0f, -h));
      verts.push_back(Vertexf::rfu(w, 0.0f, h));
      verts.push_back(Vertexf::rfu(w, 0.0f, -h));
      _gset->set_coords(verts, G_PER_VERTEX);
    }
    break;
  default:
    gui_cat->warning() << "set_properties on invalid label type ("
		       << (int)_type << ")" << endl;
  }
}

GuiLabel::~GuiLabel(void) {
#ifdef _DEBUG
  if (gui_cat->is_debug())
    gui_cat->debug() << "deleting label (0x" << (void*)this << ")" << endl;
#endif
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
      ratio *= 0.5f;
      l = -0.5f;
      r = 0.5f;
      b = -ratio;
      t = ratio;
    } else {
      // vertically dominant
      ratio = ((float)xs) / ((float)ys);
      ratio *= 0.5f;
      l = -ratio;
      r = ratio;
      b = -0.5f;
      t = 0.5f;
    }
  }
  verts.push_back(Vertexf::rfu(l, 0.0f, t));
  verts.push_back(Vertexf::rfu(l, 0.0f, b));
  verts.push_back(Vertexf::rfu(r, 0.0f, t));
  verts.push_back(Vertexf::rfu(r, 0.0f, b));
  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);
  geoset->set_coords(verts, G_PER_VERTEX);
  PTA_TexCoordf uvs;
  uvs.push_back(TexCoordf(0.0f, 1.0f));
  uvs.push_back(TexCoordf(0.0f, 0.0f));
  uvs.push_back(TexCoordf(1.0f, 1.0f));
  uvs.push_back(TexCoordf(1.0f, 0.0f));
  geoset->set_texcoords(uvs, G_PER_VERTEX);
  n2->add_geom(geoset);
  ret->_gset = geoset;
  return ret;
}

GuiLabel* GuiLabel::make_simple_text_label(const string& text, TextFont* font,
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
  ret->set_scale(1.0f);
  ret->set_pos(LVector3f(0.0f, 0.0f, 0.0f));
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
  verts.push_back(Vertexf::rfu(-0.5f, 0.0f, 0.5f));
  verts.push_back(Vertexf::rfu(-0.5f, 0.0f, -0.5f));
  verts.push_back(Vertexf::rfu(0.5f, 0.0f, 0.5f));
  verts.push_back(Vertexf::rfu(0.5f, 0.0f, -0.5f));
  geoset->set_num_prims(1);
  geoset->set_lengths(lengths);
  geoset->set_coords(verts, G_PER_VERTEX);
  n2->add_geom(geoset);
  ret->_gset = geoset;
  return ret;
}

GuiLabel* GuiLabel::make_null_label(void) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = L_NULL;
  ret->_geom = new NamedNode("GUI label");
  NamedNode* n2 = new NamedNode("dummy");
  ret->_internal = new RenderRelation(ret->_geom, n2);
  ret->_internal->set_transition(
				new ColorTransition(Colorf(ret->_foreground)));
  return ret;
}

GuiLabel* GuiLabel::make_model_label(Node* geom, float w, float h) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = MODEL;
  ret->_geom = new NamedNode("GUI label");
  ret->_model_width = w;
  ret->_model_height = h;
  ret->_internal = new RenderRelation(ret->_geom, geom);
#ifdef _DEBUG
  gui_cat->debug() << "created model label 0x" << (void*)ret 
		   << " from node 0x" << (void*)geom
		   << ", set _type(" << (int)(ret->_type) << ") to MODEL("
		   << (int)MODEL << ")" << endl;
#endif
  return ret;
}

GuiLabel* GuiLabel::make_model_label(Node* geom, float left, float right,
				     float bottom, float top) {
  GuiLabel* ret = new GuiLabel();
  ret->_type = MODEL;
  ret->_geom = new NamedNode("GUI label");
  ret->_model_pos = LVector3f::rfu(-(left + right) * 0.5f, 0.0f,
				   -(bottom + top) * 0.5f);
  ret->_model_width = right - left;
  ret->_model_height = top - bottom;
  ret->_internal = new RenderRelation(ret->_geom, geom);
#ifdef _DEBUG
  gui_cat->debug() << "created model label 0x" << (void*)ret 
		   << " from node 0x" << (void*)geom
		   << ", set _type(" << (int)(ret->_type) << ") to MODEL("
		   << (int)MODEL << ")" << endl;
#endif
  return ret;
}

int GuiLabel::freeze() {
  switch (_type) {
  case SIMPLE_TEXT:
    {
#ifdef _DEBUG
      gui_cat->spam() << "GuiLabel:: freezing text node (0x" << (void*)this
		      << ")" << endl;
#endif
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
#ifdef _DEBUG
      gui_cat->spam() << "GuiLabel:: thawing text node (0x" << (void*)this
		      << ")" << endl;
#endif
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
		ratio *= 0.5f;
		ul = LVector3f::rfu(-0.5f, 0.0f, ratio);
		lr = LVector3f::rfu(0.5f, 0.0f, -ratio);
      } else {
		// vertically dominant
		ratio = ((float)xs) / ((float)ys);
		ratio *= 0.5f;
		ul = LVector3f::rfu(-ratio, 0.0f, 0.5f);
		lr = LVector3f::rfu(ratio, 0.0f, -0.5f);
      }

/*
      LMatrix4f mat = LMatrix4f::scale_mat(LVector3f::rfu(_scale_x, _scale_y,
							  _scale_z)) *
	LMatrix4f::scale_mat(_scale) *
	LMatrix4f::translate_mat(_pos);
	
    ul = ul * mat;
    lr = lr * mat;	
*/	
	  // optimize above

      LVector3f scalevec1 = LVector3f::rfu(_scale_x*_scale, _scale_y*_scale, _scale_z*_scale);
	  LVector3f scalevec2 = LVector3f::rfu((_mirror_x?-1.0f:1.0f), 
											1.0f,
											(_mirror_y?-1.0f:1.0f));

	  scalevec1._v.v._0 *= scalevec2._v.v._0;
	  scalevec1._v.v._1 *= scalevec2._v.v._1;
	  scalevec1._v.v._2 *= scalevec2._v.v._2;

	  ul._v.v._0 = scalevec1._v.v._0 * ul._v.v._0 + _pos._v.v._0;
	  ul._v.v._1 = scalevec1._v.v._1 * ul._v.v._1 + _pos._v.v._1;
	  ul._v.v._2 = scalevec1._v.v._2 * ul._v.v._2 + _pos._v.v._2;

	  lr._v.v._0 = scalevec1._v.v._0 * lr._v.v._0 + _pos._v.v._0;
	  lr._v.v._1 = scalevec1._v.v._1 * lr._v.v._1 + _pos._v.v._1;
	  lr._v.v._2 = scalevec1._v.v._2 * lr._v.v._2 + _pos._v.v._2;

      l = ul.dot(ul.right());
      r = lr.dot(lr.right());
      b = lr.dot(lr.up());
      t = ul.dot(ul.up());
    }
    break;
  case SIMPLE_CARD:
    {
      float x = _pos.dot(LVector3f::rfu(1.0f, 0.0f, 0.0f));
      float y = _pos.dot(LVector3f::rfu(0.0f, 0.0f, 1.0f));
      l = _have_width?-(_width*0.5f):-0.5f;
      r = _have_width?(_width*0.5f):0.5f;
      l += x;
      r += x;
      b = _have_height?-(_height*0.5f):-0.5f;
      t = _have_height?(_height*0.5f):0.5f;
      b += y;
      t += y;
    }
    break;
  case L_NULL:
    {
      float x = _pos.dot(LVector3f::rfu(1.0f, 0.0f, 0.0f));
      float y = _pos.dot(LVector3f::rfu(0.0f, 0.0f, 1.0f));
      l = _have_width?-(_width*0.5f):-0.000005f;
      r = _have_width?(_width*0.5f):0.000005f;
      l += x;
      r += x;
      b = _have_height?-(_height*0.5f):-0.000005f;
      t = _have_height?(_height*0.5f):0.000005f;
      b += y;
      t += y;
    }
    break;
  case MODEL:
    {
      float x = _pos.dot(LVector3f::rfu(1.0f, 0.0f, 0.0f));
      float y = _pos.dot(LVector3f::rfu(0.0f, 0.0f, 1.0f));
      l = _have_width?-(_width*_model_width*0.5f):-(_model_width*0.5f);
      r = _have_width?(_width*_model_width*0.5f):(_model_width*0.5f);
      l += x;
      r += x;
      b = _have_height?-(_height*_model_height*0.5f):-(_model_height*0.5f);
      t = _have_height?(_height*_model_height*0.5f):(_model_height*0.5f);
      b += y;
      t += y;
    }
    break;
  default:
    gui_cat->warning()
      << "trying to get extents from something I don't know how to" << endl;
    l = b = 0.0f;
    r = t = 1.0f;
  }
}

float GuiLabel::get_width(void) {
  float w;
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      if (n->has_card()) {
	LVecBase4f v = n->get_card_actual();
	w = v[1] - v[0];
      } else
	w = n->get_width();
    }
    break;
  case SIMPLE_TEXTURE:
    gui_cat->warning() << "tried to get width from a texture label" << endl;
    w = 1.0f;
    break;
  case SIMPLE_CARD:
    w = _have_width?_width:1.0f;
    break;
  case MODEL:
    w = _have_width?(_width*_model_width):_model_width;
    break;
  case L_NULL:
    w = _have_width?_width:0.00001f;
    break;
  default:
    gui_cat->warning()
      << "trying to get width from something I don't know how to" << endl;
    w = 1.0f;
  }
  return w;
}

float GuiLabel::get_height(void) {
  float h;
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      if (n->has_card()) {
	LVecBase4f v = n->get_card_actual();
	h = v[3] - v[2];
      } else
	h = n->get_width();
    }
    break;
  case SIMPLE_TEXTURE:
    gui_cat->warning() << "tried to get height from a texture label" << endl;
    h = 1.0f;
    break;
  case SIMPLE_CARD:
    h = _have_height?_height:1.0f;
    break;
  case MODEL:
    h = _have_height?(_height*_model_height):_model_height;
    break;
  case L_NULL:
    h = _have_height?_height:0.00001;
    break;
  default:
    gui_cat->warning()
      << "trying to get height from something I don't know how to" << endl;
    h = 1.0f;
  }
  return h;
}

void GuiLabel::set_foreground_color(const Colorf& color) {
  _foreground = color;
  set_properties();
}

void GuiLabel::set_background_color(const Colorf& color) {
  static Colorf zero(0.0f, 0.0f, 0.0f, 0.0f);

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
  case MODEL:
    gui_cat->warning() << "tried to set text on a model label" << endl;
    break;
  case L_NULL:
    gui_cat->warning() << "tried to set text on a null label" << endl;
    break;
  default:
    gui_cat->warning() << "trying to set text on an unknown label type ("
		       << (int)_type << ")" << endl;
  }
  recompute();
}

void GuiLabel::set_shadow_color(const Colorf& c) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_shadow_color(c);
    }
    break;
  case SIMPLE_TEXTURE:
    gui_cat->warning() << "tried to set shadow color on a texture label"
		       << endl;
    break;
  case SIMPLE_CARD:
    gui_cat->warning() << "tried to set shadow color on a card label" << endl;
    break;
  case MODEL:
    gui_cat->warning() << "tried to set shadow color on a model label" << endl;
    break;
  case L_NULL:
    gui_cat->warning() << "tried to set shadow color on a null label" << endl;
    break;
  default:
    gui_cat->warning()
      << "trying to set shadow color on an unknown label type (" << (int)_type
      << ")" << endl;
  }
  recompute();
}

void GuiLabel::set_shadow(float x, float y) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_shadow(x, y);
    }
    break;
  case SIMPLE_TEXTURE:
    gui_cat->warning() << "tried to set shadow on a texture label" << endl;
    break;
  case SIMPLE_CARD:
    gui_cat->warning() << "tried to set shadow on a card label" << endl;
    break;
  case MODEL:
    gui_cat->warning() << "tried to set shadow on a model label" << endl;
    break;
  case L_NULL:
    gui_cat->warning() << "tried to set shadow on a null label" << endl;
    break;
  default:
    gui_cat->warning() << "trying to set shadow on an unknown label type ("
		       << (int)_type << ")" << endl;
  }
  recompute();
}

void GuiLabel::set_align(int a) {
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_align(a);
    }
    break;
  case SIMPLE_TEXTURE:
    gui_cat->warning() << "tried to set align on a texture label" << endl;
    break;
  case SIMPLE_CARD:
    gui_cat->warning() << "tried to set align on a card label" << endl;
    break;
  case MODEL:
    gui_cat->warning() << "tried to set align on a model label" << endl;
    break;
  case L_NULL:
    gui_cat->warning() << "tried to set align on a null label" << endl;
    break;
  default:
    gui_cat->warning() << "trying to set align on an unknown label type ("
		       << (int)_type << ")" << endl;
  }
  recompute();
}

bool GuiLabel::operator<(const GuiLabel& c) const {
  if (_highest_pri)
    return false;
  if (c._highest_pri)
    return true;
  if (_lowest_pri)
    return true;
  if (c._lowest_pri)
    return false;
  PriorityMap::const_iterator pi;
  pi = _priorities.find((GuiLabel*)(&c));
  if (pi != _priorities.end()) {
    if ((*pi).second == P_LOWER)
      return true;
    else
      return false;
  }
  pi = c._priorities.find((GuiLabel*)this);
  if (pi != c._priorities.end()) {
    if ((*pi).second == P_LOWER)
      return false;
    else
      return true;
  }
  return ((void*)this) < ((void*)&c);
}

#include <geomBinTransition.h>

int GuiLabel::set_draw_order(int order) {
  int ret = order+1;
  this->freeze();
  _has_hard_pri = true;
  _hard_pri = order;
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_bin("fixed");
      n->set_draw_order(order);
      ret += 2;
    }
    break;
  case SIMPLE_TEXTURE:
  case SIMPLE_CARD:
  case L_NULL:
  case MODEL:
    _internal->set_transition(new GeomBinTransition("fixed", order));
    break;
  default:
    gui_cat->warning() << "trying to set draw order on an unknown label type ("
		       << (int)_type << ")" << endl;
  }
  this->thaw();
  return ret;
}

int GuiLabel::soft_set_draw_order(int order) {
  int ret = order+1;
  this->freeze();
  _has_hard_pri = false;
  _hard_pri = order;
  switch (_type) {
  case SIMPLE_TEXT:
    {
      TextNode* n = DCAST(TextNode, _geom);
      n->set_bin("fixed");
      n->set_draw_order(order);
      ret += 2;
    }
    break;
  case SIMPLE_TEXTURE:
  case SIMPLE_CARD:
  case L_NULL:
  case MODEL:
    _arc->set_transition(new GeomBinTransition("fixed", order));
    break;
  default:
    gui_cat->warning() << "trying to set draw order on an unknown label type ("
		       << (int)_type << ")" << endl;
  }
  this->thaw();
  return ret;
}

void GuiLabel::write(ostream& os) const {
  os << "GuiLabel: (0x" << (void*)this << ")" << endl;
  os << "  refcount = " << this->get_ref_count() << endl;
  os << "  _type = ";
  switch (this->_type) {
  case NONE:
    os << "NONE";
    break;
  case SIMPLE_TEXTURE:
    os << "SIMPLE_TEXTURE";
    break;
  case SIMPLE_TEXT:
    os << "SIMPLE_TEXT";
    break;
  case SIMPLE_CARD:
    os << "SIMPLE_CARD";
    break;
  case MODEL:
    os << "MODEL";
    break;
  case L_NULL:
    os << "NULL";
    break;
  default:
    os << "bad";
  }
  os << endl << "  _geom = 0x" << (void*)(this->_geom.p()) << endl;
  os << "  _arc = 0x" << (void*)(this->_arc) << endl;
  os << "  _tex = 0x" << (void*)(this->_tex.p()) << endl;
  os << "  _internal = 0x" << (void*)(this->_internal) << endl;
  os << "  _gset = 0x" << (void*)(this->_gset) << endl;
  os << "  _model_width = " << this->_model_width << endl;
  os << "  _model_height = " << this->_model_height << endl;
  os << "  _scale = " << this->_scale << endl;
  os << "  _pos = " << this->_pos << endl;
  os << "  _foreground = " << this->_foreground << endl;
  if (_have_background)
    os << "  _background = " << this->_background << endl;
  if (_have_width)
    os << "  _width = " << this->_width << endl;
  if (_have_height)
    os << "  _height = " << this->_height << endl;
}
