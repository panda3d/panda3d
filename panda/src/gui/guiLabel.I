// Filename: guiLabel.I
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiLabel::GuiLabel(void) : _type(GuiLabel::NONE),
				  _arc((RenderRelation*)0L),
				  _tex((Texture*)0L),
				  _internal((RenderRelation*)0L),
				  _gset((Geom*)0L), _model_width(1.),
				  _model_height(1.), _scale(1.),
				  _scale_x(1.), _scale_y(1.), _scale_z(1.),
				  _pos(0., 0., 0.),
				  _foreground(1., 1., 1., 1.),
				  _have_background(false),
				  _background(0., 0., 0., 0.),
				  _have_width(false), _width(0.),
				  _have_height(false), _height(0.),
				  _mirror_x(false), _mirror_y(false),
				  _hard_pri(0), _highest_pri(false),
				  _lowest_pri(false) {
}

INLINE Node* GuiLabel::get_geometry(void) const {
  return _geom;
}

INLINE void GuiLabel::set_arc(RenderRelation* r) {
  _arc = r;
}

INLINE RenderRelation* GuiLabel::get_arc(void) const {
  return _arc;
}

INLINE void GuiLabel::set_width(float f) {
  if (f <= 0.) {
    _have_width = false;
    _width = 0.;
  } else {
    if (_type == MODEL)
      _width = f / this->get_width();
    else
      _width = f;
    _have_width = true;
  }
  this->set_properties();
}

INLINE void GuiLabel::set_height(float f) {
  if (f <= 0.) {
    _have_height = false;
    _height = 0.;
  } else {
    if (_type == MODEL)
      _height = f / this->get_height();
    else
      _height = f;
    _have_height = true;
  }
  this->set_properties();
}

INLINE void GuiLabel::set_scale(float f) {
  _scale = f;
  recompute_transform();
}

INLINE void GuiLabel::set_scale(float x, float y, float z) {
  _scale_x = x;
  _scale_y = y;
  _scale_z = z;
  recompute_transform();
}

INLINE void GuiLabel::set_mirror_x(bool b) {
  _mirror_x = b;
}

INLINE void GuiLabel::set_mirror_y(bool b) {
  _mirror_y = b;
}

INLINE void GuiLabel::set_pos(float x, float y, float z) {
  this->set_pos(LVector3f(x, y, z));
}

INLINE void GuiLabel::set_pos(const LVector3f& p) {
  _pos = p;
  recompute_transform();
}

INLINE float GuiLabel::get_scale(void) const {
  return _scale;
}

INLINE bool GuiLabel::get_mirror_x(void) const {
  return _mirror_x;
}

INLINE bool GuiLabel::get_mirror_y(void) const {
  return _mirror_y;
}

INLINE LVector3f GuiLabel::get_pos(void) const {
  return _pos;
}

INLINE void GuiLabel::set_foreground_color(float r, float g, float b,
					   float a) {
  this->set_foreground_color(Colorf(r, g, b, a));
}

INLINE void GuiLabel::set_background_color(float r, float g, float b,
					   float a) {
  this->set_background_color(Colorf(r, g, b, a));
}

INLINE Colorf GuiLabel::get_foreground_color(void) const {
  return _foreground;
}

INLINE Colorf GuiLabel::get_background_color(void) const {
  return _background;
}

INLINE void GuiLabel::set_shadow_color(float r, float g, float b, float a) {
  this->set_shadow_color(Colorf(r, g, b, a));
}

INLINE void GuiLabel::recompute(void) {
  this->recompute_transform();
}

INLINE void GuiLabel::set_priority(GuiLabel* l, const PriorityType t) {
  if (t == P_HIGHEST)
    _highest_pri = true;
  else if (t == P_LOWEST)
    _lowest_pri = true;
  else
    this->_priorities[l] = t;
}
