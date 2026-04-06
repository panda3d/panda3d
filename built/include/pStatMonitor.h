/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatMonitor.h
 * @author drose
 * @date 2000-07-08
 */

#ifndef PSTATMONITOR_H
#define PSTATMONITOR_H

#include "pandatoolbase.h"

#include "pStatClientData.h"
#include "pStatView.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "luse.h"

#include "pmap.h"

class PStatCollectorDef;
class PStatGraph;
class PStatServer;

/**
 * This is an abstract class that presents the interface to any number of
 * different front-ends for the stats monitor.  One of these will be created
 * by the PStatMonitor as each client is connected; this class is responsible
 * for opening up a new strip-chart graph or whatever is appropriate.  It
 * defines a number of empty virtual functions that will be called as new data
 * becomes available.
 */
class PStatMonitor : public ReferenceCount {
public:
  // The following functions are primarily for use by internal classes to set
  // up the monitor.
  PStatMonitor(PStatServer *server = nullptr);
  virtual ~PStatMonitor();

  void hello_from(const std::string &hostname, const std::string &progname,
                  int pid);
  void bad_version(const std::string &hostname, const std::string &progname,
                   int pid,
                   int client_major, int client_minor,
                   int server_major, int server_minor);
  void set_client_data(PStatClientData *client_data);

  bool write(const Filename &fn) const;
  bool read(const Filename &fn);

  void open_default_graphs();
  bool save_default_graphs() const;

  // The following functions are for use by user code to determine information
  // about the client data available.
  bool is_alive() const;
  void close();

  INLINE PStatServer *get_server();
  INLINE const PStatClientData *get_client_data() const;
  INLINE std::string get_collector_name(int collector_index);
  const LRGBColor &get_collector_color(int collector_index);
  void set_collector_color(int collector_index, const LRGBColor &color);
  void clear_collector_color(int collector_index);

  INLINE bool is_client_known() const;
  INLINE std::string get_client_hostname() const;
  INLINE std::string get_client_progname() const;
  INLINE int get_client_pid() const;
  INLINE bool has_read_filename() const;
  INLINE const Filename &get_read_filename() const;

  PStatView &get_view(int thread_index);
  PStatView &get_level_view(int collector_index, int thread_index);

  // The following virtual methods may be overridden by a derived monitor
  // class to customize behavior.

  virtual std::string get_monitor_name()=0;

  virtual void initialized();
  virtual void got_hello();
  virtual void got_bad_version(int client_major, int client_minor,
                               int server_major, int server_minor);
  virtual void new_collector(int collector_index);
  virtual void new_thread(int thread_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void remove_thread(int thread_index);

  virtual void lost_connection();
  virtual void idle();
  virtual bool has_idle();

  virtual bool is_thread_safe();

  virtual void user_guide_bars_changed();

  virtual PStatGraph *open_timeline();
  virtual PStatGraph *open_strip_chart(int thread_index, int collector_index, bool show_level);
  virtual PStatGraph *open_flame_graph(int thread_index, int collector_index = -1, int frame_number = -1);
  virtual PStatGraph *open_piano_roll(int thread_index);

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

protected:
  PStatServer *_server;

private:
  PT(PStatClientData) _client_data;

  bool _client_known;
  std::string _client_hostname;
  std::string _client_progname;
  int _client_pid;
  Filename _read_filename;

  typedef pmap<int, PStatView> Views;
  Views _views;
  typedef pmap<int, Views> LevelViews;
  LevelViews _level_views;

  typedef pmap<int, LRGBColor> Colors;
  Colors _colors;

public:
  typedef pset<PStatGraph *> Graphs;
  Graphs _timelines;
  Graphs _strip_charts;
  Graphs _flame_graphs;
  Graphs _piano_rolls;
};

#include "pStatMonitor.I"

#endif
