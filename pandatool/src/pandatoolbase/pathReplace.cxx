// Filename: pathReplace.cxx
// Created by:  drose (07Feb03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pathReplace.h"
#include "config_util.h"

////////////////////////////////////////////////////////////////////
//     Function: PathReplace::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PathReplace::
PathReplace() {
  _path_store = PS_keep;
}

////////////////////////////////////////////////////////////////////
//     Function: PathReplace::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PathReplace::
~PathReplace() {
}

////////////////////////////////////////////////////////////////////
//     Function: PathReplace::match_path
//       Access: Public
//  Description: Looks for a match for the given filename among all
//               the replacement patterns, and returns the first match
//               found.  If additional_path is nonempty, it is an
//               additional search path on which to look for the file.
//               The model_path is always implicitly searched.
////////////////////////////////////////////////////////////////////
Filename PathReplace::
match_path(const Filename &orig_filename, 
           const DSearchPath &additional_path) {
  Filename match;
  bool got_match = false;

  if (_entries.empty()) {
    // If we have no entries, still look up the file on the search
    // path (unless _path_store is PS_keep).
    if (_path_store != PS_keep) {
      Filename new_filename = orig_filename;
      if (new_filename.resolve_filename(_path) ||
          new_filename.resolve_filename(additional_path) ||
          new_filename.resolve_filename(get_model_path())) {
        // Found it!
        return new_filename;
      }
    }

  } else {
    Entries::const_iterator ei;
    for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
      const Entry &entry = (*ei);
      Filename new_filename;
      if (entry.try_match(orig_filename, new_filename)) {
        // The prefix matches.  Save the resulting filename for
        // posterity.
        got_match = true;
        match = new_filename;
        
        if (new_filename.is_fully_qualified()) {
          // If the resulting filename is fully qualified, it's a match
          // if and only if it exists.
          if (new_filename.exists()) {
            return new_filename;
          }
          
        } else {
          // Otherwise, if it's a relative filename, attempt to look it
          // up on the search path.
          if (new_filename.resolve_filename(_path) ||
              new_filename.resolve_filename(additional_path) ||
              new_filename.resolve_filename(get_model_path())) {
            // Found it!
            if (_path_store == PS_keep) {
              // If we asked to "keep" the pathname, we return the
              // matched path, but not the found path.
              return match;
            } else {
              // Otherwise, we return the actual, found path.
              return new_filename;
            }
          }
        }
        
        // The prefix matched, but it didn't exist.  Keep looking.
      }
    }
  }

  // The file couldn't be found anywhere.  Did we at least get any
  // prefix match?
  if (got_match) {
    return match;
  }

  // Nope, couldn't find anything.  Just return the original filename.
  return orig_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PathReplace::store_path
//       Access: Public
//  Description: Given a path to an existing filename, converts it as
//               specified in the _path_store and or _path_directory
//               properties to a form suitable for storing in an
//               output file.
////////////////////////////////////////////////////////////////////
Filename PathReplace::
store_path(const Filename &orig_filename) {
  if (_path_directory.is_local()) {
    _path_directory.make_absolute();
  }
  Filename filename = orig_filename;

  switch (_path_store) {
  case PS_relative:
    filename.make_absolute();
    filename.make_relative_to(_path_directory);
    break;

  case PS_absolute:
    filename.make_absolute();
    break;

  case PS_rel_abs:
    filename.make_absolute();
    filename.make_relative_to(_path_directory, false);
    break;

  case PS_strip:
    filename = filename.get_basename();
    break;

  case PS_keep:
    break;

  case PS_invalid:
    break;
  }

  return filename;
}

////////////////////////////////////////////////////////////////////
//     Function: PathReplace::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PathReplace::Entry::
Entry(const string &orig_prefix, const string &replacement_prefix) :
  _orig_prefix(orig_prefix),
  _replacement_prefix(replacement_prefix)
{
  // Eliminate trailing slashes; they're implicit.
  if (_orig_prefix.length() > 1 &&
      _orig_prefix[_orig_prefix.length() - 1] == '/') {
    _orig_prefix = _orig_prefix.substr(0, _orig_prefix.length() - 1);
  }
  if (_replacement_prefix.length() > 1 &&
      _replacement_prefix[_replacement_prefix.length() - 1] == '/') {
    _replacement_prefix = _replacement_prefix.substr(0, _replacement_prefix.length() - 1);
  }

  Filename filename(_orig_prefix);
  _is_local = filename.is_local();

  vector_string components;
  filename.extract_components(components);
  vector_string::const_iterator ci;
  for (ci = components.begin(); ci != components.end(); ++ci) {
    _orig_components.push_back(Component(*ci));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PathReplace::Entry::try_match
//       Access: Public
//  Description: Considers whether the indicated filename matches
//               this entry's prefix.  If so, switches the prefix and
//               stores the result in new_filename, and returns true;
//               otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool PathReplace::Entry::
try_match(const Filename &filename, Filename &new_filename) const {
  if (_is_local != filename.is_local()) {
    return false;
  }
  vector_string components;
  filename.extract_components(components);
  size_t mi = r_try_match(components, 0, 0);
  if (mi == 0) {
    // Sorry, no match.
    return false;
  }

  // We found a match.  Construct the replacement string.
  string result = _replacement_prefix;
  while (mi < components.size()) {
    if (!result.empty()) {
      result += '/';
    }
    result += components[mi];
    ++mi;
  }
  new_filename = result;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PathReplace::Entry::r_try_match
//       Access: Public
//  Description: The recursive implementation of try_match().
//               Actually, this is doubly-recursive, to implement the
//               "**" feature.
//
//               The return value is the number of the "components"
//               vector that successfully matched against all of the
//               orig_components.  (It's a variable number because
//               there might be one or more "**" entries.)
////////////////////////////////////////////////////////////////////
size_t PathReplace::Entry::
r_try_match(const vector_string &components, size_t oi, size_t ci) const {
  if (oi >= _orig_components.size()) {
    // If we ran out of user-supplied components, we're done.
    return ci;
  }
  if (ci >= components.size()) {
    // If we reached the end of the string, but we still have
    // user-supplied components, we failed.  (Arguably there should be
    // a special case here for a user-supplied string that ends in
    // "**", but I don't think the user ever wants to match the
    // complete string.)
    return 0;
  }

  const Component &orig_component = _orig_components[oi];
  if (orig_component._double_star) {
    // If we have a double star, first consider the match if it were
    // expanded as far as possible.
    size_t mi = r_try_match(components, oi, ci + 1);
    if (mi != 0) {
      return mi;
    }

    // Then try the match as if it there were no double star entry.
    return r_try_match(components, oi + 1, ci);
  }

  // We don't have a double star, it's just a one-for-one component
  // entry.  Does it match?
  if (orig_component._orig_prefix.matches(components[ci])) {
    // It does!  Keep going.
    return r_try_match(components, oi + 1, ci + 1);
  }

  // It doesn't match, sorry.
  return 0;
}
