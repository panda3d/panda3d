// Filename: guiLabel.I
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiLabel::GuiLabel(void) : _type(GuiLabel::NONE), _scale(1.),
				  _pos(0., 0., 0.) {
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
