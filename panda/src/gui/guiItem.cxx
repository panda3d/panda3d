// Filename: guiItem.cxx
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "guiItem.h"

TypeHandle GuiItem::_type_handle;

void GuiItem::recompute_frame(void) {
  test_ref_count_integrity();
}

void GuiItem::adjust_region(void) {
  test_ref_count_integrity();
  gui_cat->debug() << "in adjust_region, base values (" << _left << ", "
		   << _right << ", " << _bottom << ", " << _top << ")" << endl;
  if (!(_alt_root.is_null())) {
    // adjust for graph transform
    LMatrix4f m;
    this->get_graph_mat(m);
    LPoint3f ul = LVector3f::rfu(_left, 0., _top);
    LPoint3f lr = LVector3f::rfu(_right, 0., _bottom);
    ul = ul * m;
    lr = lr * m ;
    _left = ul.dot(LVector3f::rfu(1., 0., 0.));
    _top = ul.dot(LVector3f::rfu(0., 0., 1.));
    _right = lr.dot(LVector3f::rfu(1., 0., 0.));
    _bottom = lr.dot(LVector3f::rfu(0., 0., 1.));
    gui_cat->debug() << "childed to non-default node, current values ("
		     << _left << ", " << _right << ", " << _bottom << ", "
		     << _top << ")" << endl;
  }
}

void GuiItem::set_priority(GuiLabel*, const GuiItem::Priority) {
}

GuiItem::GuiItem(const string& name) : Namable(name), _added_hooks(false),
				       _scale(1.), _scale_x(1.), _scale_y(1.),
				       _scale_z(1.), _left(-1.), _right(1.),
				       _bottom(-1.), _top(1.),
				       _pos(0., 0., 0.),
				       _mgr((GuiManager*)0L), _pri(P_Normal) {

  if (gui_cat->is_debug())
    gui_cat->debug()
      << "creating item '" << get_name() << "' (" << (void *)this << ")\n";
}

GuiItem::~GuiItem(void) {
  if (gui_cat->is_debug())
    gui_cat->debug()
      << "deleting item '" << get_name() << "' (" << (void *)this << ")\n";
  //  this->unmanage();
}

int GuiItem::freeze() {
  return 0;
}

int GuiItem::thaw() {
  gui_cat->debug() << "GuiItem::thaw" << endl;
  return 0;
}

void GuiItem::manage(GuiManager* mgr, EventHandler&) {
  test_ref_count_integrity();
  _mgr = mgr;
  _alt_root.clear();
}

void GuiItem::manage(GuiManager* mgr, EventHandler&, Node* n) {
  test_ref_count_integrity();
  _mgr = mgr;
  _alt_root = n;
  this->adjust_region();
}

void GuiItem::unmanage(void) {
  test_ref_count_integrity();
  _mgr = (GuiManager*)0L;
}

void GuiItem::set_scale(float f) {
  _scale = f;
}

void GuiItem::set_scale(float x, float y, float z) {
  _scale_x = x;
  _scale_y = y;
  _scale_z = z;
}

void GuiItem::set_pos(const LVector3f& p) {
  _pos = p;
}

void GuiItem::set_priority(GuiItem* i, const GuiItem::Priority p) {
  if (_mgr != (GuiManager*)0L)
    _mgr->recompute_priorities();
}

int GuiItem::set_draw_order(int v) {
  return v;
}

void GuiItem::output(ostream& os) const {
  os << "GuiItem (0x" << (void*)this << ")" << endl;
  os << "  name - '" << get_name() << "'" << endl;
  os << "  hooks have" << (_added_hooks?" ":" not ") << "been added" << endl;
  os << "  scale - " << _scale << endl;
  os << "  pos - " << _pos << endl;
  os << "  mgr - 0x" << (void*)_mgr << endl;
  os << "  frame - (" << _left << ", " << _right << ", " << _bottom << ", "
     << _top << ")" << endl;
}
