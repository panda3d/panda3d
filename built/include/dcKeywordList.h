/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcKeywordList.h
 * @author drose
 * @date 2005-07-25
 */

#ifndef DCKEYWORDLIST_H
#define DCKEYWORDLIST_H

#include "dcbase.h"

class DCKeyword;
class HashGenerator;

/**
 * This is a list of keywords (see DCKeyword) that may be set on a particular
 * field.
 */
class EXPCL_DIRECT_DCPARSER DCKeywordList {
public:
  DCKeywordList();
  DCKeywordList(const DCKeywordList &copy);
  void operator = (const DCKeywordList &copy);
  ~DCKeywordList();

PUBLISHED:
  bool has_keyword(const std::string &name) const;
  bool has_keyword(const DCKeyword *keyword) const;
  int get_num_keywords() const;
  const DCKeyword *get_keyword(int n) const;
  const DCKeyword *get_keyword_by_name(const std::string &name) const;

  bool compare_keywords(const DCKeywordList &other) const;

public:
  void copy_keywords(const DCKeywordList &other);

  bool add_keyword(const DCKeyword *keyword);
  void clear_keywords();

  void output_keywords(std::ostream &out) const;
  void generate_hash(HashGenerator &hashgen) const;

private:
  typedef pvector<const DCKeyword *> Keywords;
  Keywords _keywords;

  typedef pmap<std::string, const DCKeyword *> KeywordsByName;
  KeywordsByName _keywords_by_name;

  int _flags;
};

#endif
