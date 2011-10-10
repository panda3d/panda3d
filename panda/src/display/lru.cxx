// Filename: lru.cxx
// Created by: aignacio (12Dec05)
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

//#include "stdafx.h"

#define LRU_UNIT_TEST 0

#include <stdio.h>
#include <stdlib.h>
//#include <windows.h>

#include "lru.h"


static const int HIGH_PRIORITY_SCALE = 4;
static const int LOW_PRIORITY_RANGE = 25;

////////////////////////////////////////////////////////////////////
//     Function: Lru::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Lru::Lru (int maximum_memory, int maximum_pages, int maximum_page_types)
{
  if(this) {
    int  index;

    memset(&this->_m, 0, sizeof (LruVariables));

    this->_m.maximum_memory = maximum_memory;
    this->_m.maximum_pages  = maximum_pages;
    this->_m.maximum_page_types = maximum_page_types;
    this->_m.available_memory = maximum_memory;
    this->_m.current_frame_identifier = 1;
    this->_m.weight = 0.20f;

    this->set_maximum_frame_bandwidth_utilization(2000000.0f);

    for(index = 0; index < MAXIMUM_LRU_PAGE_TYPES; index++) {
      this->_m.page_in_function_array[index] = default_page_in_function;
      this->_m.page_out_function_array[index] = default_page_out_function;
    }

    if(maximum_pages > 0) {
      this -> _m.lru_page_pool = new LruPage * [maximum_pages];
      this -> _m.lru_page_free_pool = new LruPage * [maximum_pages];
      for(index = 0; index < maximum_pages; index++) {
        LruPage  * lru_page;

        lru_page = new LruPage ( );
        if(lru_page) {
          lru_page->_m.v.pre_allocated = true;
          this->_m.lru_page_pool[index] = lru_page;
        }
        else {
// ERROR
        }
      }
    }

    if(maximum_page_types > 0) {
      this -> _m.page_type_statistics_array =
        new PageTypeStatistics [maximum_page_types];
    }

#if ENABLE_MUTEX
    this -> _m.mutex = new Mutex ("lru");
#endif

  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Lru::~Lru ( )
{
  int        index;
  LruPage  * lru_page;

  // free pre-allocated LruPages
  if(this->_m.maximum_pages > 0) {
    if(this->_m.lru_page_free_pool) {
      for(index = 0; index < this->_m.maximum_pages; index++) {
        lru_page = this->_m.lru_page_pool[index];
        if(lru_page->_m.v.in_lru) {
          this->remove_page(lru_page);
        }

        delete lru_page;
      }

      delete this -> _m.lru_page_free_pool;
    }
    if(this->_m.lru_page_pool) {
      delete this -> _m.lru_page_pool;
    }
  }

  // free dynamically allocated LruPages
  for(index = 0; index < LPP_TotalPriorities; index++) {
    LruPage  * next_lru_page;

    lru_page = this->_m.lru_page_array[index];
    while(lru_page) {
      next_lru_page = lru_page->_m.next;

      delete  lru_page;

      lru_page = next_lru_page;
    }
  }

  if(this->_m.page_type_statistics_array) {
    delete this -> _m.page_type_statistics_array;
  }

#if ENABLE_MUTEX
  if(this->_m.mutex) {
    delete this -> _m.mutex;
  }
#endif

}

////////////////////////////////////////////////////////////////////
//     Function: LruPage::Constructor
//       Access: Protected
//  Description: Internal function only.
//               Call  Lru::allocate_page instead.
////////////////////////////////////////////////////////////////////
LruPage::LruPage ( )
{
  if(this) {
    memset(&this->_m, 0, sizeof (LruPageVariables));
    _m.name = "";   
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LruPage::Destructor
//       Access: Protected
//  Description: Internal function only.
//               Call  Lru::free_page instead.
////////////////////////////////////////////////////////////////////
LruPage::~LruPage ( )
{

}

////////////////////////////////////////////////////////////////////
//     Function: LruPage::change_priority
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void LruPage::change_priority (int delta)
{
  this->_m.priority_change += delta;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::register_lru_page_type
//       Access: Public
//  Description: Registers a specific type of page and its
//               required page in and out functions.
////////////////////////////////////////////////////////////////////
bool Lru::register_lru_page_type (int index,
  LruPageTypeFunction page_in_function,
  LruPageTypeFunction page_out_function)
{
  bool  state;

  state = false;
  if(index >= 0 && index < MAXIMUM_LRU_PAGE_TYPES) {
    this->_m.page_in_function_array[index] = page_in_function;
    this->_m.page_out_function_array[index] = page_out_function;
    state = true;
  }

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::allocate_page
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LruPage *Lru::allocate_page (int size)
{
  LruPage  * lru_page;

  lru_page = 0;
  if(size <= this->_m.maximum_memory) {
    if(this->_m.maximum_pages) {
      if(this->_m.total_lru_pages_in_free_pool > 0) {
        lru_page =
          this->_m.lru_page_free_pool [this->_m.total_lru_pages_in_free_pool - 1];
        this->_m.total_lru_pages_in_free_pool--;

        memset (&lru_page -> _m, 0, sizeof (LruPage::LruPageVariables));
        lru_page->_m.v.pre_allocated = true;
      }
      else {
        if(this->_m.total_lru_pages_in_pool < this->_m.maximum_pages) {
          lru_page = this->_m.lru_page_pool[this->_m.total_lru_pages_in_pool];
          this->_m.total_lru_pages_in_pool++;
        }
        else {
          // out of pre-allocated LruPages so dynamically allocate a page
          lru_page = new LruPage ( );
        }
      }
    }
    else {
      lru_page = new LruPage;
    }
    if(lru_page) {
      lru_page->_m.lru = this;
      lru_page->_m.size = size;
      lru_page->_m.first_frame_identifier = this->_m.current_frame_identifier;
      lru_page->_m.last_frame_identifier = this->_m.current_frame_identifier;

      lru_page->_m.v.allocated = true;
      lru_page->_m.identifier = this->_m.identifier;

      lru_page->_m.average_frame_utilization = 1.0f;

      this->_m.total_pages++;
      this->_m.identifier++;
    }
    else {

// ERROR: could not allocate LruPage

    }
  }
  else {

// ERROR: requested page size is larger than maximum memory size

  }

  return lru_page;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::update_start_update_lru_page
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Lru::update_start_update_lru_page (LruPage *lru_page)
{
  if(lru_page) {
    if(this->_m.start_update_lru_page == lru_page) {
      if(lru_page->_m.next) {
        this->_m.start_update_lru_page = lru_page->_m.next;
      }
      else {
        if((this->_m.start_priority_index + 1) >= LPP_TotalPriorities) {
          this->_m.start_priority_index = 0;
        }
        else {
          this->_m.start_priority_index = this->_m.start_priority_index + 1;
        }

        this->_m.start_update_lru_page = 0;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::free_page
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Lru::free_page (LruPage *lru_page)
{
  if(this->_m.total_pages > 0) {
    if(lru_page) {
      LruMutexHolder(this->_m.mutex);

      this->update_start_update_lru_page(lru_page);

      if(lru_page->_m.v.in_cache) {
        this->_m.available_memory += lru_page->_m.size;
      }

      if(lru_page->_m.v.pre_allocated) {
        if(this->_m.maximum_pages) {
          lru_page->_m.v.allocated = false;
          this->_m.lru_page_free_pool [this->_m.total_lru_pages_in_free_pool] =
            lru_page;
          this->_m.total_lru_pages_in_free_pool++;
        }
        else {
// ERROR: this case should not happen
        }
      }
      else {
        delete lru_page;
      }

      this->_m.total_pages--;
    }
  }
  else {

// ERROR: tried to free a page when 0 pages allocated

  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::add_page
//       Access: Public
//  Description: Adds a page to the LRU based on the given priority.
////////////////////////////////////////////////////////////////////
void Lru::add_page (LruPagePriority priority, LruPage *lru_page)
{
  if(lru_page) {
    LruMutexHolder(this->_m.mutex);

    LruPage * first_lru_page;

    lru_page->_m.priority = priority;

    first_lru_page = this->_m.lru_page_array[lru_page->_m.priority];
    if(first_lru_page) {
      first_lru_page->_m.previous = lru_page;
      lru_page->_m.next = first_lru_page;
    }

    this->_m.lru_page_array[lru_page->_m.priority] = lru_page;

    lru_page->_m.v.in_lru = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::add_cached_page
//       Access: Public
//  Description: Adds a page that is already paged in to the LRU
//               based on the given priority.
////////////////////////////////////////////////////////////////////
void Lru::add_cached_page (LruPagePriority priority, LruPage *lru_page)
{
  if(lru_page) {
    LruMutexHolder(this->_m.mutex);

    lru_page->_m.v.in_cache = true;

    if(lru_page->_m.size > this->_m.available_memory) {
      int  memory_required;

      memory_required = lru_page->_m.size - this->_m.available_memory;

      // unload page(s)
      this->page_out_lru(memory_required);
    }

    this->_m.available_memory -= lru_page->_m.size;

    this->add_page(priority, lru_page);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::remove_page
//       Access: Public
//  Description: Removes a page from the LRU.
////////////////////////////////////////////////////////////////////
void Lru::remove_page (LruPage *lru_page)
{
  if(this) {
    if(this->_m.total_pages > 0) {
      if(lru_page) {
        LruMutexHolder(this->_m.mutex);

        this->update_start_update_lru_page(lru_page);

        if(lru_page->_m.previous) {
          lru_page->_m.previous->_m.next = lru_page->_m.next;
          if(lru_page->_m.next) {
            lru_page->_m.next->_m.previous = lru_page->_m.previous;
          }
        }
        else {
          this->_m.lru_page_array[lru_page->_m.priority] =
            lru_page->_m.next;
          if(lru_page->_m.next) {
            lru_page->_m.next->_m.previous = 0;
          }
        }

        lru_page->_m.next = 0;
        lru_page->_m.previous = 0;

        lru_page->_m.v.in_lru = false;
      }
    }
    else {

// ERROR: tried to remove a page when 0 pages are allocated

    }
  }
  else {

// ERROR: Lru == 0, this should not happen

  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::lock_page
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Lru::lock_page (LruPage *lru_page)
{
  lru_page->_m.v.lock = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::unlock_page
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Lru::unlock_page (LruPage *lru_page)
{
  lru_page->_m.v.lock = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::access_page
//       Access: Public
//  Description: This must always be called before accessing or
//               using a page's memory since it pages in the page
//               if it is currently paged out.
////////////////////////////////////////////////////////////////////
void Lru::access_page (LruPage *lru_page)
{
  if(lru_page) {
    if(lru_page->_m.current_frame_identifier
       == this->_m.current_frame_identifier) {
      lru_page->_m.current_frame_usage++;
      this->_m.total_page_all_access_size += lru_page->_m.size;
    }
    else {
      // first update this frame
      lru_page->_m.last_frame_identifier = lru_page->_m.current_frame_identifier;
      lru_page->_m.current_frame_identifier = this->_m.current_frame_identifier;
      lru_page->_m.last_frame_usage = lru_page->_m.current_frame_usage;
      lru_page->_m.current_frame_usage = 1;
      lru_page->_m.total_frame_page_faults = 0;

      this->_m.total_page_access_size += lru_page->_m.size;
    }

    // check if the page is out
    if(lru_page->_m.v.in_cache == false) {
      bool  state;

      state = true;

      LruMutexHolder(this->_m.mutex);

      // check memory usage
      if(lru_page->_m.size > this->_m.available_memory) {
        int  memory_required;

        memory_required = lru_page->_m.size - this->_m.available_memory;

        // unload page(s)
        state = this->page_out_lru(memory_required);
      }

      // load the page in
      if(state) {
        // PAGE IN CALLBACK
        if(this->_m.page_in_function_array[lru_page->_m.v.type](lru_page)) {
          this->_m.available_memory -= lru_page->_m.size;
          lru_page->_m.v.in_cache = true;

          // CHANGE THE PAGE PRIORITY FROM LPP_PageOut TO LPP_New
          this->remove_page(lru_page);
          this->add_page(LPP_New, lru_page);

          this->_m.total_lifetime_page_ins++;
        }
      }

      lru_page->_m.total_frame_page_faults++;
      lru_page->_m.total_page_faults++;
    }

    lru_page->_m.total_usage++;
    lru_page->_m.update_total_usage++;

    this->_m.total_page_access++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::set_maximum_frame_bandwidth_utilization
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Lru::set_maximum_frame_bandwidth_utilization
  (PN_stdfloat maximum_frame_bandwidth_utilization)
{
  this->_m.maximum_frame_bandwidth_utilization =
    maximum_frame_bandwidth_utilization;

  this->_m.frame_bandwidth_factor = (PN_stdfloat) LPP_TotalPriorities
    / this->_m.maximum_frame_bandwidth_utilization;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::begin_frame
//       Access: Public
//  Description: This must be called before each frame.
////////////////////////////////////////////////////////////////////
void Lru::begin_frame ( )
{
  this->_m.current_frame_identifier++;

  this->_m.total_page_ins_last_frame = this->_m.total_page_ins;
  this->_m.total_page_outs = this->_m.total_page_outs;

  this->_m.total_page_ins = 0;
  this->_m.total_page_outs = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::update_page_priorities
//       Access: Public
//  Description: This updates the priority of a page that has a
//               change in priority.
////////////////////////////////////////////////////////////////////
void Lru::update_page_priorities (void)
{
  int index;
  LruPage *lru_page;

  for(index = 0; index < this->_m.total_lru_page_priority_changes; index++) {
    int priority;

    lru_page = this->_m.lru_page_priority_change_array[index];

    this->remove_page(lru_page);

    priority = (( int ) lru_page->_m.priority + lru_page->_m.priority_change);
    if(priority < 0) {
      priority = 0;
    }
    if(priority >= LPP_TotalPriorities) {
      priority = LPP_TotalPriorities - 1;
    }

    this->add_page((LruPagePriority) priority, lru_page);
    lru_page->_m.priority_change = 0;
  }
  this->_m.total_lru_page_priority_changes = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::update_lru_page
//       Access: Public
//  Description: This updates the page's average utilization.
//               Priority LPP_New is considered to be average usage
//               of 1.0 (which means the page is used once per frame
//               on average).  Priorities < LPP_New are for pages
//               used more than once per frame and Priorities >
//               LPP_New are for pages used less than once per frame.
//               If there was a change in priority, then adds it to
//               the array of lru pages with changed priorities
//               which will be updated later.
////////////////////////////////////////////////////////////////////
void Lru::update_lru_page (LruPage *lru_page)
{

#if LRU_UNIT_TEST
  if(false) {
    char  string[256];

    sprintf(string, "  UPDATE %d\n", lru_page->_m.identifier);
    OutputDebugString(string);
  }
#endif

  if(lru_page->_m.v.lock == false && lru_page->_m.v.in_cache) {
    int delta_priority;
    int lifetime_frames;

    delta_priority = 0;

    lifetime_frames = this->_m.current_frame_identifier -
      lru_page->_m.first_frame_identifier;
    if(lifetime_frames >= 1) {
      if(lru_page->_m.update_frame_identifier) {
        int target_priority;
        int integer_update_frames;
        PN_stdfloat update_frames;
        PN_stdfloat one_over_update_frames;
        PN_stdfloat update_average_frame_utilization;

        integer_update_frames = (this->_m.current_frame_identifier -
          lru_page->_m.update_frame_identifier);
        if(integer_update_frames > 0) {
          update_frames = ( PN_stdfloat ) integer_update_frames;
          one_over_update_frames = 1.0f / update_frames;

          update_average_frame_utilization =
            (PN_stdfloat) (lru_page->_m.update_total_usage)* one_over_update_frames;

          lru_page->_m.average_frame_utilization =
            calculate_exponential_moving_average(
               update_average_frame_utilization, this->_m.weight,
               lru_page->_m.average_frame_utilization);

          target_priority = lru_page->_m.priority;
          if(lru_page->_m.average_frame_utilization >= 1.0f) {
            int integer_average_frame_utilization;

            integer_average_frame_utilization =
              (int) ((lru_page->_m.average_frame_utilization - 1.0f) *
              (PN_stdfloat) HIGH_PRIORITY_SCALE);
            if(integer_average_frame_utilization >= LPP_New) {
              integer_average_frame_utilization = LPP_New;
            }
            integer_average_frame_utilization = LPP_New -
              integer_average_frame_utilization;
            target_priority = integer_average_frame_utilization;
          }
          else {
            int integer_average_frame_utilization;

            integer_average_frame_utilization = (int)
               (lru_page->_m.average_frame_utilization *
               (PN_stdfloat) LOW_PRIORITY_RANGE);
            integer_average_frame_utilization = LOW_PRIORITY_RANGE -
              integer_average_frame_utilization;
            target_priority = LPP_New + integer_average_frame_utilization;
          }

          delta_priority = target_priority - lru_page->_m.priority;
          lru_page->change_priority(delta_priority);
        }
      }

      lru_page->_m.update_frame_identifier = this->_m.current_frame_identifier;
      lru_page->_m.update_total_usage = 0;
    }

    if(lru_page->_m.priority_change) {
      if(this->_m.total_lru_page_priority_changes
         < FRAME_MAXIMUM_PRIORITY_CHANGES)
      {
        this->_m.lru_page_priority_change_array
          [this->_m.total_lru_page_priority_changes] = lru_page;
        this->_m.total_lru_page_priority_changes++;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::update_lru_page_old
//       Access: Public
//  Description: This updates the page's average utilization and
//               adds it to the array of pages with changed
//               priorities if there was a change in priority.
//               Old method.
////////////////////////////////////////////////////////////////////
void Lru::update_lru_page_old (LruPage *lru_page)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::update_entire_lru
//       Access: Public
//  Description: This updates all the pages in the Lru.
//               Lru::partial_lru_update should be called instead
//               due to performance reasons.
////////////////////////////////////////////////////////////////////
void Lru::update_entire_lru ( )
{
  if(this->_m.total_pages > 0) {
    int index;
    LruPage *lru_page;

    LruMutexHolder(this->_m.mutex);

    for(index = 0; index < LPP_TotalPriorities; index++) {

      LruPage  * next_lru_page;

      lru_page = this->_m.lru_page_array[index];
      while(lru_page) {
        next_lru_page = lru_page->_m.next;

        this->update_lru_page(lru_page);

        lru_page = next_lru_page;
      }
    }

    this->update_page_priorities( );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::partial_lru_update
//       Access: Public
//  Description: This only updates a number of pages up to the
//               specified maximum_updates.
////////////////////////////////////////////////////////////////////
void Lru::partial_lru_update (int maximum_updates)
{
  int total_page_updates;

  if (maximum_updates <= 0) {
    // enforce a minimum number of updates
    maximum_updates = 1;
  }

  total_page_updates = 0;
  if(this->_m.total_pages > 0) {
    int index;
    int start_priority;
    LruPage *lru_page;

    LruMutexHolder(this->_m.mutex);

    start_priority = this->_m.start_priority_index;

    {
      for(index = start_priority;  index < LPP_TotalPriorities; index++) {

        LruPage *next_lru_page;

        if(index == start_priority) {
          if(this->_m.start_update_lru_page) {
            lru_page = this->_m.start_update_lru_page;
          }
          else {
            lru_page = this->_m.lru_page_array[index];
          }
        }
        else {
          lru_page = this->_m.lru_page_array[index];
        }
        while(lru_page) {
          next_lru_page = lru_page->_m.next;

          this->update_lru_page(lru_page);

          total_page_updates++;
          if(total_page_updates >= maximum_updates) {
            if(next_lru_page) {
              this->_m.start_priority_index = index;
              this->_m.start_update_lru_page = next_lru_page;
            }
            else {
              if((index + 1) >= LPP_TotalPriorities) {
                this->_m.start_priority_index = 0;
              }
              else {
                this->_m.start_priority_index = index + 1;
              }

              this->_m.start_update_lru_page = 0;
            }

            break;
          }

          lru_page = next_lru_page;
        }

        if(total_page_updates >= maximum_updates) {
          break;
        }
      }
    }

    if(total_page_updates < maximum_updates) {
      for(index = 0;  index <= start_priority;  index++) {
        LruPage *next_lru_page;

        lru_page = this->_m.lru_page_array[index];
        while(lru_page) {
          next_lru_page = lru_page->_m.next;

          this->update_lru_page(lru_page);

          total_page_updates++;
          if(total_page_updates >= maximum_updates) {
            if(next_lru_page) {
              this->_m.start_priority_index = index;
              this->_m.start_update_lru_page = next_lru_page;
            }
            else {
              if((index + 1) >= LPP_TotalPriorities) {
                this->_m.start_priority_index = 0;
              }
              else {
                this->_m.start_priority_index = index + 1;
              }

              this->_m.start_update_lru_page = 0;
            }

            break;
          }

          lru_page = next_lru_page;
        }

        if(total_page_updates >= maximum_updates) {
          break;
        }
      }
    }

    if(total_page_updates < maximum_updates) {
      this->_m.start_priority_index  = 0;
      this->_m.start_update_lru_page = 0;
    }

    this->update_page_priorities( );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::unlock_all_pages
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Lru::unlock_all_pages (void)
{
  if(this->_m.total_pages > 0) {
    int  index;

    for(index = 0;  index < LPP_TotalPriorities; index++) {
      LruPage *lru_page;
      LruPage *next_lru_page;

      lru_page = this->_m.lru_page_array[index];
      while(lru_page) {
        next_lru_page = lru_page->_m.next;

        lru_page->_m.v.lock = false;

        lru_page = next_lru_page;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::page_out_lru
//       Access: Public
//  Description: Pages out the lowest priority pages until the
//               memory_required is satisfied.  This will unlock
//               all pages if needed.
////////////////////////////////////////////////////////////////////
bool Lru::page_out_lru (int memory_required)
{
  bool state;
  int attempts;

  state = false;
  attempts = 0;
  if(this->_m.total_pages > 0) {
    LruMutexHolder(this->_m.mutex);

    do {
      int index;
      int minimum_frame_identifier;
      
      minimum_frame_identifier = this->_m.current_frame_identifier - 1;

      // page out lower priority pages first
      for(index = LPP_PageOut - 1; index >= 0; index--) {
        LruPage *lru_page;
        LruPage *next_lru_page;

        lru_page = this->_m.lru_page_array[index];
        while(lru_page) {
          next_lru_page = lru_page->_m.next;

          if(attempts == 0 && (lru_page->_m.current_frame_identifier >= minimum_frame_identifier)) {
            // avoid swapping out pages used in the current and last frame on the first attempt
          }
          else {
            if(lru_page->_m.v.lock == false && lru_page->_m.v.in_cache) {
              memory_required -= lru_page->_m.size;
              this->_m.available_memory += lru_page->_m.size;
              lru_page->_m.v.in_cache = false;

              // PAGE OUT CALLBACK
              this->_m.page_out_function_array[lru_page->_m.v.type](lru_page);
              this->_m.total_lifetime_page_outs++;

              // MOVE THE PAGE TO THE LPP_PageOut PRIORITY
              this->remove_page(lru_page);
              this->add_page(LPP_PageOut, lru_page);

              if(memory_required <= 0) {
                break;
              }
            }
          }
          
          lru_page = next_lru_page;
        }

        if(memory_required <= 0) {
          break;
        }
      }

      if(memory_required > 0) {
        state = false;
      }
      else {
        state = true;
      }

      attempts++;
    } while(state == false && attempts < 2);
  }

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::count_priority_level_pages
//       Access: Public
//  Description: Debug function. Counts the number of pages for each
//               priority level.
////////////////////////////////////////////////////////////////////
void Lru::count_priority_level_pages (void)
{
  int  index;

  LruMutexHolder(this->_m.mutex);

  for(index = 0; index < LPP_TotalPriorities; index++) {
    int total_pages;
    LruPage *lru_page;
    LruPage *next_lru_page;

    total_pages = 0;
    lru_page = this->_m.lru_page_array[index];
    while(lru_page) {
      next_lru_page = lru_page->_m.next;

      total_pages++;

      lru_page = next_lru_page;
    }

    this->_m.lru_page_count_array[index] = total_pages;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Lru::calculate_lru_statistics
//       Access: Public
//  Description: Debug function.
////////////////////////////////////////////////////////////////////
void Lru::calculate_lru_statistics (void)
{
  LruMutexHolder(this->_m.mutex);

  if(this->_m.maximum_page_types > 0) {
    int  index;

    memset(this->_m.page_type_statistics_array, 0,
           sizeof (PageTypeStatistics) * this->_m.maximum_page_types);
    for(index = 0;  index < LPP_TotalPriorities;  index++) {
      LruPage *lru_page;
      LruPage *next_lru_page;
      PageTypeStatistics *page_type_statistics;

      lru_page = this->_m.lru_page_array[index];
      while(lru_page) {
        int  type;

        next_lru_page = lru_page->_m.next;

        type = lru_page->_m.v.type;
        page_type_statistics = &this->_m.page_type_statistics_array[type];
        page_type_statistics->total_pages++;

        if(lru_page->_m.v.in_cache) {
          page_type_statistics->total_pages_in++;
          page_type_statistics->total_memory_in += lru_page->_m.size;
        }
        else {
          page_type_statistics->total_pages_out++;
          page_type_statistics->total_memory_out += lru_page->_m.size;
        }

        lru_page = next_lru_page;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: calculate_exponential_moving_average
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat calculate_exponential_moving_average(PN_stdfloat value,
  PN_stdfloat weight, PN_stdfloat average)
{
  return ((value - average) * weight) + average;
}

////////////////////////////////////////////////////////////////////
//     Function: default_page_in_function
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool default_page_in_function(LruPage *lru_page)
{

#if LRU_UNIT_TEST
  char  string[256];

  sprintf(string, "  PAGE IN %d\n", lru_page->_m.identifier);
  OutputDebugString(string);
#endif

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: default_page_out_function
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool default_page_out_function(LruPage *lru_page)
{

#if LRU_UNIT_TEST
  char  string[256];

  sprintf(string, "  PAGE OUT %d\n", lru_page->_m.identifier);
  OutputDebugString(string);
#endif

  return true;
}

#if LRU_UNIT_TEST

////////////////////////////////////////////////////////////////////
//     Function: test_ema
//       Access:
//  Description: Unit test function for ema.
////////////////////////////////////////////////////////////////////
void test_ema(void)
{
  int    index;
  PN_stdfloat  usage;
  PN_stdfloat  weight;
  PN_stdfloat  average;

  weight  = 0.2;
  average = 1.0f;
  for(index = 0; index < 50; index++) {
    if(index < 25) {
      usage = (PN_stdfloat) (index & 0x01);
    }
    else {
      usage = 0.0f;
    }
    average =
      calculate_exponential_moving_average(usage, weight, average);

    char  string[256];
    sprintf(string, "%d  %f\n", index, average);
    OutputDebugString(string);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: test_lru
//       Access:
//  Description: Unit test function for Lru.
////////////////////////////////////////////////////////////////////
void test_lru(void)
{
  int maximum_memory;
  int maximum_pages;
  int maximum_page_types;
  Lru *lru;

  test_ema( );

  maximum_memory = 3000000;
  maximum_pages = 3;
  maximum_page_types = 4;
  lru = new Lru (maximum_memory, maximum_pages, maximum_page_types);
  if(lru) {
    lru->_m.minimum_memory = 1000000;

    LruPage *lru_page_0;
    LruPage *lru_page_1;
    LruPage *lru_page_2;
    LruPage *lru_page_3;
    LruPage *lru_page_4;
    LruPage *lru_page_5;

    lru_page_0 = lru->allocate_page(1000000);
    if(lru_page_0) {
      lru->add_page(LPP_PageOut, lru_page_0);
    }

    lru_page_1 = lru->allocate_page(1000000);
    if(lru_page_1) {
      lru->add_page(LPP_PageOut, lru_page_1);
    }

    lru_page_2 = lru->allocate_page(1000000);
    if(lru_page_2) {
      lru->add_page(LPP_PageOut, lru_page_2);
    }

    lru_page_3 = lru->allocate_page(1000000);
    if(lru_page_3) {
      lru->add_page(LPP_PageOut, lru_page_3);
    }

    lru_page_4 = lru->allocate_page(1000000);
    if(lru_page_4) {
      lru->add_page(LPP_PageOut, lru_page_4);
    }

    lru_page_5 = lru->allocate_page(1000000);
    if(lru_page_5) {
      lru->add_page(LPP_PageOut, lru_page_5);
    }

    int index;
    int total_frames;

    total_frames = 300;
    for(index = 0;  index < total_frames;  index++) {
      char  string[256];

      sprintf(string, "FRAME %d\n", index);
      OutputDebugString(string);

      lru->begin_frame( );

      if(index <= 5) {
        lru->access_page(lru_page_0);
      }

      lru->access_page(lru_page_1);
      lru->access_page(lru_page_1);

      if(index & 0x01) {
        lru->access_page(lru_page_2);
      }

      if((index % 10) == 0) {
        lru->access_page(lru_page_3);
      }

      if(index >= 100) {
        lru->access_page(lru_page_4);
      }

      if(index >= 200) {
        lru->access_page(lru_page_5);
      }

      if(false) {
        lru->update_entire_lru( );
      }
      else {
        int  maximum_updates;

        maximum_updates = 3;
        lru->partial_lru_update(maximum_updates);
      }
    }

    if(!true) {
      lru->remove_page(lru_page_2);
      lru->free_page(lru_page_2);

      lru->remove_page(lru_page_3);
      lru->free_page(lru_page_3);

      lru->remove_page(lru_page_1);
      lru->free_page(lru_page_1);
    }

    delete lru;
  }
}

#endif
