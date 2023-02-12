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
  PStatClientData() = default;
  PStatClientData(PStatReader *reader);
  ~PStatClientData();

  void clear_dirty() const;
  bool is_dirty() const;

  bool is_alive() const;
  void close();

  double get_latest_time() const;

  int get_num_collectors() const;
  bool has_collector(int index) const;
  int find_collector(const std::string &fullname) const;
  const PStatCollectorDef &get_collector_def(int index) const;
  std::string get_collector_name(int index) const;
  std::string get_collector_fullname(int index) const;
  bool set_collector_has_level(int index, int thread_index, bool flag);
  bool get_collector_has_level(int index, int thread_index) const;

  int get_num_toplevel_collectors() const;
  int get_toplevel_collector(int index) const;

  int get_num_threads() const;
  bool has_thread(int index) const;
  int find_thread(const std::string &name) const;
  std::string get_thread_name(int index) const;
  const PStatThreadData *get_thread_data(int index) const;
  bool is_thread_alive(int index) const;

  int get_child_distance(int parent, int child) const;


  void add_collector(PStatCollectorDef *def);
  void define_thread(int thread_index, const std::string &name = std::string(),
                     bool mark_alive = false);
  void expire_thread(int thread_index);
  void remove_thread(int thread_index);

  void record_new_frame(int thread_index, int frame_number,
                        PStatFrameData *frame_data);

  void write_json(std::ostream &out, int pid = 0) const;
  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

private:
  void slot_collector(int collector_index);
  void update_toplevel_collectors();

private:
  bool _is_alive = false;
  mutable bool _is_dirty = false;
  PStatReader *_reader = nullptr;

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
    bool _is_alive = false;
  };
  typedef pvector<Thread> Threads;
  Threads _threads;

  static PStatCollectorDef _null_collector;
  friend class PStatReader;
};

#endif
