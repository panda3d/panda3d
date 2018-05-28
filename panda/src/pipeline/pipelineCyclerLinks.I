/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCyclerLinks.I
 * @author drose
 * @date 2006-02-16
 */

#ifdef THREADED_PIPELINE
/**
 *
 */
INLINE PipelineCyclerLinks::
PipelineCyclerLinks() {
#ifndef NDEBUG
  _next = nullptr;
  _prev = nullptr;
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 *
 */
INLINE PipelineCyclerLinks::
~PipelineCyclerLinks() {
  nassertv(_next == nullptr && _prev == nullptr);
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * When called on an empty object, sets it up to be the head of a linked list.
 */
INLINE void PipelineCyclerLinks::
make_head() {
  nassertv(_next == nullptr && _prev == nullptr);
  _next = this;
  _prev = this;
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * When called on the head of an empty linked list, resets it to an empty
 * object, for safe destruction.
 */
INLINE void PipelineCyclerLinks::
clear_head() {
  nassertv(_next == this && _prev == this);
#ifndef NDEBUG
  _next = nullptr;
  _prev = nullptr;
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * Removes a PipelineCyclerLinks record from the doubly-linked list.
 */
INLINE void PipelineCyclerLinks::
remove_from_list() {
  nassertv(_prev->_next == this && _next->_prev == this);
  _prev->_next = _next;
  _next->_prev = _prev;
#ifndef NDEBUG
  _next = nullptr;
  _prev = nullptr;
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * Adds a PipelineCyclerLinks record before the indicated node in the doubly-
 * linked list.
 */
INLINE void PipelineCyclerLinks::
insert_before(PipelineCyclerLinks *node) {
  nassertv(node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == nullptr &&
           _next == nullptr);
  _prev = node->_prev;
  _next = node;
  _prev->_next = this;
  node->_prev = this;
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * When called on the head of an empty list, takes all of the leemnts from the
 * indicated list and moves them to this list.
 */
INLINE void PipelineCyclerLinks::
take_list(PipelineCyclerLinks &other) {
  nassertv(_next == this && _prev == this);
  if (other._next == &other && other._prev == &other) {
    // The other list is empty; this is a no-op.
    return;
  }

  other._next->_prev = this;
  other._prev->_next = this;
  _next = other._next;
  _prev = other._prev;

  other._next = &other;
  other._prev = &other;
}
#endif  // THREADED_PIPELINE
