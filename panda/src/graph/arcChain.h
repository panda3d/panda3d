// Filename: arcChain.h
// Created by:  drose (05Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef ARCCHAIN_H
#define ARCCHAIN_H

#include <pandabase.h>

#include "nodeRelation.h"

#include <pointerTo.h>
#include <referenceCount.h>
#include <notify.h>

////////////////////////////////////////////////////////////////////
//       Class : ArcChain
// Description : This defines a singly-linked chain of arcs from one
//               point (e.g. the root of the scene graph) to another
//               point (e.g. a leaf), although no assumption is made
//               about the relationship between the arcs.  It is
//               simply a list of arcs that may only be lengthened or
//               shortened, or copied.
//
//               This list may be copied by reference using the copy
//               constructor; the new copy may be appended to without
//               modifying the source (but individual nodes may not be
//               modified).
//
//               This serves as the fundamental implementation of
//               NodePaths, for instance, and also is used during
//               render traversals to manage the list of arcs
//               traversed so far (and thus compute unambiguous
//               wrt's).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ArcChain {
private:
  // We maintain our own linked list structure here instead of using
  // some STL structure, so we can efficiently copy-construct these
  // things by sharing the initial part of the list.

  // We have a singly-linked list whose head is the bottom arc of the
  // path, and whose tail is the top arc of the path.  Thus, we can
  // copy the entire path by simply copying the head pointer, and we
  // can then append to or shorten our own path without affecting the
  // paths we're sharing ArcComponents with.  Very LISPy.
  class ArcComponent : public ReferenceCount {
  public:
    INLINE ArcComponent(NodeRelation *arc, ArcComponent *next);
    PT(NodeRelation) _arc;
    PT(ArcComponent) _next;
  };

  PT(ArcComponent) _head;

  // This is a supporting class for iterating through all the arcs via
  // begin() .. end().
  class ForwardIterator {
  public:
    INLINE ForwardIterator(ArcComponent *comp = NULL);
    INLINE NodeRelation *operator * () const;
    INLINE void operator ++();
    INLINE bool operator == (const ForwardIterator &other) const;
    INLINE bool operator != (const ForwardIterator &other) const;

  private:
    ArcComponent *_comp;
  };

public:
  typedef ForwardIterator iterator;
  typedef ForwardIterator const_iterator;

  INLINE ArcChain();
  INLINE ArcChain(const ArcChain &copy);
  INLINE void operator = (const ArcChain &copy);

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;
  INLINE bool empty() const;

  INLINE void push_back(NodeRelation *arc);
  INLINE void pop_back();
  INLINE NodeRelation *back() const;

  INLINE bool operator == (const ArcChain &other) const;
  INLINE bool operator != (const ArcChain &other) const;
  INLINE bool operator < (const ArcChain &other) const;
  int compare_to(const ArcChain &other) const;

  INLINE void output(ostream &out) const;

private:
  void r_output(ostream &out, ArcComponent *comp) const;
};

INLINE ostream &operator << (ostream &out, const ArcChain &arc_chain) {
  arc_chain.output(out);
  return out;
}

#include "arcChain.I"

#endif
