/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iterator_types.h
 * @author drose
 * @date 1999-02-10
 */

#ifndef ITERATOR_TYPES_H
#define ITERATOR_TYPES_H

#include "dtoolbase.h"

/**
 * This is an iterator adaptor that converts any iterator that returns a pair
 * (e.g.  a map iterator) into one that returns just the first component of
 * that pair.
 */
template<class pair_iterator>
class first_of_pair_iterator : public pair_iterator {
public:
  typedef typename pair_iterator::value_type::first_type value_type;

  first_of_pair_iterator() = default;
  first_of_pair_iterator(const pair_iterator &init) : pair_iterator(init) { }

  value_type operator *() {
    return pair_iterator::operator *().first;
  }
};

/**
 * This is an iterator adaptor that converts any iterator that returns a pair
 * (e.g.  a map iterator) into one that returns just the second component of
 * that pair.
 */
template<class pair_iterator>
class second_of_pair_iterator : public pair_iterator {
public:
  typedef typename pair_iterator::value_type::second_type value_type;

  second_of_pair_iterator() = default;
  second_of_pair_iterator(const pair_iterator &init) : pair_iterator(init) { }

  value_type operator *() {
    return pair_iterator::operator *().second;
  }
};

/**
 * This is an iterator adaptor that explicitly typecasts each value returned
 * by the base iterator to the indicated type.
 */
template<class base_iterator, class new_type>
class typecast_iterator : public base_iterator {
public:
  typedef new_type value_type;

  typecast_iterator() = default;
  typecast_iterator(const base_iterator &init) : base_iterator(init) { }

  value_type operator *() {
    return (new_type)base_iterator::operator *();
  }
};

#endif
