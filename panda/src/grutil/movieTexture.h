// Filename: movieTexture.h
// Created by: jyelon (01Aug2007)
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

#ifndef MOVIETEXTURE_H
#define MOVIETEXTURE_H

#include "pandabase.h"

#ifdef HAVE_AUDIO

#include "movieVideo.h"
#include "audioSound.h"
#include "pipelineCycler.h"
#include "cycleData.h"
#include "cycleDataWriter.h"
#include "cycleDataReader.h"

////////////////////////////////////////////////////////////////////
//       Class : MovieTexture
// Description : A texture that fetches video frames from an
//               underlying object of class Movie.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL MovieTexture : public Texture {
PUBLISHED:
  MovieTexture(const string &name);
  MovieTexture(MovieVideo *video);
private:
  MovieTexture(const MovieTexture &copy);
PUBLISHED:
  virtual ~MovieTexture();

  INLINE double get_video_length() const;
  INLINE int get_video_width() const;
  INLINE int get_video_height() const;
  INLINE LVecBase2f get_tex_scale() const;
  
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
  
public:
  static PT(Texture) make_texture();
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;
 
protected:
  virtual PT(Texture) do_make_copy();
  void do_assign(const MovieTexture &copy);

  virtual void do_reload_ram_image();
  virtual bool get_keep_ram_image() const;
  virtual bool do_has_bam_rawdata() const;
  virtual void do_get_bam_rawdata();

  virtual bool do_adjust_this_size(int &x_size, int &y_size, const string &name, 
                                   bool for_padding);

  virtual bool do_read_one(const Filename &fullpath, const Filename &alpha_fullpath,
                           int z, int n, int primary_file_num_channels, int alpha_file_channel,
                           const LoaderOptions &options,
                           bool header_only, BamCacheRecord *record);
  virtual bool do_load_one(const PNMImage &pnmimage, const string &name,
                           int z, int n, const LoaderOptions &options);
  bool do_load_one(PT(MovieVideoCursor) color, PT(MovieVideoCursor) alpha, 
                   int z, const LoaderOptions &options);
  virtual void do_allocate_pages();

  class VideoPage {
  public:
    PT(MovieVideoCursor) _color;
    PT(MovieVideoCursor) _alpha;
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

    double         _clock;
    bool           _playing;
    int            _loop_count;
    int            _loops_total;
    double         _play_rate;
    PT(AudioSound) _synchronize;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  
  void do_recalculate_image_properties(CDWriter &cdata, const LoaderOptions &options);

public:
  static void register_with_read_factory();

  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  virtual void do_write_datagram_rawdata(BamWriter *manager, Datagram &dg);
  virtual void do_fillin_rawdata(DatagramIterator &scan, BamReader *manager);

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
