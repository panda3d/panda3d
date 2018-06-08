/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClientData.h
 * @author drose
 * @date 2000-07-11
 */

#ifndef PSTATCLIENTDATA_H
#define PSTATCLIENTDATA_H

#include "pandatoolbase.h"

#include "pStatThreadData.h"

#include "pStatClientVersion.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "bitArray.h"

#include "pvector.h"
#include "vector_int.h"

class PStatReader;

/**
 * The data associated with a particular client, but not with any one
 * particular frame or thread: the list of collectors and threads, for
 * instance.
 */
class PStatClientData : public PStatClientVersion {
public:
  PStatClientData(PStatReader *reader);
  ~PStatClientData();

  bool is_alive() const;
  void close();

  int get_num_collectors() const;
  bool has_collector(int index) const;
  const PStatCollectorDef &get_collector_def(int index) const;
  std::string get_collector_name(int index) const;
  std::string get_collector_fullname(int index) const;
  bool set_collector_has_level(int index, int thread_index, bool flag);
  bool get_collector_has_level(int index, int thread_index) const;

  int get_num_toplevel_collectors() const;
  int get_toplevel_collector(int index) const;

  int get_num_threads() const;
  bool has_thread(int index) const;
  std::string get_thread_name(int index) const;
  const PStatThreadData *get_thread_data(int index) const;

  int get_child_distance(int parent, int child) const;


  void add_collector(PStatCollectorDef *def);
  void define_thread(int thread_index, const std::string &name = std::string());

  void record_new_frame(int thread_index, int frame_number,
                        PStatFrameData *frame_data);
private:
  void slot_collector(int collector_index);
  void update_toplevel_collectors();

private:
  bool _is_alive;
  PStatReader *_reader;

  class Collector {
  public:
    PStatCollectorDef *_def;
    BitArray _is_level;
  };

  typedef pvector<Collector> Collectors;
  Collectors _collectors;

  typedef vector_int ToplevelCollectors;
  ToplevelCollectors _toplevel_collectors;

  class Thread {
  public:
    std::string _name;
    PT(PStatThreadData) _data;
  };
  typedef pvector<Thread> Threads;
  Threads _threads;

  static PStatCollectorDef _null_collector;
  friend class PStatReader;
};

#endif
