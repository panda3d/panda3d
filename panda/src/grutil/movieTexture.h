// Filename: movieTexture.h
// Created by: jyelon (01Aug2007)
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

#ifndef MOVIETEXTURE_H
#define MOVIETEXTURE_H

#include "pandabase.h"
#include "movieVideo.h"
#include "audioSound.h"

////////////////////////////////////////////////////////////////////
//       Class : MovieTexture
// Description : A texture that fetches video frames from an
//               underlying object of class Movie.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL MovieTexture : public Texture {
PUBLISHED:
  MovieTexture(const string &name);
  MovieTexture(PT(MovieVideo) video);
protected:
  MovieTexture(const MovieTexture &copy);
PUBLISHED:
  virtual ~MovieTexture();

  virtual PT(Texture) make_copy();

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
  virtual void reload_ram_image();
  virtual bool get_keep_ram_image() const;
  virtual bool do_read_one(const Filename &fullpath, const Filename &alpha_fullpath,
                           int z, int n, int primary_file_num_channels, int alpha_file_channel,
                           bool header_only, BamCacheRecord *record);
  virtual bool do_load_one(const PNMImage &pnmimage, const string &name,
                           int z, int n);
  bool do_load_one(PT(MovieVideoCursor) color, PT(MovieVideoCursor) alpha, int z);

  class VideoPage {
  public:
    VideoPage();
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
  
  void recalculate_image_properties(CDWriter &cdata);

public:
  static void register_with_read_factory();

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


#endif
