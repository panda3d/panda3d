// Filename: guiLabel.I
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiLabel::GuiLabel(void) : _type(GuiLabel::NONE),
				  _arc((RenderRelation*)0L),
				  _tex((Texture*)0L),
				  _internal((RenderRelation*)0L), _scale(1.),
				  _pos(0., 0., 0.),
				  _foreground(1., 1., 1., 1.),
				  _have_background(false),
				  _background(0., 0., 0., 0.) {
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

INLINE void GuiLabel::set_scale(float f) {
  _scale = f;
  recompute_transform();
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
