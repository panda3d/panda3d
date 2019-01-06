/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pathReplace.cxx
 * @author drose
 * @date 2003-02-07
 */

#include "pathReplace.h"
#include "config_putil.h"
#include "config_pandatoolbase.h"
#include "indent.h"
#include "virtualFileSystem.h"

/**
 *
 */
PathReplace::
PathReplace() {
  _path_store = PS_keep;
  _copy_files = false;
  _noabs = false;
  _exists = false;
  _error_flag = false;
}

/**
 *
 */
PathReplace::
~PathReplace() {
}

/**
 * Looks for a match for the given filename among all the replacement
 * patterns, and returns the first match found.  If additional_path is
 * nonempty, it is an additional search path on which to look for the file.
 * The model_path is always implicitly searched.
 */
Filename PathReplace::
match_path(const Filename &orig_filename,
           const DSearchPath &additional_path) {
  Filename match;
  bool got_match = false;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    const Entry &entry = (*ei);
    Filename new_filename;
    if (entry.try_match(orig_filename, new_filename)) {
      // The prefix matches.  Save the resulting filename for posterity.
      got_match = true;
      match = new_filename;

      if (new_filename.is_fully_qualified()) {
        // If the resulting filename is fully qualified, it's a match if and
        // only if it exists.
        if (vfs->exists(new_filename)) {
          return new_filename;
        }

      } else {
        // Otherwise, if it's a relative filename, attempt to look it up on
        // the search path.
        if (vfs->resolve_filename(new_filename, _path) ||
            vfs->resolve_filename(new_filename, additional_path) ||
            vfs->resolve_filename(new_filename, get_model_path())) {
          // Found it!
          if (_path_store == PS_keep) {
            // If we asked to "keep" the pathname, we return the matched path,
            // but not the found path.
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

  // The file couldn't be found anywhere.  Did we at least get any prefix
  // match?
  if (got_match) {
    if (_exists) {
      _error_flag = true;
      pandatoolbase_cat.error()
        << "File does not exist: " << match << "\n";
    } else if (pandatoolbase_cat.is_debug()) {
      pandatoolbase_cat.debug()
        << "File does not exist: " << match << "\n";
    }

    return match;
  }

  if (!orig_filename.is_local()) {
    // Ok, we didn't match any specified prefixes.  If the file is an absolute
    // pathname and we have _noabs set, that's an error.
    if (_noabs) {
      _error_flag = true;
      pandatoolbase_cat.error()
        << "Absolute pathname: " << orig_filename << "\n";
    } else if (pandatoolbase_cat.is_debug()) {
      pandatoolbase_cat.debug()
        << "Absolute pathname: " << orig_filename << "\n";
    }
  }

  // Well, we still haven't found it; look it up on the search path as is.
  if (_path_store != PS_keep) {
    Filename new_filename = orig_filename;
    if (vfs->resolve_filename(new_filename, _path) ||
        vfs->resolve_filename(new_filename, additional_path) ||
        vfs->resolve_filename(new_filename, get_model_path())) {
      // Found it!
      return new_filename;
    }
  }

  // Nope, couldn't find anything.  This is an error, but just return the
  // original filename.
  if (_exists) {
    _error_flag = true;
    pandatoolbase_cat.error()
      << "File does not exist: " << orig_filename << "\n";
  } else if (pandatoolbase_cat.is_debug()) {
    pandatoolbase_cat.debug()
      << "File does not exist: " << orig_filename << "\n";
  }
  return orig_filename;
}

/**
 * Given a path to an existing filename, converts it as specified in the
 * _path_store and or _path_directory properties to a form suitable for
 * storing in an output file.
 */
Filename PathReplace::
store_path(const Filename &orig_filename) {
  if (orig_filename.empty()) {
    return orig_filename;
  }

  if (_path_directory.is_local()) {
    _path_directory.make_absolute();
  }
  Filename filename = orig_filename;

  if (_copy_files) {
    copy_this_file(filename);
  }

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

/**
 * Converts the input path into two different forms: A resolved path, and an
 * output path.  The resolved path is an absolute path if at all possible.
 * The output path is in the form specified by the -ps path store option.
 */
void PathReplace::
full_convert_path(const Filename &orig_filename,
                  const DSearchPath &additional_path,
                  Filename &resolved_path,
                  Filename &output_path) {
  if (_path_directory.is_local()) {
    _path_directory.make_absolute();
  }

  Filename match;
  bool got_match = false;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    const Entry &entry = (*ei);
    Filename new_filename;
    if (entry.try_match(orig_filename, new_filename)) {
      // The prefix matches.  Save the resulting filename for posterity.
      got_match = true;
      match = new_filename;

      if (new_filename.is_fully_qualified()) {
        // If the resulting filename is fully qualified, it's a match if and
        // only if it exists.
        if (vfs->exists(new_filename)) {
          resolved_path = new_filename;
          goto calculate_output_path;
        }

      } else {
        // Otherwise, if it's a relative filename, attempt to look it up on
        // the search path.
        if (vfs->resolve_filename(new_filename, _path) ||
            vfs->resolve_filename(new_filename, additional_path) ||
            vfs->resolve_filename(new_filename, get_model_path())) {
          // Found it!
          resolved_path = new_filename;
          goto calculate_output_path;
        }
      }

      // The prefix matched, but it didn't exist.  Keep looking.
    }
  }

  // The file couldn't be found anywhere.  Did we at least get any prefix
  // match?
  if (got_match) {
    if (_exists) {
      _error_flag = true;
      pandatoolbase_cat.error()
        << "File does not exist: " << match << "\n";
    } else if (pandatoolbase_cat.is_debug()) {
      pandatoolbase_cat.debug()
        << "File does not exist: " << match << "\n";
    }

    resolved_path = match;
    goto calculate_output_path;
  }

  if (!orig_filename.is_local()) {
    // Ok, we didn't match any specified prefixes.  If the file is an absolute
    // pathname and we have _noabs set, that's an error.
    if (_noabs) {
      _error_flag = true;
      pandatoolbase_cat.error()
        << "Absolute pathname: " << orig_filename << "\n";
    } else if (pandatoolbase_cat.is_debug()) {
      pandatoolbase_cat.debug()
        << "Absolute pathname: " << orig_filename << "\n";
    }
  }

  // Well, we still haven't found it; look it up on the search path as is.
  {
    Filename new_filename = orig_filename;
    if (vfs->resolve_filename(new_filename, _path) ||
        vfs->resolve_filename(new_filename, additional_path) ||
        vfs->resolve_filename(new_filename, get_model_path())) {
      // Found it!
      match = orig_filename;
      resolved_path = new_filename;
      goto calculate_output_path;
    }
  }

  // Nope, couldn't find anything.  This is an error, but just return the
  // original filename.
  if (_exists) {
    _error_flag = true;
    pandatoolbase_cat.error()
      << "File does not exist: " << orig_filename << "\n";
  } else if (pandatoolbase_cat.is_debug()) {
    pandatoolbase_cat.debug()
      << "File does not exist: " << orig_filename << "\n";
  }
  match = orig_filename;
  resolved_path = orig_filename;

  // To calculate the output path, we need two inputs: the match, and the
  // resolved path.  Which one is used depends upon the path-store mode.
 calculate_output_path:

  if (_copy_files) {
    if (copy_this_file(resolved_path)) {
      match = resolved_path;
    }
  }

  switch (_path_store) {
  case PS_relative:
    if (resolved_path.empty())
      output_path = resolved_path;
    else {
      output_path = resolved_path;
      output_path.make_absolute();
      output_path.make_relative_to(_path_directory);
    }
    break;

  case PS_absolute:
    if (resolved_path.empty())
      output_path = resolved_path;
    else {
      output_path = resolved_path;
      output_path.make_absolute();
    }
    break;

  case PS_rel_abs:
    if (resolved_path.empty())
      output_path = resolved_path;
    else {
      output_path = resolved_path;
      output_path.make_absolute();
      output_path.make_relative_to(_path_directory, false);
    }
    break;

  case PS_strip:
    output_path = match.get_basename();
    break;

  case PS_keep:
    output_path = match;
    break;

  case PS_invalid:
    output_path = "";
    break;
  }
}

/**
 *
 */
void PathReplace::
write(std::ostream &out, int indent_level) const {
  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    indent(out, indent_level)
      << "-pr " << (*ei)._orig_prefix << "="
      << (*ei)._replacement_prefix << "\n";
  }
  int num_directories = _path.get_num_directories();
  for (int i = 0; i < num_directories; i++) {
    indent(out, indent_level)
      << "-pp " << _path.get_directory(i) << "\n";
  }
  indent(out, indent_level)
    << "-ps " << _path_store << "\n";

  // The path directory is only relevant if _path_store is rel or rel_abs.
  switch (_path_store) {
  case PS_relative:
  case PS_rel_abs:
    indent(out, indent_level)
      << "-pd " << _path_directory << "\n";

  default:
    break;
  }

  if (_copy_files) {
    indent(out, indent_level)
      << "-pc " << _copy_into_directory << "\n";
  }

  if (_noabs) {
    indent(out, indent_level)
      << "-noabs\n";
  }
}

/**
 * Copies the indicated file into the copy_into_directory, and adjusts
 * filename to reference the new location.  Returns true if the copy is made
 * and the filename is changed, false otherwise.
 */
bool PathReplace::
copy_this_file(Filename &filename) {
  if (_copy_into_directory.is_local()) {
    _copy_into_directory = Filename(_path_directory, _copy_into_directory);
  }

  Copied::iterator ci = _orig_to_target.find(filename);
  if (ci != _orig_to_target.end()) {
    // This file has already been successfully copied, so we can quietly
    // return its new target filename.
    if (filename != (*ci).second) {
      filename = (*ci).second;
      return true;
    }
    return false;
  }

  Filename target_filename(_copy_into_directory, filename.get_basename());
  ci = _target_to_orig.find(target_filename);
  if (ci != _target_to_orig.end()) {
    if ((*ci).second != filename) {
      _error_flag = true;
      pandatoolbase_cat.error()
        << "Filename conflict!  Both " << (*ci).second << " and "
        << filename << " map to " << target_filename << "\n";
    }

    // Don't copy this one.
    _orig_to_target[filename] = filename;
    return false;
  }

  _orig_to_target[filename] = target_filename;
  _target_to_orig[target_filename] = filename;

  // Make the copy.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->make_directory_full(_copy_into_directory);
  if (!vfs->copy_file(filename, target_filename)) {
    _error_flag = true;
    pandatoolbase_cat.error()
      << "Cannot copy file from " << filename << " to " << target_filename
      << "\n";
    _orig_to_target[filename] = filename;
    return false;
  }

  filename = target_filename;
  return true;
}

/**
 *
 */
PathReplace::Entry::
Entry(const std::string &orig_prefix, const std::string &replacement_prefix) :
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

/**
 * Considers whether the indicated filename matches this entry's prefix.  If
 * so, switches the prefix and stores the result in new_filename, and returns
 * true; otherwise, returns false.
 */
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
  std::string result = _replacement_prefix;
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

/**
 * The recursive implementation of try_match(). Actually, this is doubly-
 * recursive, to implement the "**" feature.
 *
 * The return value is the number of the "components" vector that successfully
 * matched against all of the orig_components.  (It's a variable number
 * because there might be one or more "**" entries.)
 */
size_t PathReplace::Entry::
r_try_match(const vector_string &components, size_t oi, size_t ci) const {
  if (oi >= _orig_components.size()) {
    // If we ran out of user-supplied components, we're done.
    return ci;
  }
  if (ci >= components.size()) {
    // If we reached the end of the string, but we still have user-supplied
    // components, we failed.  (Arguably there should be a special case here
    // for a user-supplied string that ends in "**", but I don't think the
    // user ever wants to match the complete string.)
    return 0;
  }

  const Component &orig_component = _orig_components[oi];
  if (orig_component._double_star) {
    // If we have a double star, first consider the match if it were expanded
    // as far as possible.
    size_t mi = r_try_match(components, oi, ci + 1);
    if (mi != 0) {
      return mi;
    }

    // Then try the match as if it there were no double star entry.
    return r_try_match(components, oi + 1, ci);
  }

  // We don't have a double star, it's just a one-for-one component entry.
  // Does it match?
  if (orig_component._orig_prefix.matches(components[ci])) {
    // It does!  Keep going.
    return r_try_match(components, oi + 1, ci + 1);
  }

  // It doesn't match, sorry.
  return 0;
}
