// Filename: eggTextureCards.h
// Created by:  drose (21Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGTEXTURECARDS_H
#define EGGTEXTURECARDS_H

#include <pandatoolbase.h>

#include <eggWriter.h>
#include <eggTexture.h>
#include <luse.h>

class EggVertexPool;
class EggVertex;

////////////////////////////////////////////////////////////////////
// 	 Class : EggTextureCards
// Description : Generates an egg file featuring a number of polygons,
//               one for each named texture.  This is a support
//               program for getting textures through egg-palettize.
////////////////////////////////////////////////////////////////////
class EggTextureCards : public EggWriter {
public:
  EggTextureCards();

protected:
  virtual bool handle_args(Args &args);

  static bool dispatch_wrap_mode(const string &opt, const string &arg, void *var);

private:
  bool scan_texture(const Filename &filename, LVecBase4d &geometry);
  void make_vertices(const LPoint4d &geometry, EggVertexPool *vpool,
                     EggVertex *&v1, EggVertex *&v2, EggVertex *&v3, EggVertex *&v4);

public:
  void run();

  LVecBase4d _polygon_geometry;
  LVecBase2d _pixel_scale;
  bool _got_pixel_scale;
  Colorf _polygon_color;
  vector_string _texture_names;
  EggTexture::WrapMode _wrap_mode;
};

#endif

