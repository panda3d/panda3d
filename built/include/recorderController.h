/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderController.h
 * @author drose
 * @date 2004-01-25
 */

#ifndef RECORDERCONTROLLER_H
#define RECORDERCONTROLLER_H

#include "pandabase.h"
#include "datagramOutputFile.h"
#include "datagramInputFile.h"
#include "recorderTable.h"
#include "recorderHeader.h"
#include "typedReferenceCount.h"
#include "factory.h"

class RecorderBase;
class RecorderFrame;

/**
 * This object manages the process of recording the user's runtime inputs to a
 * bam file so that the session can be recreated later.
 */
class EXPCL_PANDA_RECORDER RecorderController : public TypedReferenceCount {
PUBLISHED:
  RecorderController();
  ~RecorderController();

  bool begin_record(const Filename &filename);
  bool begin_playback(const Filename &filename);
  void close();

  INLINE time_t get_start_time() const;

  INLINE void set_random_seed(int random_seed);
  INLINE int get_random_seed() const;

  INLINE bool is_recording() const;
  INLINE bool is_playing() const;
  INLINE bool is_open() const;
  INLINE const Filename &get_filename() const;

  INLINE bool is_error();
  INLINE double get_clock_offset() const;
  INLINE int get_frame_offset() const;

  INLINE void add_recorder(const std::string &name, RecorderBase *recorder);
  INLINE bool has_recorder(const std::string &name) const;
  INLINE RecorderBase *get_recorder(const std::string &name) const;
  INLINE bool remove_recorder(const std::string &name);

  INLINE void set_frame_tie(bool frame_tie);
  INLINE bool get_frame_tie() const;

  void record_frame();
  void play_frame();

public:
  typedef Factory<RecorderBase> RecorderFactory;

  INLINE static RecorderFactory *get_factory();

private:
  INLINE static void create_factory();
  RecorderFrame *read_frame();

private:
  RecorderHeader _header;
  double _clock_offset;
  int _frame_offset;

  Filename _filename;
  DatagramOutputFile _dout;
  DatagramInputFile _din;
  BamWriter *_writer;
  BamReader *_reader;
  bool _frame_tie;

  // _user_table is directly modified by the user.
  RecorderTable *_user_table;
  bool _user_table_modified;

  // In playback mode, _file_table represents the table as read directly from
  // the session file, with default recorders in each slot.
  RecorderTable *_file_table;

  // In playback mode, _active_table is the result of the merge of _file_table
  // and _user_table, with a default recorder or a user-specified recorder in
  // each active slot.
  RecorderTable *_active_table;

  RecorderFrame *_next_frame;
  bool _eof;

  static RecorderFactory *_factory;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "RecorderController",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class RecorderFrame;
};

#include "recorderController.I"

#endif
