// Filename: textureStage.I
// Created by:  masad (15Jul04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_name
//       Access: Published
//  Description: Returns the name of this texture stage
////////////////////////////////////////////////////////////////////
INLINE const string &TextureStage::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_name
//       Access: Published
//  Description: Changes the name of this texture stage
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_name(const string &name) {
  _name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_sort
//       Access: Published
//  Description: Changes the order in which the texture associated
//               with this stage is rendered relative to the other
//               texture stages.  When geometry is rendered with
//               multiple textures, the textures are rendered in order
//               from the lowest sort number to the highest sort
//               number.
//
//               Also see set_priority(), which is used to select the
//               most important textures for rendering when some must
//               be omitted because of hardware limitations.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_sort(int sort) {
  _sort = sort;

  // Update the global flag to indicate that all TextureAttribs in the
  // world must now re-sort their lists.
  _sort_seq++;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_sort
//       Access: Published
//  Description: Returns the sort order of this texture stage.
////////////////////////////////////////////////////////////////////
INLINE int TextureStage::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_priority
//       Access: Published
//  Description: Changes the relative importance of the texture
//               associated with this stage relative to the other
//               texture stages that are applied simultaneously.
//
//               This is unrelated to set_sort(), which controls the
//               order in which multiple textures are applied.  The
//               priority number is used to decide which of the
//               requested textures are to be selected for rendering
//               when more textures are requested than the hardware
//               will support.  The highest-priority n textures are
//               selected for rendering, and then rendered in order by
//               their sort factor.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_priority(int priority) {
  _priority = priority;

  // Update the global flag to indicate that all TextureAttribs in the
  // world must now re-sort their lists.
  _sort_seq++;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_priority
//       Access: Published
//  Description: Returns the priority associated with this stage.
//
//               This is specially helpful for cards that do not
//               support more than n stages of multi-texturing.
////////////////////////////////////////////////////////////////////
INLINE int TextureStage::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::opertor <
//       Access: Published
//  Description: Compare if the sort order is lower
////////////////////////////////////////////////////////////////////
INLINE bool TextureStage::
operator < (const TextureStage &other) const {
  return (_sort < other._sort);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::opertor =
//       Access: Published
//  Description: just copy the members of other to this
////////////////////////////////////////////////////////////////////
INLINE TextureStage &TextureStage::
operator = (const TextureStage &other) {
  _name = other._name;
  _sort = other._sort;
  _priority = other._priority;
  _texcoord_name = other._texcoord_name;
  _mode = other._mode;
  _color = other._color;
  _combine_rgb_mode = other._combine_rgb_mode;
  _combine_rgb_source0 = other._combine_rgb_source0;
  _combine_rgb_operand0 = other._combine_rgb_operand0;
  _combine_rgb_source1 = other._combine_rgb_source1;
  _combine_rgb_operand1 = other._combine_rgb_operand1;
  _combine_rgb_source2 = other._combine_rgb_source2;
  _combine_rgb_operand2 = other._combine_rgb_operand2;
  _combine_alpha_mode = other._combine_alpha_mode;
  _combine_alpha_source0 = other._combine_alpha_source0;
  _combine_alpha_operand0 = _combine_alpha_operand0;
  _combine_alpha_source1 = other._combine_alpha_source1;
  _combine_alpha_operand1 = other._combine_alpha_operand1;
  _combine_alpha_source2 = other._combine_alpha_source2;
  _combine_alpha_operand2 = other._combine_alpha_operand2;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_texcoord_name
//       Access: Published
//  Description: Indicate which set of UV's this texture stage will
//               use.  Geometry may have any number of associated UV
//               sets, each of which must have a unique name.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_texcoord_name(const TexCoordName *name) {
  _texcoord_name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_texcoord_name
//       Access: Published
//  Description: Indicate which set of UV's this texture stage will
//               use.  Geometry may have any number of associated UV
//               sets, each of which must have a unique name.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_texcoord_name(const string &name) {
  _texcoord_name = TexCoordName::make(name);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_texcoord_name
//       Access: Published
//  Description: Returns the TexCoordName
////////////////////////////////////////////////////////////////////
INLINE const TexCoordName *TextureStage::
get_texcoord_name() const {
  return _texcoord_name;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_mode
//       Access: Published
//  Description: Set the mode of this texture stage
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_mode(TextureStage::Mode mode) {
  _mode = mode;

  if (_mode != M_combine) {
    _num_combine_rgb_operands = 0;
    _num_combine_alpha_operands = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_mode
//       Access: Published
//  Description: Return the mode of this stage
////////////////////////////////////////////////////////////////////
INLINE TextureStage::Mode TextureStage::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_color
//       Access: Published
//  Description: Set the color for this stage
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_color(const Colorf &color) {
  _color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_color
//       Access: Published
//  Description: return the color for this stage
////////////////////////////////////////////////////////////////////
INLINE Colorf TextureStage::
get_color() const {
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_combine_rgb
//       Access: Published
//  Description: Specifies any of the CombineMode values that
//               represent a one-parameter operation.  Specifically,
//               this is CM_replace only.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_combine_rgb(CombineMode mode, 
                CombineSource source0, CombineOperand operand0) {
  nassertv(get_expected_num_combine_operands(mode) == 1);
  nassertv(operand_valid_for_rgb(operand0));
  _mode = M_combine;
  _num_combine_rgb_operands = 1;
  _combine_rgb_mode = mode;
  _combine_rgb_source0 = source0;
  _combine_rgb_operand0 = operand0;
  _combine_rgb_source1 = CS_undefined;
  _combine_rgb_operand1 = CO_undefined;
  _combine_rgb_source2 = CS_undefined;
  _combine_rgb_operand2 = CO_undefined;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_combine_rgb
//       Access: Published
//  Description: Specifies any of the CombineMode values that
//               represent a two-parameter operation.  Specifically,
//               this is everything except for CM_replace and
//               CM_interpolate.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_combine_rgb(CombineMode mode, 
                CombineSource source0, CombineOperand operand0,
                CombineSource source1, CombineOperand operand1) {
  nassertv(get_expected_num_combine_operands(mode) == 2);
  nassertv(operand_valid_for_rgb(operand0));
  nassertv(operand_valid_for_rgb(operand1));
  _mode = M_combine;
  _num_combine_rgb_operands = 2;
  _combine_rgb_mode = mode;
  _combine_rgb_source0 = source0;
  _combine_rgb_operand0 = operand0;
  _combine_rgb_source1 = source1;
  _combine_rgb_operand1 = operand1;
  _combine_rgb_source2 = CS_undefined;
  _combine_rgb_operand2 = CO_undefined;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_combine_rgb
//       Access: Published
//  Description: Specifies any of the CombineMode values that
//               represent a one-parameter operation.  Specifically,
//               this is CM_interpolate only.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_combine_rgb(CombineMode mode, 
                CombineSource source0, CombineOperand operand0,
                CombineSource source1, CombineOperand operand1,
                CombineSource source2, CombineOperand operand2) {
  nassertv(get_expected_num_combine_operands(mode) == 3);
  nassertv(operand_valid_for_rgb(operand0));
  nassertv(operand_valid_for_rgb(operand1));
  nassertv(operand_valid_for_rgb(operand2));
  _mode = M_combine;
  _num_combine_rgb_operands = 3;
  _combine_rgb_mode = mode;
  _combine_rgb_source0 = source0;
  _combine_rgb_operand0 = operand0;
  _combine_rgb_source1 = source1;
  _combine_rgb_operand1 = operand1;
  _combine_rgb_source2 = source2;
  _combine_rgb_operand2 = operand2;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_rgb_mode
//       Access: Published
//  Description: Get the combine_rgb_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineMode TextureStage::
get_combine_rgb_mode() const {
  return _combine_rgb_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_num_combine_rgb_operands
//       Access: Published
//  Description: Returns the number of meaningful operands that may be
//               retrieved via get_combine_rgb_sourceN() and
//               get_combine_rgb_operandN().
////////////////////////////////////////////////////////////////////
INLINE int TextureStage::
get_num_combine_rgb_operands() const {
  return _num_combine_rgb_operands;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_rgb_source0
//       Access: Published
//  Description: Get source0 of combine_rgb_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineSource TextureStage::
get_combine_rgb_source0() const {
  return _combine_rgb_source0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_rgb_operand0
//       Access: Published
//  Description: Get operand0 of combine_rgb_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineOperand TextureStage::
get_combine_rgb_operand0() const {
  return _combine_rgb_operand0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_rgb_source1
//       Access: Published
//  Description: Get source1 of combine_rgb_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineSource TextureStage::
get_combine_rgb_source1() const {
  return _combine_rgb_source1;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_rgb_operand1
//       Access: Published
//  Description: Get operand1 of combine_rgb_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineOperand TextureStage::
get_combine_rgb_operand1() const {
  return _combine_rgb_operand1;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_rgb_source2
//       Access: Published
//  Description: Get source2 of combine_rgb_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineSource TextureStage::
get_combine_rgb_source2() const {
  return _combine_rgb_source2;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_rgb_operand2
//       Access: Published
//  Description: Get operand2 of combine_rgb_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineOperand TextureStage::
get_combine_rgb_operand2() const {
  return _combine_rgb_operand2;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_combine_alpha
//       Access: Published
//  Description: Specifies any of the CombineMode values that
//               represent a one-parameter operation.  Specifically,
//               this is CM_replace only.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_combine_alpha(CombineMode mode, 
                  CombineSource source0, CombineOperand operand0) {
  nassertv(get_expected_num_combine_operands(mode) == 1);
  nassertv(operand_valid_for_alpha(operand0));
  _mode = M_combine;
  _num_combine_alpha_operands = 1;
  _combine_alpha_mode = mode;
  _combine_alpha_source0 = source0;
  _combine_alpha_operand0 = operand0;
  _combine_alpha_source1 = CS_undefined;
  _combine_alpha_operand1 = CO_undefined;
  _combine_alpha_source2 = CS_undefined;
  _combine_alpha_operand2 = CO_undefined;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_combine_alpha
//       Access: Published
//  Description: Specifies any of the CombineMode values that
//               represent a two-parameter operation.  Specifically,
//               this is everything except for CM_replace and
//               CM_interpolate.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_combine_alpha(CombineMode mode, 
                  CombineSource source0, CombineOperand operand0,
                  CombineSource source1, CombineOperand operand1) {
  nassertv(get_expected_num_combine_operands(mode) == 2);
  nassertv(operand_valid_for_alpha(operand0));
  nassertv(operand_valid_for_alpha(operand1));
  _mode = M_combine;
  _num_combine_alpha_operands = 2;
  _combine_alpha_mode = mode;
  _combine_alpha_source0 = source0;
  _combine_alpha_operand0 = operand0;
  _combine_alpha_source1 = source1;
  _combine_alpha_operand1 = operand1;
  _combine_alpha_source2 = CS_undefined;
  _combine_alpha_operand2 = CO_undefined;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::set_combine_alpha
//       Access: Published
//  Description: Specifies any of the CombineMode values that
//               represent a one-parameter operation.  Specifically,
//               this is CM_interpolate only.
////////////////////////////////////////////////////////////////////
INLINE void TextureStage::
set_combine_alpha(CombineMode mode, 
                  CombineSource source0, CombineOperand operand0,
                  CombineSource source1, CombineOperand operand1,
                  CombineSource source2, CombineOperand operand2) {
  nassertv(get_expected_num_combine_operands(mode) == 3);
  nassertv(operand_valid_for_alpha(operand0));
  nassertv(operand_valid_for_alpha(operand1));
  nassertv(operand_valid_for_alpha(operand2));
  _mode = M_combine;
  _num_combine_alpha_operands = 3;
  _combine_alpha_mode = mode;
  _combine_alpha_source0 = source0;
  _combine_alpha_operand0 = operand0;
  _combine_alpha_source1 = source1;
  _combine_alpha_operand1 = operand1;
  _combine_alpha_source2 = source2;
  _combine_alpha_operand2 = operand2;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_alpha
//       Access: Published
//  Description: Get combine_alpha_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineMode TextureStage::
get_combine_alpha_mode() const {
  return _combine_alpha_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_num_combine_alpha_operands
//       Access: Published
//  Description: Returns the number of meaningful operands that may be
//               retrieved via get_combine_alpha_sourceN() and
//               get_combine_alpha_operandN().
////////////////////////////////////////////////////////////////////
INLINE int TextureStage::
get_num_combine_alpha_operands() const {
  return _num_combine_alpha_operands;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_alpha_source0
//       Access: Published
//  Description: Get source0 of combine_alpha_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineSource TextureStage::
get_combine_alpha_source0() const {
  return _combine_alpha_source0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_alpha_operand0
//       Access: Published
//  Description: Get operand0 of combine_alpha_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineOperand TextureStage::
get_combine_alpha_operand0() const {
  return _combine_alpha_operand0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_alpha_source1
//       Access: Published
//  Description: Get source1 of combine_alpha_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineSource TextureStage::
get_combine_alpha_source1() const {
  return _combine_alpha_source1;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_alpha_operand1
//       Access: Published
//  Description: Get operand1 of combine_alpha_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineOperand TextureStage::
get_combine_alpha_operand1() const {
  return _combine_alpha_operand1;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_alpha_source2
//       Access: Published
//  Description: Get source2 of combine_alpha_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineSource TextureStage::
get_combine_alpha_source2() const {
  return _combine_alpha_source2;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_combine_alpha_operand2
//       Access: Published
//  Description: Get operand2 of combine_alpha_mode
////////////////////////////////////////////////////////////////////
INLINE TextureStage::CombineOperand TextureStage::
get_combine_alpha_operand2() const {
  return _combine_alpha_operand2;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_default
//       Access: Published, Static
//  Description: Returns the default TextureStage that will be used
//               for all texturing that does not name a particular
//               stage.  This generally handles the normal
//               single-texture case.
////////////////////////////////////////////////////////////////////
INLINE TextureStage *TextureStage::
get_default() {
  if (_default_stage == (TextureStage *)NULL) {
    _default_stage = new TextureStage("default");
  }
  return _default_stage;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStage::get_sort_seq
//       Access: Published, Static
//  Description: Returns a global sequence number that is incremented
//               any time any TextureStage in the world changes sort
//               or priority.  This is used by TextureAttrib to
//               determine when it is necessary to re-sort its
//               internal array of stages.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq TextureStage::
get_sort_seq() {
  return _sort_seq;
}

INLINE ostream &
operator << (ostream &out, const TextureStage &ts) {
  ts.output(out);
  return out;
}
