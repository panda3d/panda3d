// Filename: guiLabel.I
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiLabel::GuiLabel(void) : _type(GuiLabel::NONE) {
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
