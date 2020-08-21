/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateDatabase.I
 * @author drose
 * @date 2000-08-01
 */

/**
 * Checks that all the latest data for all the libraries have been loaded.
 * Loads them if not.
 */
INLINE void InterrogateDatabase::
check_latest() {
  if (!_requests.empty()) {
    load_latest();
  }
}

/**
 * Returns the TypeIndex associated with the first type found with the given
 * name, or 0 if no type has this name.
 */
INLINE TypeIndex InterrogateDatabase::
lookup_type_by_name(const std::string &name) {
  check_latest();
  return lookup(name, _types_by_name, LT_type_name,
                &InterrogateDatabase::freshen_types_by_name);
}

/**
 * Returns the TypeIndex associated with the first type found with the given
 * scoped name, or 0 if no type has this name.
 */
INLINE TypeIndex InterrogateDatabase::
lookup_type_by_scoped_name(const std::string &name) {
  check_latest();
  return lookup(name, _types_by_scoped_name, LT_type_scoped_name,
                &InterrogateDatabase::freshen_types_by_scoped_name);
}

/**
 * Returns the TypeIndex associated with the first type found with the given
 * true name, or 0 if no type has this name.
 */
INLINE TypeIndex InterrogateDatabase::
lookup_type_by_true_name(const std::string &name) {
  check_latest();
  return lookup(name, _types_by_true_name, LT_type_true_name,
                &InterrogateDatabase::freshen_types_by_true_name);
}

/**
 * Returns the ManifestIndex associated with the first manifest found with the
 * given name, or 0 if no manifest has this name.
 */
INLINE ManifestIndex InterrogateDatabase::
lookup_manifest_by_name(const std::string &name) {
  check_latest();
  return lookup(name, _manifests_by_name, LT_manifest_name,
                &InterrogateDatabase::freshen_manifests_by_name);
}

/**
 * Returns the ElementIndex associated with the first element found with the
 * given name, or 0 if no element has this name.
 */
INLINE ElementIndex InterrogateDatabase::
lookup_element_by_name(const std::string &name) {
  check_latest();
  return lookup(name, _elements_by_name, LT_element_name,
                &InterrogateDatabase::freshen_elements_by_name);
}

/**
 * Returns the ElementIndex associated with the first element found with the
 * given scoped name, or 0 if no element has this name.
 */
INLINE ElementIndex InterrogateDatabase::
lookup_element_by_scoped_name(const std::string &name) {
  check_latest();
  return lookup(name, _elements_by_scoped_name, LT_element_scoped_name,
                &InterrogateDatabase::freshen_elements_by_scoped_name);
}
