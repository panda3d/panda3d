// Filename: setTransitionHelpers.h
// Created by:  drose (25Jan99)
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

#ifndef SETTRANSITIONHELPERS_H
#define SETTRANSITIONHELPERS_H

#include <pandabase.h>

#include <updateSeq.h>


////////////////////////////////////////////////////////////////////
//
// tmap_* functions
//
// The SetTransition classes are implemented with the help of a
// handful of template functions that operate on NodeTransition maps.
//
// Each of these follows the standard STL calling conventions for
// operating on one or two sequences, and/or storing the results into
// an output sequence.
//
////////////////////////////////////////////////////////////////////

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_get_interest(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, InputIterator2 last2,
                  OutputIterator result);

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_union(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2,
           OutputIterator result);

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_arc_union(InputIterator1 first1, InputIterator1 last1,
               InputIterator2 first2, InputIterator2 last2,
               NodeRelation *to_arc, OutputIterator result);

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_arc_compose(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2,
                 NodeRelation *to_arc, OutputIterator result);

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_compose(InputIterator1 first1, InputIterator1 last1,
             InputIterator2 first2, InputIterator2 last2,
             OutputIterator result);

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_invert_compose(InputIterator1 first1, InputIterator1 last1,
                    InputIterator2 first2, InputIterator2 last2,
                    OutputIterator result);

template<class InputIterator1, class InputIterator2, class InputIterator3,
  class OutputIterator>
OutputIterator
tmap_cached_compose(InputIterator1 first1, InputIterator1 last1,
                    InputIterator2 cached_first, InputIterator2 cached_last,
                    InputIterator3 value_first, InputIterator3 value_last,
                    UpdateSeq now, OutputIterator result);

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_apply(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2,
           OutputIterator result);

template<class InputIterator, class OutputIterator>
OutputIterator
tmap_invert(InputIterator first, InputIterator last,
            OutputIterator result);

template<class InputIterator1, class InputIterator2>
bool
tmap_equiv_trans(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2);

template<class InputIterator1, class InputIterator2>
bool
tmap_equiv_attr(InputIterator1 first1, InputIterator1 last1,
                InputIterator2 first2, InputIterator2 last2);

template<class InputIterator1, class InputIterator2>
int
tmap_compare_cache(InputIterator1 first1, InputIterator1 last1,
                   InputIterator2 first2, InputIterator2 last2);

template<class InputIterator1, class InputIterator2>
int
tmap_compare_trans(InputIterator1 first1, InputIterator1 last1,
                   InputIterator2 first2, InputIterator2 last2);

template<class InputIterator1, class InputIterator2>
int
tmap_compare_attr(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, InputIterator2 last2);

template<class InputIterator>
bool
tmap_is_identity(InputIterator first, InputIterator last);

template<class InputIterator>
bool
tmap_is_initial(InputIterator first, InputIterator last);

template<class InputIterator>
ostream &
tmap_output(InputIterator first, InputIterator last, ostream &out);

#include "setTransitionHelpers.T"

#endif
