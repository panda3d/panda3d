/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file adaptiveLru.cxx
 * @author drose
 * @date 2008-09-03
 */

#include "adaptiveLru.h"
#include "config_gobj.h"
#include "clockObject.h"
#include "indent.h"

using std::cerr;
using std::ostream;

static const int HIGH_PRIORITY_SCALE = 4;
static const int LOW_PRIORITY_RANGE = 25;

/**
 *
 */
AdaptiveLru::
AdaptiveLru(const std::string &name, size_t max_size) :
  Namable(name)
{
  _total_size = 0;
  _max_size = max_size;

  _current_frame_identifier = 0;
  _weight = adaptive_lru_weight;
  _max_updates_per_frame = adaptive_lru_max_updates_per_frame;

  // Initialize our list heads to empty.
  _static_list._next = &_static_list;
  _static_list._prev = &_static_list;

  int index;
  for (index = 0; index < LPP_TotalPriorities; ++index) {
    _page_array[index]._next = &_page_array[index];
    _page_array[index]._prev = &_page_array[index];
  }
}

/**
 *
 */
AdaptiveLru::
~AdaptiveLru() {
#ifndef NDEBUG
  // We're shutting down.  Force-remove everything remaining, but don't
  // explicitly evict it (that would force vertex buffers to write themselves
  // to disk unnecessarily).

  while (_static_list._next != &_static_list) {
    nassertv(_static_list._next != nullptr);
    AdaptiveLruPage *page = (AdaptiveLruPage *)(AdaptiveLruPageStaticList *)_static_list._next;

    page->_lru = nullptr;
    ((AdaptiveLruPageDynamicList *)page)->remove_from_list();
    ((AdaptiveLruPageStaticList *)page)->remove_from_list();
  }
#endif
}

/**
 * This only updates a number of pages up to the specified maximum_updates.
 * Assumes the lock is held.
 */
void AdaptiveLru::
do_partial_lru_update(int num_updates) {
  // Iterate sequentially through the static list of pages.  As we process
  // each page, pop it and push it back on the tail.  Stop when we have
  // processed num_updates, or come back to the starting one.

  AdaptiveLruPageStaticList *start_node = (AdaptiveLruPageStaticList *)_static_list._next;
  if (start_node == &_static_list) {
    // List is empty.
    return;
  }

  AdaptiveLruPageStaticList *node = start_node;
  do {
    AdaptiveLruPageStaticList *next = (AdaptiveLruPageStaticList *)node->_next;
    if (--num_updates <= 0) {
      return;
    }

    update_page((AdaptiveLruPage *)node);
    node->remove_from_list();
    node->insert_before(&_static_list);
    node = next;
  } while (node != start_node && node != &_static_list);
}

/**
 * This updates the page's average utilization.  Priority LPP_New is
 * considered to be average usage of 1.0 (which means the page is used once
 * per frame on average).  Priorities < LPP_New are for pages used more than
 * once per frame and Priorities > LPP_New are for pages used less than once
 * per frame.
 *
 * Assumes the lock is held.
 */
void AdaptiveLru::
update_page(AdaptiveLruPage *page) {
  int target_priority = page->_priority;
  unsigned int lifetime_frames = _current_frame_identifier - page->_first_frame_identifier;
  if (lifetime_frames > 0) {
    if (page->_update_frame_identifier) {
      unsigned int update_frames;

      update_frames = (_current_frame_identifier - page->_update_frame_identifier);
      if (update_frames > 0) {
        if (page->_update_total_usage > 0) {
          PN_stdfloat update_average_frame_utilization =
            (PN_stdfloat) (page->_update_total_usage) / (PN_stdfloat)update_frames;

          page->_average_frame_utilization =
            calculate_exponential_moving_average(update_average_frame_utilization,
                                                 page->_average_frame_utilization);
        } else {
          page->_average_frame_utilization *= 1.0f - _weight;
        }

        target_priority = page->_priority;
        if (page->_average_frame_utilization >= 1.0f) {
          int integer_average_frame_utilization;

          integer_average_frame_utilization =
            (int) ((page->_average_frame_utilization - 1.0f) *
                   (PN_stdfloat) HIGH_PRIORITY_SCALE);
          if (integer_average_frame_utilization >= LPP_New) {
            integer_average_frame_utilization = LPP_New;
          }
          integer_average_frame_utilization = LPP_New -
            integer_average_frame_utilization;
          target_priority = integer_average_frame_utilization;
        } else {
          int integer_average_frame_utilization;

          integer_average_frame_utilization = (int)
            (page->_average_frame_utilization *
             (PN_stdfloat) LOW_PRIORITY_RANGE);
          integer_average_frame_utilization = LOW_PRIORITY_RANGE -
            integer_average_frame_utilization;
          target_priority = LPP_New + integer_average_frame_utilization;
        }
      }
    }

    page->_update_frame_identifier = _current_frame_identifier;
    page->_update_total_usage = 0;
  }

  if (target_priority != page->_priority) {
    page->_priority = std::min(std::max(target_priority, 0), LPP_TotalPriorities - 1);
    ((AdaptiveLruPageDynamicList *)page)->remove_from_list();
    ((AdaptiveLruPageDynamicList *)page)->insert_before(&_page_array[page->_priority]);
  }
}

/**
 * Adds the page to the LRU for the first time, or marks it recently-accessed
 * if it has already been added.
 *
 * If lru is NULL, it means to remove this page from its LRU.
 */
void AdaptiveLruPage::
enqueue_lru(AdaptiveLru *lru) {
  if (lru != _lru && _lru != nullptr) {
    // It was previously on a different LRU.  Remove it first.
    _lru->do_remove_page(this);
    _lru = nullptr;
  }

  if (lru == _lru) {
    if (_lru != nullptr) {
      // It's already on this LRU.  Access it.
      _lru->do_access_page(this);
    }
  } else {
    nassertv(lru != nullptr);
    // Add it to a new LRU.
    _lru = lru;

    _priority = AdaptiveLru::LPP_New;
    _first_frame_identifier = _lru->_current_frame_identifier;
    _current_frame_identifier = _lru->_current_frame_identifier;
    _lru->do_add_page(this);
  }
}

/**
 * Returns the total size of the pages that were enqueued since the last call
 * to begin_epoch().
 */
size_t AdaptiveLru::
count_active_size() const {
  size_t counted_size = 0;

  AdaptiveLruPageStaticList *node = (AdaptiveLruPageStaticList *)_static_list._next;
  while (node != &_static_list) {
    AdaptiveLruPage *page = (AdaptiveLruPage *)node;
    if (page->_current_frame_identifier + 1 >= _current_frame_identifier) {
      counted_size += page->_lru_size;
    }
    node = (AdaptiveLruPageStaticList *)node->_next;
  }

  return counted_size;
}

/**
 * Marks the end of the previous epoch and the beginning of the next one.
 * This will evict any objects that are pending eviction, and also update any
 * internal bookkeeping.
 */
void AdaptiveLru::
begin_epoch() {
  LightMutexHolder holder(_lock);
  do_partial_lru_update(_max_updates_per_frame);
  if (_total_size > _max_size) {
    do_evict_to(_max_size, false);
  }

  _current_frame_identifier = ClockObject::get_global_clock()->get_frame_count();
}

/**
 *
 */
void AdaptiveLru::
output(ostream &out) const {
  LightMutexHolder holder(_lock);
  out << "AdaptiveLru " << get_name()
      << ", " << _total_size << " of " << _max_size;
}

/**
 *
 */
void AdaptiveLru::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";

  // We write out the list backwards.  Things we write out first are the
  // freshest in the LRU.  Things at the end of the list will be the next to
  // be evicted.

  LightMutexHolder holder(_lock);

  int index;
  for (index = 0; index < LPP_TotalPriorities; ++index) {
    AdaptiveLruPageDynamicList *node = (AdaptiveLruPageDynamicList *)_page_array[index]._prev;
    if (node != &_page_array[index]) {
      indent(out, indent_level + 2) << "Priority " << index << ":\n";
      while (node != &_page_array[index]) {
        AdaptiveLruPage *page = (AdaptiveLruPage *)node;
        indent(out, indent_level + 4) << *page;

        if (page->_current_frame_identifier + 1 >= _current_frame_identifier) {
          out << " (active)";
        }
        out << "\n";

        node = (AdaptiveLruPageDynamicList *)node->_prev;
      }
    }
  }

#ifndef NDEBUG
  ((AdaptiveLru *)this)->do_validate();
#endif
}

/**
 * Adds a new page the the LRU.
 */
void AdaptiveLru::
do_add_page(AdaptiveLruPage *page) {
  nassertv(page != nullptr && page->_lru == this);
  LightMutexHolder holder(_lock);

  _total_size += page->_lru_size;
  ((AdaptiveLruPageDynamicList *)page)->insert_before(&_page_array[page->_priority]);
  ((AdaptiveLruPageStaticList *)page)->insert_before(&_static_list);
}

/**
 * Removes a page from the LRU.
 */
void AdaptiveLru::
do_remove_page(AdaptiveLruPage *page) {
  nassertv(page != nullptr && page->_lru == this);
  LightMutexHolder holder(_lock);

  _total_size -= page->_lru_size;
  ((AdaptiveLruPageDynamicList *)page)->remove_from_list();
  ((AdaptiveLruPageStaticList *)page)->remove_from_list();
}

/**
 * Marks a page accessed.
 */
void AdaptiveLru::
do_access_page(AdaptiveLruPage *page) {
  nassertv(page != nullptr && page->_lru == this);
  LightMutexHolder holder(_lock);

  if (page->_current_frame_identifier == _current_frame_identifier) {
    // This is the second or more time this page is accessed this frame.
    ++(page->_current_frame_usage);

  } else {
    // This page has not yet been accessed this frame.  Update it.
    page->_current_frame_identifier = _current_frame_identifier;
    page->_last_frame_usage = page->_current_frame_usage;
    page->_current_frame_usage = 1;
  }

  // Move it to the tail of its priority list.
  ((AdaptiveLruPageDynamicList *)page)->remove_from_list();
  ((AdaptiveLruPageDynamicList *)page)->insert_before(&_page_array[page->_priority]);

  ++(page->_update_total_usage);
}

/**
 * Evicts pages until the LRU is within the indicated size.  Assumes the lock
 * is already held.  If hard_evict is false, does not evict "active" pages
 * that were added within this epoch.
 */
void AdaptiveLru::
do_evict_to(size_t target_size, bool hard_evict) {
  int attempts;

  attempts = 0;
  do {
    // page out lower priority pages first
    int index;
    for (index = LPP_TotalPriorities - 1; index >= 0; index--) {

      // Store the current end of the list.  If pages re-enqueue themselves
      // during this traversal, we don't want to visit them twice.
      AdaptiveLruPageDynamicList *end = (AdaptiveLruPageDynamicList *)_page_array[index]._prev;

      AdaptiveLruPageDynamicList *node = (AdaptiveLruPageDynamicList *)_page_array[index]._next;

      while (node != &_page_array[index]) {
        AdaptiveLruPageDynamicList *next = (AdaptiveLruPageDynamicList *)node->_next;
        AdaptiveLruPage *page = (AdaptiveLruPage *)node;

        if (attempts == 0 &&
            (page->_current_frame_identifier + 1 >= _current_frame_identifier)) {
          // avoid swapping out pages used in the current and last frame on
          // the first attempt

        } else {
          // We must release the lock while we call evict_lru().
          _lock.unlock();
          page->evict_lru();
          _lock.lock();

          if (_total_size <= target_size) {
            // We've evicted enough to satisfy our target.
            return;
          }
        }
        if (node == end) {
          // We've reached the former end of the list.  Stop here; everything
          // after has been re-queued.
          break;
        }
        node = next;
      }
    }
    attempts++;
  } while (hard_evict && attempts < 2);
}

/**
 * Checks that the LRU is internally consistent.  Assume the lock is already
 * held.
 */
bool AdaptiveLru::
do_validate() {
  bool okflag = true;
  pset<AdaptiveLruPage *> pages;

  // First, walk through the dynamic pages.
  size_t counted_size = 0;
  int index;
  for (index = 0; index < LPP_TotalPriorities; ++index) {
    AdaptiveLruPageDynamicList *node = (AdaptiveLruPageDynamicList *)_page_array[index]._next;
    while (node != &_page_array[index]) {
      AdaptiveLruPage *page = (AdaptiveLruPage *)node;
      counted_size += page->_lru_size;
      if (page->_priority != index) {
        nout << "page " << page << " has priority " << page->_priority
             << " but is in queue " << index << "\n";
        okflag = false;
      }

      bool inserted_ok = pages.insert(page).second;
      if (!inserted_ok) {
        nout << "page " << page << " appears more than once in the dynamic index\n";
        okflag = false;
      }
      node = (AdaptiveLruPageDynamicList *)node->_next;
    }
  }

  if (counted_size != _total_size) {
    nout << "count " << counted_size << " bytes in dynamic index, but have " << _total_size << " on record\n";
    okflag = false;
  }

  // Now, walk through the static pages.
  counted_size = 0;
  AdaptiveLruPageStaticList *node = (AdaptiveLruPageStaticList *)_static_list._next;
  while (node != &_static_list) {
    AdaptiveLruPage *page = (AdaptiveLruPage *)node;
    counted_size += page->_lru_size;

    if (pages.find(page) == pages.end()) {
      nout << "page " << page << " appears in dynamic index, but not in static index (or multiple times in static index)\n";
      okflag = false;
    } else {
      pages.erase(page);
    }
    node = (AdaptiveLruPageStaticList *)node->_next;
  }

  if (counted_size != _total_size) {
    nout << "count " << counted_size << " bytes in static index, but have " << _total_size << " on record\n";
    okflag = false;
  }

  return okflag;
}

/**
 *
 */
AdaptiveLruPage::
AdaptiveLruPage(size_t lru_size) :
  _lru(nullptr),
  _lru_size(lru_size),
  _priority(0),
  _first_frame_identifier(0),
  _current_frame_identifier(0),
  _update_frame_identifier(0),
  _current_frame_usage(0),
  _last_frame_usage(0),
  _update_total_usage(0),
  _average_frame_utilization(1.0f)
{
}

/**
 *
 */
AdaptiveLruPage::
AdaptiveLruPage(const AdaptiveLruPage &copy) :
  _lru(nullptr),
  _lru_size(copy._lru_size),
  _priority(0),
  _first_frame_identifier(0),
  _current_frame_identifier(0),
  _update_frame_identifier(0),
  _current_frame_usage(0),
  _last_frame_usage(0),
  _update_total_usage(0),
  _average_frame_utilization(1.0f)
{
}

/**
 *
 */
void AdaptiveLruPage::
operator = (const AdaptiveLruPage &copy) {
  set_lru_size(copy.get_lru_size());
}

/**
 *
 */
AdaptiveLruPage::
~AdaptiveLruPage() {
  if (_lru != nullptr) {
    dequeue_lru();
  }
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void AdaptiveLruPage::
evict_lru() {
  dequeue_lru();
}

/**
 *
 */
void AdaptiveLruPage::
output(ostream &out) const {
  out << "page " << this << ", " << _lru_size;
}

/**
 *
 */
void AdaptiveLruPage::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 * Returns the number of frames since the page was first added to its LRU.
 * Returns 0 if it does not have an LRU.
 */
unsigned int AdaptiveLruPage::
get_num_frames() const {
  if (_lru == nullptr) {
    return 0;
  }
  return _lru->_current_frame_identifier - _first_frame_identifier;
}

/**
 * Returns the number of frames since the page was last accessed on its LRU.
 * Returns 0 if it does not have an LRU.
 */
unsigned int AdaptiveLruPage::
get_num_inactive_frames() const {
  if (_lru == nullptr) {
    return 0;
  }
  return _lru->_current_frame_identifier - _current_frame_identifier;
}


#if 0

/**
 * Unit test function for Lru.
 */
void
test_adaptive_lru() {
  int maximum_memory = 3000000;
  AdaptiveLru *lru = new AdaptiveLru("test", maximum_memory);

  AdaptiveLruPage *lru_page_0;
  AdaptiveLruPage *lru_page_1;
  AdaptiveLruPage *lru_page_2;
  AdaptiveLruPage *lru_page_3;
  AdaptiveLruPage *lru_page_4;
  AdaptiveLruPage *lru_page_5;

  lru_page_0 = new AdaptiveLruPage(1000000);
  cerr << "created lru_page_0: " << lru_page_0 << "\n";
  lru_page_0->enqueue_lru(lru);

  lru_page_1 = new AdaptiveLruPage(1000000);
  cerr << "created lru_page_1: " << lru_page_1 << "\n";
  lru_page_1->enqueue_lru(lru);

  lru_page_2 = new AdaptiveLruPage(1000000);
  cerr << "created lru_page_2: " << lru_page_2 << "\n";
  lru_page_2->enqueue_lru(lru);

  lru_page_3 = new AdaptiveLruPage(1000000);
  cerr << "created lru_page_3: " << lru_page_3 << "\n";
  lru_page_3->enqueue_lru(lru);

  lru_page_4 = new AdaptiveLruPage(1000000);
  cerr << "created lru_page_4: " << lru_page_4 << "\n";
  lru_page_4->enqueue_lru(lru);

  lru_page_5 = new AdaptiveLruPage(1000000);
  cerr << "created lru_page_5: " << lru_page_5 << "\n";
  lru_page_5->enqueue_lru(lru);

  int total_frames = 300;
  int index;
  for (index = 0;  index < total_frames;  index++) {
    cerr << "FRAME " << index << "\n";

    lru->begin_epoch();

    if (index <= 5) {
      lru_page_0->mark_used_lru(lru);
    }

    lru_page_1->mark_used_lru(lru);
    lru_page_1->mark_used_lru(lru);

    if (index & 0x01) {
      lru_page_2->mark_used_lru(lru);
    }

    if ((index % 10) == 0) {
      lru_page_3->mark_used_lru(lru);
    }

    if (index >= 100) {
      lru_page_4->mark_used_lru(lru);
    }

    if (index >= 200) {
      lru_page_5->mark_used_lru(lru);
    }

    if (!lru->validate()) {
      cerr << "Failed validation\n";
      break;
    }
  }

  delete lru;
  delete lru_page_0;
  delete lru_page_1;
  delete lru_page_2;
  delete lru_page_3;
  delete lru_page_4;
  delete lru_page_5;
}

#endif  // test_adaptive_lru
