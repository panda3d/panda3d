// Filename: lru.h
// Created by: aignacio (12Dec05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights
// reserved.
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef LRU_H
#define LRU_H


#define MAXIMUM_LRU_PAGE_TYPES 8
#define FRAME_MAXIMUM_PRIORITY_CHANGES 256


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

class LruPage
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
    };

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

    float average_frame_utilization;

    LruPage *previous;
    LruPage *next;
    Lru *lru;
  }
  LruPageVariables;

  LruPageVariables _m;

  friend class Lru;
};


class Lru
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

  void set_maximum_frame_bandwidth_utilization (float maximum_frame_bandwidth_utilization);

  void begin_frame ( );

  void update_entire_lru ( );
  void partial_lru_update (int maximum_updates);

  // set maximum number of page updates per frame
  // pause/resume updates/current_frame_identifier

  void unlock_all_pages (void);

  void count_priority_level_pages (void);

  void calculate_lru_statistics (void);

private:
  bool page_out_lru (int memory_required);
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
    int minimum_memory;    // target amount of memory to keep free if possible
    int maximum_page_types;

    int total_lifetime_page_ins;
    int total_lifetime_page_outs;

    int total_page_ins_last_frame;
    int total_page_outs_last_frame;

    int total_page_ins;
    int total_page_outs;

    int total_page_access;

    int minimum_page_out_frames;           // number of frames required before page out
    int maximum_page_updates_per_frame;    // unused pages

    int start_priority_index;
    LruPage *start_update_lru_page;

    int identifier;  // this is also the number of pages created during the lifetime of the LRU

    float weight;  // used for exponential moving average
    float maximum_frame_bandwidth_utilization;

    float frame_bandwidth_factor;

    LruPageTypeFunction page_in_function_array [MAXIMUM_LRU_PAGE_TYPES];
    LruPageTypeFunction page_out_function_array [MAXIMUM_LRU_PAGE_TYPES];

    int total_lru_page_priority_changes;
    LruPage *lru_page_priority_change_array [FRAME_MAXIMUM_PRIORITY_CHANGES];

    void *context;

    int lru_page_count_array [LPP_TotalPriorities];

    int maximum_pages;
    int total_lru_pages_in_pool;
    int total_lru_pages_in_free_pool;
    LruPage **lru_page_pool;
    LruPage **lru_page_free_pool;

    PageTypeStatistics *page_type_statistics_array;
  }
  LruVariables;

  LruVariables _m;

  friend class LruPage;
};

float calculate_exponential_moving_average (float value, float weight, float average);
bool default_page_in_function (LruPage *lru_page);
bool default_page_out_function (LruPage *lru_page);

void test_ema (void);
void test_lru (void);

#endif
