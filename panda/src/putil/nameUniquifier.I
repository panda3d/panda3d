// Filename: nameUniquifier.I
// Created by:  drose (16Feb00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NameUniquifier::Constructor
//       Access: Public
//  Description: Creates a new NameUniquifier.  
//
//               The separator string is used to separate the original
//               name (or supplied prefix) and the generated number
//               when a name must be generated.
//
//               If the original name is empty, the empty string is
//               used, followed by the generated number.
////////////////////////////////////////////////////////////////////
INLINE NameUniquifier::
NameUniquifier(const string &separator,
	       const string &empty) :
  _separator(separator),
  _empty(empty)
{
  _counter = 0;

  if (_empty.empty()) {
    _empty = _separator;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NameUniquifier::add_name
//       Access: Public
//  Description: If name is nonempty and so far unique, returns it
//               unchanged.
//
//               Otherwise, generates and returns a new name according
//               to the following rules:
//
//               If the name is empty, the new name is the
//               NameUniquifier's "empty" string followed by a number,
//               or the "separator" string if the "empty" string is
//               empty.
//
//               If the name is nonempty, the new name is the original
//               name, followed by the NameUniquifier's "separator"
//               string, followed by a number.
////////////////////////////////////////////////////////////////////
INLINE string NameUniquifier::
add_name(const string &name) {
  return add_name_body(name, name);
}

////////////////////////////////////////////////////////////////////
//     Function: NameUniquifier::add_name
//       Access: Public
//  Description: If name is nonempty and so far unique, returns it
//               unchanged.
//
//               Otherwise, generates and returns a new name according
//               to the following rules:
//
//               If the prefix is empty, the new name is the
//               NameUniquifier's "empty" string followed by a number,
//               or the "separator" string if the "empty" string is
//               empty.
//
//               If the prefix is nonempty, the new name is the
//               prefix, followed by the NameUniquifier's "separator"
//               string, followed by a number.
////////////////////////////////////////////////////////////////////
INLINE string NameUniquifier::
add_name(const string &name, const string &prefix) {
  return add_name_body(name, prefix);
}
