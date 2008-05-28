// Filename: dcKeywordList.h
// Created by:  drose (25Jul05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef DCKEYWORDLIST_H
#define DCKEYWORDLIST_H

#include "dcbase.h"

class DCKeyword;
class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCKeywordList
// Description : This is a list of keywords (see DCKeyword) that may
//               be set on a particular field.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCKeywordList {
public:
  DCKeywordList();
  DCKeywordList(const DCKeywordList &copy);
  void operator = (const DCKeywordList &copy);
  ~DCKeywordList();

PUBLISHED:
  bool has_keyword(const string &name) const;
  bool has_keyword(const DCKeyword *keyword) const;
  int get_num_keywords() const;
  const DCKeyword *get_keyword(int n) const;
  const DCKeyword *get_keyword_by_name(const string &name) const;

  bool compare_keywords(const DCKeywordList &other) const;

public:
  void copy_keywords(const DCKeywordList &other);

  bool add_keyword(const DCKeyword *keyword);
  void clear_keywords();

  void output_keywords(ostream &out) const;
  void generate_hash(HashGenerator &hashgen) const;

private:
  typedef pvector<const DCKeyword *> Keywords;
  Keywords _keywords;

  typedef pmap<string, const DCKeyword *> KeywordsByName;
  KeywordsByName _keywords_by_name;

  int _flags;
};

#endif
