// Filename: multiTransitionHelpers.I
// Created by:  drose (15Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: dmap_compose
//  Description: Accepts two DirectionMaps, and builds a new
//               list (another DirectionMap) which represents the
//               memberwise composition of the two input maps.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
dmap_compose(InputIterator1 first1, InputIterator1 last1,
             InputIterator2 first2, InputIterator2 last2,
             OutputIterator result) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      *result = *first1;
      ++first1;
      ++result;
    } else if ((*first2).first < (*first1).first) {
      *result = *first2;
      ++first2;
      ++result;
    } else {
      // If the second Transition is identity, it has no effect.
      // Otherwise, it replaces us.
      if ((*first2).second != TD_identity) {
        *result = *first2;
      } else {
        *result = *first1;
      }
      ++first1;
      ++first2;
      ++result;
    }
  }

  while (first1 != last1) {
    *result = *first1;
    ++first1;
    ++result;
  }

  while (first2 != last2) {
    *result = *first2;
    ++first2;
    ++result;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: dmap_invert_compose
//  Description: Accepts two DirectionMaps, and builds a new
//               list (another DirectionMap) which represents the
//               memberwise result inverting and composing.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
dmap_invert_compose(InputIterator1 first1, InputIterator1 last1,
                    InputIterator2 first2, InputIterator2 last2,
                    OutputIterator result) {
  typedef TYPENAME InputIterator1::value_type OutputType;

  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      (*result) = OutputType((*first1).first, TD_inverse);
      ++first1;
      ++result;
    } else if ((*first2).first < (*first1).first) {
      *result = *first2;
      ++first2;
      ++result;
    } else {
      TransitionDirection dir1 = (*first1).second;
      TransitionDirection dir2 = (*first2).second;

      if (dir1 == dir2 &&
          (dir1 != TD_normal || (*first1).first == (*first2).first)) {
        // The two transitions are equivalent, so the invert_compose
        // will be identity.  Don't bother to store it.

      } else {
        if ((*first2).second != TD_identity) {
          *result = *first2;
        } else {
          (*result) = OutputType((*first1).first, TD_inverse);
        }
        ++result;
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    (*result) = OutputType((*first1).first, TD_inverse);
    ++first1;
    ++result;
  }

  while (first2 != last2) {
    *result = *first2;
    ++first2;
    ++result;
  }
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: dmap_invert
//  Description: Accepts a DirectionMap, and builds a new list
//               which represents the memberwise inversion of the
//               input.  Guarantees that the new list will have
//               exactly the same length as the input list.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class OutputIterator, class ValueType>
OutputIterator
dmap_invert(InputIterator first, InputIterator last,
            OutputIterator result, ValueType *) {
  while (first != last) {
    switch ((*first).second) {
    case TD_identity:
      (*result) = ValueType((*first).first, TD_identity);
      ++result;
      break;

    case TD_on:
      (*result) = ValueType((*first).first, TD_off);
      ++result;
      break;

    case TD_off:
      (*result) = ValueType((*first).first, TD_on);
      ++result;
      break;
    }
    ++first;
  }
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: dmap_equiv
//  Description: Accepts a pair of DirectionMaps, and returns
//               true if they are equivalent, false otherwise.  Two
//               DirectionMaps are defined to be equivalent if
//               all nonidentity members present in one set are
//               present and equivalent in the other set,
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
bool
dmap_equiv(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      if ((*first1).second != TD_identity) {
        return false;
      }
      ++first1;
    } else if ((*first2).first < (*first1).first) {
      if ((*first2).second != TD_identity) {
        return false;
      }
      ++first2;
    } else {
      TransitionDirection dir1 = (*first1).second;
      TransitionDirection dir2 = (*first2).second;

      if (dir1 != dir2) {
        return false;
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    if ((*first1).second != TD_identity) {
      return false;
    }
    ++first1;
  }

  while (first2 != last2) {
    if ((*first2).second != TD_identity) {
      return false;
    }
    ++first2;
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: dmap_compare
//  Description: Accepts a pair of DirectionMaps, and returns
//               < 0 if the first one sorts before the second one, > 0
//               if the first one sorts after, 0 if they are
//               equivalent.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
int
dmap_compare(InputIterator1 first1, InputIterator1 last1,
             InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      if ((*first1).second != TD_identity) {
        // list1 has the first value that does not appear in list2.
        return 1;
      }
      ++first1;
    } else if ((*first2).first < (*first1).first) {
      if ((*first2).second != TD_identity) {
        // list2 has the first value that does not appear in list1.
        return -1;
      }
      ++first2;
    } else {
      TransitionDirection dir1 = (*first1).second;
      TransitionDirection dir2 = (*first2).second;

      if (dir1 != dir2) {
        if (dir1 == TD_identity) {
          // list1 has the first value that does not appear in list2.
          return 1;
        } else if (dir2 == TD_identity) {
          // list2 has the first value that does not appear in list1.
          return -1;
        }

        // This value appears in both lists, but it is TD_normal in
        // one and TD_inverse in the other.
        return (dir1 < dir2) ? -1 : 1;
      }

      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    if ((*first1).second != TD_identity) {
      // list1 is longer.
      return -1;
    }
    ++first1;
  }

  while (first2 != last2) {
    if ((*first2).second != TD_identity) {
      // list2 is longer.
      return 1;
    }
    ++first2;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////
//     Function: dmap_is_identity
//  Description: Accepts a DirectionMap, and returns true if all
//               elements in the map correspond to the identity
//               transition, false otherwise.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
bool
dmap_is_identity(InputIterator first, InputIterator last) {
  while (first != last) {
    if ((*first).second != TD_identity) {
      return false;
    }
    ++first;
  }
  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: bmap_equiv
//  Description: Accepts a pair of BoolMaps, and returns true if they
//               are equivalent, false otherwise.  Two BoolMaps are
//               defined to be equivalent if all 'on' members present
//               in one set are present and equivalent in the other
//               set,
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
bool
bmap_equiv(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1) < (*first2)) {
      return false;
    } else if ((*first2) < (*first1)) {
      return false;
    } else {
      ++first1;
      ++first2;
    }
  }

  if (first1 != last1 || first2 != last2) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: bmap_compare
//  Description: Accepts a pair of BoolMaps, and returns < 0 if the
//               first one sorts first, > 0 if the second one sorts
//               first, 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
int
bmap_compare(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1) < (*first2)) {
      return -1;
    } else if ((*first2) < (*first1)) {
      return 1;
    } else {
      ++first1;
      ++first2;
    }
  }

  if (first1 != last1) {
    // list 1 is longer.
    return -1;
  }
  if (first2 != last2) {
    // list 2 is longer.
    return 1;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////
//     Function: bmap_apply
//  Description: Accepts a BoolMap and a DirectionMap, and builds a
//               new list (another BoolMap) which represents the
//               memberwise application of the two input maps.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
bmap_apply(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2,
           bool complete_transition, TransitionDirection want_dirs,
           OutputIterator result) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1) < (*first2).first) {
      // Here's an attribute property that wasn't mentioned by the
      // transition.  It stays, unless the transition is "complete".
      if (!complete_transition) {
        *result = *first1;
        ++result;
      }
      ++first1;

    } else if ((*first2).first < (*first1)) {
      // The transition mentions this property that wasn't previously
      // in the attribute.  It becomes a member of the attribute only
      // if the transition explicitly turned it on.

      if ((*first2).second == want_dirs) {
        *result = (*first2).first;
        ++result;
        break;
      }

      ++first2;

    } else {  // ((*first2).first == (*first1))

      // The transition mentions this property that was already in the
      // attribute.  Does the transition replace, preserve, or delete
      // the attribute?

      if ((*first2).second == TD_identity) {
        // Preserve.
        *result = *first1;
        ++result;

      } else if ((*first2).second == want_dirs) {
        // Replace.
        *result = (*first2).first;
        ++result;

      } else {
        // Omit.
      }

      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    // Here's an attribute property that wasn't mentioned by the
    // transition.  It stays, unless the transition is "complete".
    if (!complete_transition) {
      *result = *first1;
      ++result;
    }
    ++first1;
  }

  while (first2 != last2) {
    // The transition mentions this property that wasn't previously
    // in the attribute.  It becomes a member of the attribute only
    // if the transition explicitly turned it on.

    if ((*first2).second == want_dirs) {
      *result = (*first2).first;
      ++result;
      break;
    }

    ++first2;
  }
  return result;
}
