// Filename: builderAttrib.I
// Created by:  drose (22Jan99)
// 
////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
//     Function: BuilderAttrib::set_normal_value
//       Access: Public
//  Description: Reassigns the normal, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttrib::
set_normal_value(const BuilderN *array, ushort index) {
  set_normal(array[index]);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttrib::set_color_value
//       Access: Public
//  Description: Reassigns the color, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttrib::
set_color_value(const BuilderC *array, ushort index) {
  set_color(array[index]);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribI::set_normal_value
//       Access: Public
//  Description: Reassigns the normal, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttribI::
set_normal_value(const BuilderN *, ushort index) {
  set_normal(index);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribI::set_color_value
//       Access: Public
//  Description: Reassigns the color, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttribI::
set_color_value(const BuilderC *, ushort index) {
  set_color(index);
}
