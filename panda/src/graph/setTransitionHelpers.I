// Filename: setTransitionHelpers.I
// Created by:  drose (25Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "nodeTransitionCacheEntry.h"
#include "config_graph.h"

////////////////////////////////////////////////////////////////////
//     Function: tmap_get_interest
//  Description: Returns the NodeTransition elements of interest to
//               us.  These are the elements of the first sequence--a
//               NodeTransition map--whose keys appear in the second
//               sequence, a sorted list of NodeTransition
//               TypeHandles.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_get_interest(InputIterator1 first1, InputIterator1 last1,
		  InputIterator2 first2, InputIterator2 last2,
		  OutputIterator result) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < *first2) {
      ++first1;
      ++result;
    } else if (*first2 < (*first1).first) {
      ++first2;
      ++result;
    } else {
      *result = *first1;
      ++first1;
      ++first2;
      ++result;
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_override_union
//  Description: Accepts two NodeTransition or NodeAttribute maps, and
//               builds a new map which is the union of the first two.
//               If an element appears in both maps, the entry from
//               the second is preferred.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_override_union(InputIterator1 first1, InputIterator1 last1,
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
      *result = *first2;
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
//     Function: tmap_merge_union
//  Description: Accepts two NodeAttribute maps, and builds a new map
//               which is the union of the first two.  If an element
//               appears in both maps, the two are merged, preferring
//               the second.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_merge_union(InputIterator1 first1, InputIterator1 last1,
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
      PT(NodeAttribute) c = (*first1).second->merge((*first2).second);
      *result = pair<TypeHandle,  PT(NodeAttribute) >((*first1).first, c);
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
//     Function: tmap_arc_union
//  Description: As above, but updates the transitions on the way to
//               indicate they are being attached to (or removed from)
//               the indicated arc.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_arc_union(InputIterator1 first1, InputIterator1 last1,
	       InputIterator2 first2, InputIterator2 last2,
	       NodeRelation *to_arc, OutputIterator result) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      *result = *first1;
      ++first1;
      ++result;
    } else if ((*first2).first < (*first1).first) {
      (*first2).second->added_to_arc(to_arc);
      *result = *first2;
      ++first2;
      ++result;
    } else {
      (*first1).second->removed_from_arc(to_arc);
      (*first2).second->added_to_arc(to_arc);
      *result = *first2;
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
    (*first2).second->added_to_arc(to_arc);
    *result = *first2;
    ++first2;
    ++result;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_arc_compose
//  Description: 
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_arc_compose(InputIterator1 first1, InputIterator1 last1,
		 InputIterator2 first2, InputIterator2 last2,
		 NodeRelation *to_arc, OutputIterator result) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      *result = *first1;
      ++first1;
      ++result;
    } else if ((*first2).first < (*first1).first) {
      (*first2).second->added_to_arc(to_arc);
      *result = *first2;
      ++first2;
      ++result;
    } else {
      (*first1).second->removed_from_arc(to_arc);

      PT(NodeTransition) c = (*first1).second->compose((*first2).second);
      if (c != (NodeTransition *)NULL) {
	c->added_to_arc(to_arc);
	*result = pair<TypeHandle,  PT(NodeTransition) >((*first1).first, c);
	++result;
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    *result = *first1;
    ++first1;
    ++result;
  }

  while (first2 != last2) { 
    (*first2).second->added_to_arc(to_arc);
    *result = *first2;
    ++first2;
    ++result;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_arc_compose
//  Description: 
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_arc_compose(InputIterator1 first1, InputIterator1 last1,
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
      NodeTransition *c = (*first1).second->compose((*first2).second);
      if (c != (NodeTransition *)NULL) {
	*result = pair<TypeHandle, NodeTransitionCacheEntry>
	  ((*first1).first, c);
	++result;
      }
      ++first1;
      ++first2;
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
//     Function: tmap_compose
//  Description: Accepts two NodeTransition maps, and builds a new
//               list (a NodeTransition map) which represents the
//               memberwise composition of the two input maps.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_compose(InputIterator1 first1, InputIterator1 last1,
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
      *result = pair<TypeHandle, NodeTransitionCacheEntry>
	((*first1).first, 
	 NodeTransitionCacheEntry::compose((*first1).second, (*first2).second));
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
//     Function: tmap_invert_compose
//  Description: Accepts two NodeTransition maps, and builds a new
//               list (a NodeTransition map) which represents the
//               memberwise result of invert_compose of the two input
//               maps.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_invert_compose(InputIterator1 first1, InputIterator1 last1,
		    InputIterator2 first2, InputIterator2 last2,
		    OutputIterator result) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      *result = pair<TypeHandle, NodeTransitionCacheEntry>
	((*first1).first, NodeTransitionCacheEntry::invert((*first1).second));
      ++first1;
      ++result;
    } else if ((*first2).first < (*first1).first) {
      *result = *first2;
      ++first2;
      ++result;
    } else {
      NodeTransitionCacheEntry ic =
	NodeTransitionCacheEntry::invert_compose((*first1).second,
						 (*first2).second);
      if (!ic.is_identity()) {
	// Only bother to store the result if it's not identity.
	*result = pair<TypeHandle,  PT(NodeTransition) >((*first1).first, ic);
	++result;
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, NodeTransitionCacheEntry::invert((*first1).second));
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

// This macro is used in the various cached_compose functions to
// possibly output debug statements about which pieces we're
// composing.

#ifndef NDEBUG
#define OUTPUT_CC_ELEM(desc, type) \
{ \
  if (wrt_cat.is_spam()) { \
    wrt_cat.spam() << "in " << desc << ": " << (type) << "\n"; \
  } \
}
#else
#define OUTPUT_CC_ELEM(desc, type)
#endif

////////////////////////////////////////////////////////////////////
//     Function: tmap_cached_compose_not_1
//  Description: One of several support functions for
//               tmap_cached_compose(), below, this handles the case
//               of tmap_cached_compose() for a NodeTransition that
//               is already known not to be present in list 1, but may
//               be in one or both of lists 2 and 3.
////////////////////////////////////////////////////////////////////
template<class InputIterator2, class InputIterator3, class OutputIterator>
void
tmap_cached_compose_not_1(InputIterator2 &first2, InputIterator3 &first3,
			  UpdateSeq now, OutputIterator &result) {
  if ((*first2).first < (*first3).first) {
    // Here's an element in list 2 that is not in lists 1 or 3.
    OUTPUT_CC_ELEM("2", (*first2).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first2).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	(*first2).second, 
	NodeTransitionCacheEntry(),
	now));
    ++first2;
    ++result;
    
  } else if ((*first3).first < (*first2).first) {
    // Here's an element in list 3 that is not lists 2 or 1.
    OUTPUT_CC_ELEM("3", (*first3).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first3).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	NodeTransitionCacheEntry(),
	(*first3).second, 
	now));
    ++first3;
    ++result;
    
  } else {  // (*first2).first == (*first3).first
    // Here's an element in lists 2 and 3 that is not in list 1.
    OUTPUT_CC_ELEM("2, 3", (*first2).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first2).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	(*first2).second, 
	(*first3).second, 
	now));
    ++first2;
    ++first3;
    ++result;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: tmap_cached_compose_not_2
//  Description: One of several support functions for
//               tmap_cached_compose(), below, this handles the case
//               of tmap_cached_compose() for an NodeTransition that
//               is already known not to be present in list 2, but may
//               be in one or both of lists 1 and 3.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator3, class OutputIterator>
void
tmap_cached_compose_not_2(InputIterator1 &first1, InputIterator3 &first3,
			  UpdateSeq now, OutputIterator &result) {
  if ((*first1).first < (*first3).first) {
    // Here's an element in list 1 that is not in lists 2 or 3.
    OUTPUT_CC_ELEM("1", (*first1).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, 
       NodeTransitionCacheEntry::cached_compose
       ((*first1).second, 
	NodeTransitionCacheEntry(),
	NodeTransitionCacheEntry(),
	now));
    ++first1;
    ++result;
    
  } else if ((*first3).first < (*first1).first) {
    // Here's an element in list 3 that is not lists 1 or 2.
    OUTPUT_CC_ELEM("3", (*first3).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first3).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	NodeTransitionCacheEntry(),
	(*first3).second, 
	now));
    ++first3;
    ++result;
    
  } else {  // (*first1).first == (*first3).first
    // Here's an element in lists 1 and 3 that is not in list 2.
    OUTPUT_CC_ELEM("1, 3", (*first1).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, 
       NodeTransitionCacheEntry::cached_compose
       ((*first1).second,
	NodeTransitionCacheEntry(),
	(*first3).second, 
	now));
    ++first1;
    ++first3;
    ++result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_cached_compose_not_3
//  Description: One of several support functions for
//               tmap_cached_compose(), below, this handles the case
//               of tmap_cached_compose() for an NodeTransition that
//               is already known not to be present in list 3, but may
//               be in one or both of lists 1 and 2.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
void
tmap_cached_compose_not_3(InputIterator1 &first1, InputIterator2 &first2,
			  UpdateSeq now, OutputIterator &result) {
  if ((*first1).first < (*first2).first) {
    // Here's an element in list 1 that is not in lists 2 or 3.
    OUTPUT_CC_ELEM("1", (*first1).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, 
       NodeTransitionCacheEntry::cached_compose
       ((*first1).second, 
	NodeTransitionCacheEntry(),
	NodeTransitionCacheEntry(),
	now));
    ++first1;
    ++result;
    
  } else if ((*first2).first < (*first1).first) {
    // Here's an element in list 2 that is not lists 1 or 3.
    OUTPUT_CC_ELEM("2", (*first2).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first2).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	(*first2).second, 
	NodeTransitionCacheEntry(),
	now));
    ++first2;
    ++result;
    
  } else {  // (*first1).first == (*first2).first
    // Here's an element in lists 1 and 2 that is not in list 3.
    OUTPUT_CC_ELEM("1, 2", (*first1).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, 
       NodeTransitionCacheEntry::cached_compose
       ((*first1).second,
	(*first2).second,
	NodeTransitionCacheEntry(),
	now));
    ++first1;
    ++first2;
    ++result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_cached_compose_12
//  Description: One of several support functions for
//               tmap_cached_compose(), below, this handles the case
//               of tmap_cached_compose() for an NodeTransition that
//               is already known to be in both lists 1 and 2, and may
//               or may not also be in list 3.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class InputIterator3, 
  class OutputIterator>
void
tmap_cached_compose_12(InputIterator1 &first1, InputIterator2 &first2,
		       InputIterator3 &first3,
		       UpdateSeq now, OutputIterator &result) {
  if ((*first1).first < (*first3).first) {
    // Here's an element in lists 1 and 2 that is not in list 3.
    OUTPUT_CC_ELEM("1, 2", (*first1).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, 
       NodeTransitionCacheEntry::cached_compose
       ((*first1).second,
	(*first2).second,
	NodeTransitionCacheEntry(),
	now));
    ++first1;
    ++first2;
    ++result;
    
  } else if ((*first3).first < (*first1).first) {
    // Here's an element in list 3 that is not lists 1 or 2.
    OUTPUT_CC_ELEM("3", (*first3).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first3).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	NodeTransitionCacheEntry(),
	(*first3).second,
	now));
    ++first3;
    ++result;
    
  } else {  // (*first1).first == (*first3).first
    // Here's an element in lists 1, 2, and 3.
    OUTPUT_CC_ELEM("1, 2, 3", (*first1).first);
    
    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, 
       NodeTransitionCacheEntry::cached_compose
       ((*first1).second,
	(*first2).second,
	(*first3).second,
	now));
    ++first1;
    ++first2;
    ++first3;
    ++result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_cached_compose
//  Description: Calls dynamic_cached_compose() on each matching
//               element of [first1 .. last1), [cached_first
//               .. cached_last), and [value_first .. value_last).
//
//               This function is big and complicated because it has
//               to do a parallel merge on three separate lists.  Any
//               NodeTransition type may appear in any or all of the
//               lists; if a particular type appears in any list but
//               is absent from the others, an implicit identity
//               transition should be inferred where it is absent.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class InputIterator3,
  class OutputIterator>
OutputIterator
tmap_cached_compose(InputIterator1 first1, InputIterator1 last1,
		    InputIterator2 first2, InputIterator2 last2,
		    InputIterator3 first3, InputIterator3 last3,
		    UpdateSeq now, OutputIterator result) {
  while (first1 != last1 && first2 != last2 && first3 != last3) {
    if ((*first1).first < (*first2).first) {
      tmap_cached_compose_not_2(first1, first3, now, result);

    } else if ((*first2).first < (*first1).first) {
      tmap_cached_compose_not_1(first2, first3, now, result);

    } else {    // (*first1).first == (*first2).first
      tmap_cached_compose_12(first1, first2, first3, now, result);
    }
  }

  // Now pick up everything else.
  if (first1 == last1) {
    while (first2 != last2 && first3 != last3) {
      tmap_cached_compose_not_1(first2, first3, now, result);
    }

  } else if (first2 == last2) {
    while (first1 != last1 && first3 != last3) {
      tmap_cached_compose_not_2(first1, first3, now, result);
    }

  } else if (first3 == last3) {
    while (first1 != last1 && first2 != last2) {
      tmap_cached_compose_not_3(first1, first2, now, result);
    }
  }

  while (first1 != last1) {
    // Here's an element in list 1 that is not in lists 2 or 3.
    OUTPUT_CC_ELEM("1", (*first1).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first1).first, 
       NodeTransitionCacheEntry::cached_compose
       ((*first1).second, 
	NodeTransitionCacheEntry(),
	NodeTransitionCacheEntry(),
	now));
    ++first1;
    ++result;
  }

  while (first2 != last2) {
    // Here's an element in list 2 that is not in lists 1 or 3.
    OUTPUT_CC_ELEM("2", (*first2).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first2).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	(*first2).second, 
	NodeTransitionCacheEntry(),
	now));
    ++first2;
    ++result;
  }

  while (first3 != last3) {
    // Here's an element in list 3 that is not lists 1 or 2.
    OUTPUT_CC_ELEM("3", (*first3).first);

    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first3).first, 
       NodeTransitionCacheEntry::cached_compose
       (NodeTransitionCacheEntry(),
	NodeTransitionCacheEntry(),
	(*first3).second, 
	now));
    ++first3;
    ++result;
  }

  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: tmap_apply
//  Description: Accepts a NodeAttribute map and a NodeTransition map,
//               and builds a new list (a NodeAttribute map) which
//               represents the memberwise application of the two
//               input maps.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator
tmap_apply(InputIterator1 first1, InputIterator1 last1,
	   InputIterator2 first2, InputIterator2 last2,
	   OutputIterator result) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      *result = *first1;
      ++first1;
      ++result;
    } else if ((*first2).first < (*first1).first) {
      *result = pair<TypeHandle,  PT(NodeAttribute) >
	((*first2).first, 
	 NodeTransitionCacheEntry::apply(NULL, (*first2).second));
      ++first2;
      ++result;
    } else {
      *result = pair<TypeHandle,  PT(NodeAttribute) >
	((*first1).first, 
	 NodeTransitionCacheEntry::apply((*first1).second, (*first2).second));
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
    *result = pair<TypeHandle,  PT(NodeAttribute) >
      ((*first2).first, 
       NodeTransitionCacheEntry::apply(NULL, (*first2).second));
    ++first2;
    ++result;
  }
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: tmap_invert
//  Description: Accepts a NodeTransition map, and builds a new list
//               which represents the memberwise inversion of the
//               input.  Guarantees that the new list will have
//               exactly the same length as the input list.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class OutputIterator>
OutputIterator
tmap_invert(InputIterator first, InputIterator last,
	    OutputIterator result) {
  while (first != last) {
    *result = pair<TypeHandle, NodeTransitionCacheEntry>
      ((*first).first, NodeTransitionCacheEntry::invert((*first).second));
    ++first;
    ++result;
  }
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: tmap_equiv_trans
//  Description: Accepts a pair of NodeTransition maps, and returns
//               true if they are equivalent, false otherwise.  Two
//               NodeTransition maps are defined to be equivalent if
//               all nonidentity members present in one set are
//               present and equivalent in the other set,
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
bool
tmap_equiv_trans(InputIterator1 first1, InputIterator1 last1,
		 InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      if (!(*first1).second.is_identity()) {
	return false;
      }
      ++first1;
    } else if ((*first2).first < (*first1).first) {
      if (!(*first2).second.is_identity()) {
	return false;
      }
      ++first2;
    } else {
      if (!(*first1).second.compare_to((*first2).second) == 0) {
	return false;
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    if (!(*first1).second.is_identity()) {
      return false;
    }
    ++first1;
  }

  while (first2 != last2) { 
    if (!(*first2).second.is_identity()) {
      return false;
    }
    ++first2;
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: tmap_equiv_attr
//  Description: Accepts a pair of NodeAttribute maps, and returns
//               true if they are equivalent, false otherwise.  Two
//               NodeAttributes maps are defined to be equivalent if
//               all non-NULL members present in one set are present
//               and equivalent in the other set,
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
bool
tmap_equiv_attr(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      if ((*first1).second != (NodeAttribute *)NULL) {
	return false;
      }
      ++first1;
    } else if ((*first2).first < (*first1).first) {
      if ((*first2).second != (NodeAttribute *)NULL) {
	return false;
      }
      ++first2;
    } else {
      if ((*first1).second != (*first2).second) {
	if ((*first1).second == (NodeAttribute *)NULL ||
	    (*first2).second == (NodeAttribute *)NULL ||
	    (*first1).second->compare_to((*first2).second) != 0) {
	  return false;
	}
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    if ((*first1).second != (NodeAttribute *)NULL) {
      return false;
    }
    ++first1;
  }

  while (first2 != last2) { 
    if ((*first2).second != (NodeAttribute *)NULL) {
      return false;
    }
    ++first2;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_compare_cache
//  Description: Accepts a pair of NodeTransition maps, and returns
//               -1 if the first one sorts before the second one,
//               1 if it sorts after, or 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
int
tmap_compare_cache(InputIterator1 first1, InputIterator1 last1,
		   InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      if (!(*first1).second.is_identity()) {
	return 1;
      }
      ++first1;
    } else if ((*first2).first < (*first1).first) {
      if (!(*first2).second.is_identity()) {
	return -1;
      }
      ++first2;
    } else {
      int result;
      result = (*first1).second.compare_to((*first2).second);
      if (result != 0) {
	return result;
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    if (!(*first1).second.is_identity()) {
      // list1 is longer.
      return -1;
    }
    ++first1;
  }

  while (first2 != last2) { 
    if (!(*first2).second.is_identity()) {
      // list2 is longer.
      return 1;
    }
    ++first2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_compare_trans
//  Description: Accepts a pair of NodeTransition maps, and returns
//               -1 if the first one sorts before the second one,
//               1 if it sorts after, or 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
int
tmap_compare_trans(InputIterator1 first1, InputIterator1 last1,
		   InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      if ((*first1).second != (NodeTransition *)NULL) {
	return 1;
      }
      ++first1;
    } else if ((*first2).first < (*first1).first) {
      if ((*first2).second != (NodeTransition *)NULL) {
	return -1;
      }
      ++first2;
    } else {
      if ((*first1).second != (*first2).second) {
	if ((*first1).second == (NodeTransition *)NULL) {
	  return -1;
	} else if ((*first2).second == (NodeTransition *)NULL) {
	  return 1;
	} else {
	  int result;
	  result = (*first1).second->compare_to(*(*first2).second);
	  if (result != 0) {
	    return result;
	  }
	}
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    if ((*first1).second != (NodeTransition *)NULL) {
      // list1 is longer.
      return -1;
    }
    ++first1;
  }

  while (first2 != last2) { 
    if ((*first2).second != (NodeTransition *)NULL) {
      // list2 is longer.
      return 1;
    }
    ++first2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_compare_attr
//  Description: Accepts a pair of NodeAttribute maps, and returns
//               -1 if the first one sorts before the second one,
//               1 if it sorts after, or 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
template<class InputIterator1, class InputIterator2>
int
tmap_compare_attr(InputIterator1 first1, InputIterator1 last1,
		  InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if ((*first1).first < (*first2).first) {
      if ((*first1).second != (NodeAttribute *)NULL) {
	return 1;
      }
      ++first1;
    } else if ((*first2).first < (*first1).first) {
      if ((*first2).second != (NodeAttribute *)NULL) {
	return -1;
      }
      ++first2;
    } else {
      if ((*first1).second != (*first2).second) {
	if ((*first1).second == (NodeAttribute *)NULL) {
	  return -1;
	} else if ((*first2).second == (NodeAttribute *)NULL) {
	  return 1;
	} else {
	  int result;
	  result = (*first1).second->compare_to(*(*first2).second);
	  if (result != 0) {
	    return result;
	  }
	}
      }
      ++first1;
      ++first2;
    }
  }

  while (first1 != last1) {
    if ((*first1).second != (NodeAttribute *)NULL) {
      // list1 is longer.
      return -1;
    }
    ++first1;
  }

  while (first2 != last2) { 
    if ((*first2).second != (NodeAttribute *)NULL) {
      // list2 is longer.
      return 1;
    }
    ++first2;
  }

  return 0;
}



////////////////////////////////////////////////////////////////////
//     Function: tmap_is_identity
//  Description: Accepts a NodeTransition map, and returns true if all
//               elements in the map correspond to the identity
//               transition, false otherwise.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
bool
tmap_is_identity(InputIterator first, InputIterator last) {
  while (first != last) {
    if (!(*first).second.is_identity()) {
      return false;
    }
    ++first;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: tmap_is_initial
//  Description: Accepts a NodeAttribute map, and returns true if all
//               elements in the map correspond to the initial
//               attribute, false otherwise.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
bool
tmap_is_initial(InputIterator first, InputIterator last) {
  while (first != last) {
    if (!(*first).second.is_initial()) {
      return false;
    }
    ++first;
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: tmap_output
//  Description: Accepts a NodeTransition or NodeAttribute map and
//               writes each element to the given output stream with a
//               single space separating them.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
ostream &
tmap_output(InputIterator first, InputIterator last, ostream &out) {
  if (first != last) {
    out << *((*first).second);
    ++first;
    while (first != last) {
      out << " " << *((*first).second);
      ++first;
    }
  }
  return out;
}
