/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieTexture.h
 * @author jyelon
 * @date 2007-08-01
 */

#ifndef MOVIETEXTURE_H
#define MOVIETEXTURE_H

#include "pandabase.h"

#ifdef HAVE_AUDIO

#include "movieVideo.h"
#include "movieVideoCursor.h"
#include "audioSound.h"
#include "pipelineCycler.h"
#include "cycleData.h"
#include "cycleDataWriter.h"
#include "cycleDataReader.h"

/**
 * A texture that fetches video frames from an underlying object of class
 * Movie.
 */
class EXPCL_PANDA_GRUTIL MovieTexture : public Texture {
PUBLISHED:
  explicit MovieTexture(const std::string &name);
  explicit MovieTexture(MovieVideo *video);
  MovieTexture(const MovieTexture &copy) = delete;
  virtual ~MovieTexture();

  INLINE double get_video_length() const;
  INLINE int get_video_width() const;
  INLINE int get_video_height() const;

  INLINE MovieVideoCursor *get_color_cursor(int page);
  INLINE MovieVideoCursor *get_alpha_cursor(int page);

  void   restart();
  void   stop();
  void   play();
  void   set_time(double t);
  double get_time() const;
  void   set_loop(bool enable);
  bool   get_loop() const;
  void   set_loop_count(int count);
  int    get_loop_count() const;
  void   set_play_rate(double play_rate);
  double get_play_rate() const;
  bool   is_playing() const;
  void   synchronize_to(AudioSound *sound);
  void   unsynchronize();

PUBLISHED:
  MAKE_PROPERTY(video_length, get_video_length);
  MAKE_PROPERTY(video_width, get_video_width);
  MAKE_PROPERTY(video_height, get_video_height);

  MAKE_PROPERTY(time, get_time, set_time);
  MAKE_PROPERTY(loop, get_loop, set_loop);
  MAKE_PROPERTY(loop_count, get_loop_count, set_loop_count);
  MAKE_PROPERTY(play_rate, get_play_rate, set_play_rate);
  MAKE_PROPERTY(playing, is_playing);

public:
  virtual void ensure_loader_type(const Filename &filename);

  static PT(Texture) make_texture();
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

protected:
  class CData;

  virtual PT(Texture) make_copy_impl() const;
  void do_assign(CData *cdata, Texture::CData *cdata_tex, const MovieTexture *copy,
                 const CData *cdata_copy, const Texture::CData *cdata_copy_tex);

  virtual void do_reload_ram_image(Texture::CData *cdata, bool allow_compression);
  virtual bool get_keep_ram_image() const;
  virtual bool do_has_bam_rawdata(const Texture::CData *cdata) const;
  virtual void do_get_bam_rawdata(Texture::CData *cdata);
  virtual bool do_can_reload(const Texture::CData *cdata) const;

  virtual bool do_adjust_this_size(const Texture::CData *cdata,
                                   int &x_size, int &y_size, const std::string &name,
                                   bool for_padding) const;

  virtual bool do_read_one(Texture::CData *cdata,
                           const Filename &fullpath, const Filename &alpha_fullpath,
                           int z, int n, int primary_file_num_channels, int alpha_file_channel,
                           const LoaderOptions &options,
                           bool header_only, BamCacheRecord *record);
  virtual bool do_load_one(Texture::CData *cdata,
                           const PNMImage &pnmimage, const std::string &name,
                           int z, int n, const LoaderOptions &options);
  virtual bool do_load_one(Texture::CData *cdata,
                           const PfmFile &pfm, const std::string &name,
                           int z, int n, const LoaderOptions &options);
  bool do_load_one(Texture::CData *cdata,
                   PT(MovieVideoCursor) color, PT(MovieVideoCursor) alpha,
                   int z, const LoaderOptions &options);
  virtual void do_allocate_pages(Texture::CData *cdata);

  class VideoPage {
  public:
    PT(MovieVideoCursor) _color;
    PT(MovieVideoCursor) _alpha;

    // The current (but not yet applied) frame for each video.
    PT(MovieVideoCursor::Buffer) _cbuffer;
    PT(MovieVideoCursor::Buffer) _abuffer;
  };

  typedef pvector<VideoPage> Pages;

  class EXPCL_PANDA_GRUTIL CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return MovieTexture::get_class_type();
    }

    Pages _pages;
    int _video_width;
    int _video_height;
    double _video_length;

    double _clock;
    bool _playing;
    int _loop_count;
    double _play_rate;
    PT(AudioSound) _synchronize;

    // The remaining values represent a local cache only; it is not preserved
    // through the pipeline.
    bool _has_offset;
    double _offset;
    int _true_loop_count;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  void do_recalculate_image_properties(CData *cdata, Texture::CData *cdata_tex,
                                       const LoaderOptions &options);

private:
  bool do_update_frames(const CData *cdata) const;

public:
  static void register_with_read_factory();

  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  virtual void do_write_datagram_rawdata(Texture::CData *cdata, BamWriter *manager, Datagram &me);
  virtual void do_fillin_rawdata(Texture::CData *cdata, DatagramIterator &scan, BamReader *manager);

  virtual void finalize(BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Texture::init_type();
    register_type(_type_handle, "MovieTexture",
                  Texture::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "movieTexture.I"

#endif  // HAVE_AUDIO

#endif
