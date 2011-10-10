// Filename: lru.h
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

#ifndef LRU_H
#define LRU_H

#define ENABLE_MUTEX 1

#if ENABLE_MUTEX
#include "lightMutex.h"
#include "lightMutexHolder.h"
#define LruMutexHolder(mutex) MutexHolder(mutex)
#else
#define LruMutexHolder(mutex)
#endif


static const int MAXIMUM_LRU_PAGE_TYPES = 8;
static const int FRAME_MAXIMUM_PRIORITY_CHANGES = 256;


class Lru;
class LruPage;

enum LruPagePriority
{
  LPP_Highest = 0,
  LPP_High = 10,
  LPP_New = 20,
  LPP_Normal = 25,
  LPP_Intermediate = 30,
  LPP_Low = 40,
  LPP_TotalPriorities = 50,

  LPP_PageOut = LPP_TotalPriorities - 1
};

typedef union _LruPageType
{
  void *pointer;

}
LruPageType;

typedef struct
{
  int total_pages;
  int total_pages_in;
  int total_pages_out;
  int total_memory_in;
  int total_memory_out;
}
PageTypeStatistics;

typedef bool (*LruPageTypeFunction) (LruPage *lru_page);

class EXPCL_PANDA_DISPLAY LruPage
{

protected:

  LruPage ( );
  ~LruPage ( );
  void change_priority (int delta);

public:

  typedef struct _LruPageVariables
  {
    LruPageType lru_page_type;  // pointer to memory type

    int size;
    LruPagePriority priority;
    int priority_change;

    struct
    {
      unsigned int type : 8;
      unsigned int lock : 1;
      unsigned int in_cache : 1;
      unsigned int in_memory : 1;
      unsigned int on_disk : 1;
      unsigned int pre_allocated : 1;
      unsigned int allocated : 1;
      unsigned int in_lru : 1;
    } v;

    int first_frame_identifier;   // creation time
    int last_frame_identifier;    // previous time page was used
    int current_frame_identifier;
    int update_frame_identifier;

    int current_frame_usage;
    int last_frame_usage;

    int total_frame_page_faults;
    int total_page_faults;

    int total_usage;
    int update_total_usage;

    int identifier;

    PN_stdfloat average_frame_utilization;

    LruPage *previous;
    LruPage *next;
    Lru *lru;
    
    string name;
  }
  LruPageVariables;

  LruPageVariables _m;

  friend class Lru;
};

////////////////////////////////////////////////////////////////////
//       Class : Lru
// Description : Least Recently Used algorithm implementation:
// In the Lru, each "memory page" has an associated class LruPage.
// The Lru has a range of priorities from LPP_Highest to
// LPP_PagedOut. Each priority has a doubly linked list of LruPages.
// The algorithim uses an adaptive method based on the average
// utilization of each page per frame (or time slice). The
// average utilization is calculated with an exponetial moving
// average. This is superior to a standard average since a standard
// average becomes less and less adaptive the longer a page exists.
// The average utilization is used to set the priority of each page.
// A higher average utilization automatically raises the priority
// of a page and a lower average utilization automatically lowers
// the priority of a page. Therefore, pages with a higher average
// utilization have a higher chance of being kept in memory or
// cached and pages with a lower average utilization have a higher
// chance of being paged out.  When a page is paged in and there
// is not enough memory available, then the lowest priority pages
// will be paged out first until there is enough memory available.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY Lru
{
public:

  Lru (int maximum_memory, int maximum_pages, int maximum_page_types);
  ~Lru ( );

  bool register_lru_page_type (int index, LruPageTypeFunction page_in_function, LruPageTypeFunction page_out_function);

  LruPage *allocate_page (int size);
  void update_start_update_lru_page (LruPage *lru_page);

  void free_page (LruPage *lru_page);

  void add_page (LruPagePriority priority, LruPage *lru_page);
  void add_cached_page (LruPagePriority priority, LruPage *lru_page);
  void remove_page (LruPage *lru_page);

  void lock_page (LruPage *lru_page);
  void unlock_page (LruPage *lru_page);

  void access_page (LruPage *lru_page);

  void set_maximum_frame_bandwidth_utilization (PN_stdfloat maximum_frame_bandwidth_utilization);

  void begin_frame ( );

  void update_entire_lru ( );
  void partial_lru_update (int maximum_updates);

  // set maximum number of page updates per frame
  // pause/resume updates/current_frame_identifier

  void unlock_all_pages (void);

  void count_priority_level_pages (void);

  void calculate_lru_statistics (void);

  bool page_out_lru (int memory_required);

private:
  void update_page_priorities (void);
  void update_lru_page (LruPage *lru_page);
  void update_lru_page_old (LruPage *lru_page);

public:
  typedef struct _LruVariables
  {
    // LruPagePriority lists
    LruPage *lru_page_array [LPP_TotalPriorities];

    int total_pages;
    int available_memory;
    int current_frame_identifier;

    int maximum_memory;
    int minimum_memory; // target amount of memory to keep free if possible
    int maximum_page_types;

    int total_lifetime_page_ins;
    int total_lifetime_page_outs;

    int total_page_ins_last_frame;
    int total_page_outs_last_frame;

    int total_page_ins;
    int total_page_outs;

    int total_page_access;
    double total_page_access_size;
    double total_page_all_access_size;

    int start_priority_index;
    LruPage *start_update_lru_page;

    int identifier; // the number of pages created during the lifetime of the LRU

    PN_stdfloat weight; // used for exponential moving average
    PN_stdfloat maximum_frame_bandwidth_utilization;

    PN_stdfloat frame_bandwidth_factor;

    LruPageTypeFunction page_in_function_array [MAXIMUM_LRU_PAGE_TYPES];
    LruPageTypeFunction page_out_function_array [MAXIMUM_LRU_PAGE_TYPES];

    int total_lru_page_priority_changes;
    LruPage *lru_page_priority_change_array [FRAME_MAXIMUM_PRIORITY_CHANGES];

    int maximum_pages;
    int total_lru_pages_in_pool;
    int total_lru_pages_in_free_pool;
    LruPage **lru_page_pool;
    LruPage **lru_page_free_pool;

    int lru_page_count_array [LPP_TotalPriorities];
    PageTypeStatistics *page_type_statistics_array;

    void *context;  // user specified data

#if ENABLE_MUTEX
    Mutex *mutex;
#endif
  }
  LruVariables;

  LruVariables _m;

  friend class LruPage;
};

PN_stdfloat calculate_exponential_moving_average (PN_stdfloat value, PN_stdfloat weight, PN_stdfloat average);
bool default_page_in_function (LruPage *lru_page);
bool default_page_out_function (LruPage *lru_page);

void test_ema (void);
void test_lru (void);

#endif
