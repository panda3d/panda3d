// Filename: simpleLru.h
// Created by:  drose (11May07)
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

#ifndef SIMPLELRU_H
#define SIMPLELRU_H

#include "pandabase.h"
#include "linkedListNode.h"
#include "namable.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"

class SimpleLruPage;

////////////////////////////////////////////////////////////////////
//       Class : SimpleLru
// Description : An implementation of a very simple LRU algorithm.
//               Also see AdaptiveLru.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ SimpleLru : public LinkedListNode, public Namable {
PUBLISHED:
  SimpleLru(const string &name, size_t max_size);
  ~SimpleLru();

  INLINE size_t get_total_size() const;
  INLINE size_t get_max_size() const;
  INLINE void set_max_size(size_t max_size);
  size_t count_active_size() const;

  INLINE void consider_evict();
  INLINE void evict_to(size_t target_size);
  INLINE void begin_epoch();

  INLINE bool validate();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

public:
  static LightMutex &_global_lock;

private:
  void do_evict_to(size_t target_size, bool hard_evict);
  bool do_validate();

  size_t _total_size;
  size_t _max_size;
  SimpleLruPage *_active_marker;

  friend class SimpleLruPage;
};

////////////////////////////////////////////////////////////////////
//       Class : SimpleLruPage
// Description : One atomic piece that may be managed by a SimpleLru
//               chain.  To use this class, inherit from it and
//               override evict_lru().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ SimpleLruPage : public LinkedListNode {
PUBLISHED:
  INLINE SimpleLruPage(size_t lru_size);
  INLINE SimpleLruPage(const SimpleLruPage &copy);
  INLINE void operator = (const SimpleLruPage &copy);

  virtual ~SimpleLruPage();

  INLINE SimpleLru *get_lru() const;

  void enqueue_lru(SimpleLru *lru);
  INLINE void dequeue_lru();

  INLINE void mark_used_lru() const;
  INLINE void mark_used_lru(SimpleLru *lru);

  INLINE size_t get_lru_size() const;
  INLINE void set_lru_size(size_t lru_size);

  virtual void evict_lru();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

private:
  SimpleLru *_lru;

  size_t _lru_size;

  friend class SimpleLru;
};

inline ostream &operator << (ostream &out, const SimpleLru &lru) {
  lru.output(out);
  return out;
}

inline ostream &operator << (ostream &out, const SimpleLruPage &page) {
  page.output(out);
  return out;
}

#include "simpleLru.I"

#endif
