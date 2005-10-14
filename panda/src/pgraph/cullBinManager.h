// Filename: cullBinManager.h
// Created by:  drose (27Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CULLBINMANAGER_H
#define CULLBINMANAGER_H

#include "pandabase.h"
#include "cullBin.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"
#include "vector_int.h"

class CullResult;
class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : CullBinManager
// Description : This is a global object that maintains the collection
//               of named CullBins in the world.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBinManager {
protected:
  CullBinManager();
  ~CullBinManager();

PUBLISHED:
  enum BinType {
    BT_invalid,
    BT_unsorted,
    BT_state_sorted,
    BT_back_to_front,
    BT_front_to_back,
    BT_fixed,
  };

  int add_bin(const string &name, BinType type, int sort);
  void remove_bin(int bin_index);

  INLINE int get_num_bins() const;
  INLINE int get_bin(int n) const;
  int find_bin(const string &name) const;

  INLINE string get_bin_name(int bin_index) const;
  INLINE BinType get_bin_type(int bin_index) const;

  INLINE int get_bin_sort(int bin_index) const;
  INLINE void set_bin_sort(int bin_index, int sort);

  void write(ostream &out) const;

  static CullBinManager *get_global_ptr();

public:
  // This interface is only intended to be used by CullResult.
  PT(CullBin) make_new_bin(int bin_index, GraphicsStateGuardianBase *gsg);

private:
  void do_sort_bins();
  void setup_initial_bins();
  static BinType parse_bin_type(const string &bin_type);

  class EXPCL_PANDA BinDefinition {
  public:
    bool _in_use;
    string _name;
    BinType _type;
    int _sort;
  };
  typedef pvector<BinDefinition> BinDefinitions;
  BinDefinitions _bin_definitions;

  class SortBins {
  public:
    INLINE SortBins(CullBinManager *manager);
    INLINE bool operator () (int a, int b) const;
    CullBinManager *_manager;
  };

  typedef pmap<string, int> BinsByName;
  BinsByName _bins_by_name;

  typedef vector_int SortedBins;
  SortedBins _sorted_bins;
  bool _bins_are_sorted;
  bool _unused_bin_index;

  static CullBinManager *_global_ptr;
  friend class SortBins;
};

EXPCL_PANDA ostream &
operator << (ostream &out, CullBinManager::BinType bin_type);

#include "cullBinManager.I"

#endif
