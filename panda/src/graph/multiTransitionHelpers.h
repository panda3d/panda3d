// Filename: multiTransitionHelpers.h
// Created by:  drose (15Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef MULTITRANSITIONHELPERS_H
#define MULTITRANSITIONHELPERS_H

#include <pandabase.h>

#include "transitionDirection.h"

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
	     OutputIterator result);


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
		    OutputIterator result);


////////////////////////////////////////////////////////////////////
//     Function: dmap_invert
//  Description: Accepts a DirectionMap, and builds a new list
//               which represents the memberwise inversion of the
//               input.  Guarantees that the new list will have
//               exactly the same length as the input list.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class OutputIterator>
OutputIterator
dmap_invert(InputIterator first, InputIterator last,
	    OutputIterator result);


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
	   InputIterator2 first2, InputIterator2 last2);

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
	     InputIterator2 first2, InputIterator2 last2);


////////////////////////////////////////////////////////////////////
//     Function: dmap_is_identity
//  Description: Accepts a DirectionMap, and returns true if all
//               elements in the map correspond to the identity
//               transition, false otherwise.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
bool
dmap_is_identity(InputIterator first, InputIterator last);


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
	   InputIterator2 first2, InputIterator2 last2);

////////////////////////////////////////////////////////////////////
//     Function: bmap_compare
//  Description: Accepts a pair of BoolMaps, and returns < 0 if the
//               first one sorts first, > 0 if the second one sorts
//               first, 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
int
bmap_compare(InputIterator1 first1, InputIterator1 last1,
	     InputIterator2 first2, InputIterator2 last2);


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
	   OutputIterator result);

#ifndef DONT_INLINE_GRAPH
#include "multiTransitionHelpers.I"
#endif

#endif
