// Filename: iterator_types.h
// Created by:  drose (10Feb99)
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

#ifndef ITERATOR_TYPES_H
#define ITERATOR_TYPES_H


////////////////////////////////////////////////////////////////////
//       Class : first_of_pair_iterator
// Description : This is an iterator adaptor that converts any
//               iterator that returns a pair (e.g. a map iterator)
//               into one that returns just the first component of
//               that pair.
////////////////////////////////////////////////////////////////////
template<class pair_iterator>
class first_of_pair_iterator : public pair_iterator {
public:
  typedef TYPENAME pair_iterator::value_type::first_type value_type;

  first_of_pair_iterator() { }
  first_of_pair_iterator(const pair_iterator &init) : pair_iterator(init) { }
  first_of_pair_iterator(const first_of_pair_iterator<pair_iterator> &copy) : pair_iterator(copy) { }

  value_type operator *() {
    return pair_iterator::operator *().first;
  }
};

////////////////////////////////////////////////////////////////////
//       Class : second_of_pair_iterator
// Description : This is an iterator adaptor that converts any
//               iterator that returns a pair (e.g. a map iterator)
//               into one that returns just the second component of
//               that pair.
////////////////////////////////////////////////////////////////////
template<class pair_iterator>
class second_of_pair_iterator : public pair_iterator {
public:
  typedef TYPENAME pair_iterator::value_type::second_type value_type;

  second_of_pair_iterator() { }
  second_of_pair_iterator(const pair_iterator &init) : pair_iterator(init) { }
  second_of_pair_iterator(const second_of_pair_iterator<pair_iterator> &copy) : pair_iterator(copy) { }

  value_type operator *() {
    return pair_iterator::operator *().second;
  }
};

////////////////////////////////////////////////////////////////////
//       Class : typecast_iterator
// Description : This is an iterator adaptor that explicitly typecasts
//               each value returned by the base iterator to the
//               indicated type.
////////////////////////////////////////////////////////////////////
template<class base_iterator, class new_type>
class typecast_iterator : public base_iterator {
public:
  typedef new_type value_type;

  typecast_iterator() { }
  typecast_iterator(const base_iterator &init) : base_iterator(init) { }
  typecast_iterator(const typecast_iterator<base_iterator, new_type> &copy) : base_iterator(copy) { }

  value_type operator *() {
    return (new_type)base_iterator::operator *();
  }
};

#endif
