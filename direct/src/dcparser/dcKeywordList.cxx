/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcKeywordList.cxx
 * @author drose
 * @date 2005-07-25
 */

#include "dcKeywordList.h"
#include "dcKeyword.h"
#include "hashGenerator.h"

/**
 *
 */
DCKeywordList::
DCKeywordList() :
  _flags(0)
{
}

/**
 *
 */
DCKeywordList::
DCKeywordList(const DCKeywordList &copy) :
  _keywords(copy._keywords),
  _keywords_by_name(copy._keywords_by_name),
  _flags(copy._flags)
{
}

/**
 *
 */
void DCKeywordList::
operator = (const DCKeywordList &copy) {
  _keywords = copy._keywords;
  _keywords_by_name = copy._keywords_by_name;
  _flags = copy._flags;
}

/**
 *
 */
DCKeywordList::
~DCKeywordList() {
  nassertv(_keywords_by_name.size() == _keywords.size());
}

/**
 * Returns true if this list includes the indicated keyword, false otherwise.
 */
bool DCKeywordList::
has_keyword(const std::string &name) const {
  return (_keywords_by_name.find(name) != _keywords_by_name.end());
}

/**
 * Returns true if this list includes the indicated keyword, false otherwise.
 */
bool DCKeywordList::
has_keyword(const DCKeyword *keyword) const {
  return has_keyword(keyword->get_name());
}

/**
 * Returns the number of keywords in the list.
 */
int DCKeywordList::
get_num_keywords() const {
  nassertr(_keywords_by_name.size() == _keywords.size(), 0);
  return _keywords.size();
}

/**
 * Returns the nth keyword in the list.
 */
const DCKeyword *DCKeywordList::
get_keyword(int n) const {
  nassertr(n >= 0 && n < (int)_keywords.size(), nullptr);
  return _keywords[n];
}

/**
 * Returns the keyword in the list with the indicated name, or NULL if there
 * is no keyword in the list with that name.
 */
const DCKeyword *DCKeywordList::
get_keyword_by_name(const std::string &name) const {
  KeywordsByName::const_iterator ni;
  ni = _keywords_by_name.find(name);
  if (ni != _keywords_by_name.end()) {
    return (*ni).second;
  }

  return nullptr;
}

/**
 * Returns true if this list has the same keywords as the other list, false if
 * some keywords differ.  Order is not considered important.
 */
bool DCKeywordList::
compare_keywords(const DCKeywordList &other) const {
  return _keywords_by_name == other._keywords_by_name;
}

/**
 * Replaces this keyword list with those from the other list.
 */
void DCKeywordList::
copy_keywords(const DCKeywordList &other) {
  (*this) = other;
}

/**
 * Adds the indicated keyword to the list.  Returns true if it is added, false
 * if it was already there.
 */
bool DCKeywordList::
add_keyword(const DCKeyword *keyword) {
  bool inserted = _keywords_by_name.insert(KeywordsByName::value_type(keyword->get_name(), keyword)).second;
  if (inserted) {
    _keywords.push_back(keyword);
    _flags |= keyword->get_historical_flag();
  }

  return inserted;
}

/**
 * Removes all keywords from the field.
 */
void DCKeywordList::
clear_keywords() {
  _keywords.clear();
  _keywords_by_name.clear();
  _flags = 0;
}

/**
 *
 */
void DCKeywordList::
output_keywords(std::ostream &out) const {
  Keywords::const_iterator ki;
  for (ki = _keywords.begin(); ki != _keywords.end(); ++ki) {
    out << " " << (*ki)->get_name();
  }
}

/**
 * Accumulates the properties of these keywords into the hash.
 */
void DCKeywordList::
generate_hash(HashGenerator &hashgen) const {
  if (_flags != ~0) {
    // All of the flags are historical flags only, so add just the flags
    // bitmask to keep the hash code the same as it has historically been.
    hashgen.add_int(_flags);

  } else {
    // There is at least one custom flag, so go ahead and make the hash code
    // reflect it.

    hashgen.add_int(_keywords_by_name.size());
    KeywordsByName::const_iterator ni;
    for (ni = _keywords_by_name.begin(); ni != _keywords_by_name.end(); ++ni) {
      (*ni).second->generate_hash(hashgen);
    }
  }
}
