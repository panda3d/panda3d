// Filename: string_utils.I
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////


template<class Thing>
INLINE string
format_string(const Thing &thing) {
  ostringstream str;
  str << thing;
  return str.str();
}
