/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animInterface.h
 * @author drose
 * @date 2005-09-20
 */

#ifndef ANIMINTERFACE_H
#define ANIMINTERFACE_H

#include "pandabase.h"
#include "typeHandle.h"
#include "register_type.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;

/**
 * This is the fundamental interface for things that have a play/loop/stop
 * type interface for frame-based animation, such as animated characters.
 * This is the base class for AnimControl and other, similar classes.
 */
class EXPCL_PANDA_PUTIL AnimInterface {
protected:
  AnimInterface();
  AnimInterface(const AnimInterface &copy);

PUBLISHED:
  virtual ~AnimInterface();
  INLINE void play();
  INLINE void play(double from, double to);
  INLINE void loop(bool restart);
  INLINE void loop(bool restart, double from, double to);
  INLINE void pingpong(bool restart);
  INLINE void pingpong(bool restart, double from, double to);
  INLINE void stop();
  INLINE void pose(double frame);

  INLINE void set_play_rate(double play_rate);
  INLINE double get_play_rate() const;
  INLINE double get_frame_rate() const;
  virtual int get_num_frames() const;

  INLINE int get_frame() const;
  INLINE int get_next_frame() const;
  INLINE double get_frac() const;
  INLINE int get_full_frame() const;
  INLINE double get_full_fframe() const;
  INLINE bool is_playing() const;

  virtual void output(std::ostream &out) const;

PUBLISHED:
  MAKE_PROPERTY(play_rate, get_play_rate, set_play_rate);
  MAKE_PROPERTY(frame_rate, get_frame_rate);
  MAKE_PROPERTY(num_frames, get_num_frames);

  MAKE_PROPERTY(frame, get_frame);
  MAKE_PROPERTY(next_frame, get_next_frame);
  MAKE_PROPERTY(frac, get_frac);
  MAKE_PROPERTY(full_frame, get_full_frame);
  MAKE_PROPERTY(full_fframe, get_full_fframe);
  MAKE_PROPERTY(playing, is_playing);

protected:
  INLINE void set_frame_rate(double frame_rate);
  INLINE void set_num_frames(int num_frames);
  virtual void animation_activated();

private:
  enum PlayMode {
    PM_pose,
    PM_play,
    PM_loop,
    PM_pingpong,
  };

  // This data is not cycled, because it is a semi-permanent part of the
  // interface.  Also, some derivatives of AnimInterface don't even use it.
  int _num_frames;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PUTIL CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return AnimInterface::get_class_type();
    }

    void play(double from, double to);
    void loop(bool restart, double from, double to);
    void pingpong(bool restart, double from, double to);
    void pose(double frame);

    INLINE double get_frac() const;
    int get_full_frame(int increment) const;
    double get_full_fframe() const;
    bool is_playing() const;

    virtual void output(std::ostream &out) const;

    void internal_set_rate(double frame_rate, double play_rate);
    double get_f() const;

    double _frame_rate;

    PlayMode _play_mode;
    double _start_time;
    double _start_frame;
    double _play_frames;
    int _from_frame;
    int _to_frame;

    double _play_rate;
    double _effective_frame_rate;
    bool _paused;
    double _paused_f;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "AnimInterface");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const AnimInterface &ai);

#include "animInterface.I"

#endif
