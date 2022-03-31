/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file adaptiveLru.h
 * @author drose
 * @date 2008-09-03
 */

#ifndef ADAPTIVELRU_H
#define ADAPTIVELRU_H

#include "pandabase.h"
#include "linkedListNode.h"
#include "namable.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"

class AdaptiveLruPage;

// See the comment in the head of AdaptiveLruPage, below, for an explanation
// of these two silly little classes.
class EXPCL_PANDA_GOBJ AdaptiveLruPageDynamicList : public LinkedListNode {
public:
  friend class AdaptiveLru;
};

class EXPCL_PANDA_GOBJ AdaptiveLruPageStaticList : public LinkedListNode {
public:
  friend class AdaptiveLru;
};

/**
 * A basic LRU-type algorithm, except that it is adaptive and attempts to
 * avoid evicting pages that have been used more frequently (even if less
 * recently) than other pages.
 *
 * The interface is designed to be identical to that for SimpleLru, so that it
 * may be used as a drop-in replacement.
 */
class EXPCL_PANDA_GOBJ AdaptiveLru : public Namable {
PUBLISHED:
  explicit AdaptiveLru(const std::string &name, size_t max_size);
  ~AdaptiveLru();

  INLINE size_t get_total_size() const;
  INLINE size_t get_max_size() const;
  INLINE void set_max_size(size_t max_size);
  size_t count_active_size() const;

  INLINE void consider_evict();
  INLINE void evict_to(size_t target_size);
  void begin_epoch();

  INLINE bool validate();

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

  // The following methods are specific to AdaptiveLru, and do not exist in
  // the SimpleLru implementation.  In most cases, the defaults will be
  // sufficient, so you do not need to mess with them.
  INLINE void set_weight(PN_stdfloat weight);
  INLINE PN_stdfloat get_weight() const;

  INLINE void set_max_updates_per_frame(int max_updates_per_frame);
  INLINE int get_max_updates_per_frame() const;

private:
   public:  // temp hack
  enum LruPagePriority {
    LPP_Highest = 0,
    LPP_High = 10,
    LPP_New = 20,
    LPP_Normal = 25,
    LPP_Intermediate = 30,
    LPP_Low = 40,
    LPP_TotalPriorities = 50,
  };

  INLINE PN_stdfloat calculate_exponential_moving_average(PN_stdfloat value, PN_stdfloat average) const;

  void do_partial_lru_update(int num_updates);
  void update_page(AdaptiveLruPage *page);

  void do_add_page(AdaptiveLruPage *page);
  void do_remove_page(AdaptiveLruPage *page);
  void do_access_page(AdaptiveLruPage *page);

  void do_evict_to(size_t target_size, bool hard_evict);
  bool do_validate();

  LightMutex _lock;

  size_t _total_size;
  size_t _max_size;

  unsigned int _current_frame_identifier;
  PN_stdfloat _weight;
  int _max_updates_per_frame;

  // This array of linked lists keeps all of the active pages, grouped by
  // priority.  We reshuffle pages among these lists as they are accessed and
  // as they change priority in update_page().
  AdaptiveLruPageDynamicList _page_array[LPP_TotalPriorities];

/*
 * This linked list keeps all of the active pages, in arbitrary order.  This
 * list exists solely to allow us to incrementally update pages without having
 * to iterate through the complex lists above and worry about losing our
 * place.  New pages are added to the tail.  We also move pages from the head
 * to the tail of this list in do_partial_lru_update() as we process each page
 * with update_page().  Pages do not move within this list other that that.
 */
  AdaptiveLruPageStaticList _static_list;

  friend class AdaptiveLruPage;
};

/**
 * One atomic piece that may be managed by a AdaptiveLru chain.  To use this
 * class, inherit from it and override evict_lru().
 *
 * This class multiply inherits from two classes which in turn both inherit
 * from LinkedListNode.  This is just a sneaky C++ trick to allow this class
 * to inherit from LinkedListNode twice, so that pages can be stored on two
 * different linked lists simultaneously.  The AdaptiveLru class depends on
 * this; it maintains its pages in two different lists, one grouped by
 * priority, and one in order by next partial update needs.
 */
class EXPCL_PANDA_GOBJ AdaptiveLruPage : public AdaptiveLruPageDynamicList, public AdaptiveLruPageStaticList {
PUBLISHED:
  explicit AdaptiveLruPage(size_t lru_size);
  AdaptiveLruPage(const AdaptiveLruPage &copy);
  void operator = (const AdaptiveLruPage &copy);

  virtual ~AdaptiveLruPage();

  INLINE AdaptiveLru *get_lru() const;

  void enqueue_lru(AdaptiveLru *lru);
  INLINE void dequeue_lru();

  INLINE void mark_used_lru() const;
  INLINE void mark_used_lru(AdaptiveLru *lru);

  INLINE size_t get_lru_size() const;
  INLINE void set_lru_size(size_t lru_size);

  virtual void evict_lru();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

  // Not defined in SimpleLruPage.
  unsigned int get_num_frames() const;
  unsigned int get_num_inactive_frames() const;

private:
  AdaptiveLru *_lru;

  size_t _lru_size;
  int _priority;

  unsigned int _first_frame_identifier;     // Frame first added.
  unsigned int _current_frame_identifier;   // Frame last accessed.
  unsigned int _update_frame_identifier;    // Frame last updated.

  int _current_frame_usage;
  int _last_frame_usage;
  int _update_total_usage;

  PN_stdfloat _average_frame_utilization;

  friend class AdaptiveLru;
};

inline std::ostream &operator << (std::ostream &out, const AdaptiveLru &lru) {
  lru.output(out);
  return out;
}

inline std::ostream &operator << (std::ostream &out, const AdaptiveLruPage &page) {
  page.output(out);
  return out;
}

#if 0
BEGIN_PUBLISH
void test_adaptive_lru();
END_PUBLISH
#endif

#include "adaptiveLru.I"

#endif
