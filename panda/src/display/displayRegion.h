// Filename: displayRegion.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef DISPLAYREGION_H
#define DISPLAYREGION_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <referenceCount.h>
#include <camera.h>

#include <list>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class GraphicsLayer;
class GraphicsChannel;
class GraphicsWindow;
class GraphicsPipe;

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegion
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DisplayRegion : public ReferenceCount {
public:

  DisplayRegion(GraphicsLayer *);
  DisplayRegion(GraphicsLayer *,
		const float l, const float r,
		const float b, const float t);
  DisplayRegion(int xsize, int ysize);
  virtual ~DisplayRegion();

  INLINE void get_dimensions(float &l, float &r, float &b, float &t) const;
  void set_dimensions(float l, float r, float b, float t);

  INLINE GraphicsLayer *get_layer() const;
  GraphicsChannel *get_channel() const;
  GraphicsWindow *get_window() const;
  GraphicsPipe *get_pipe() const;

  void set_camera(const PT(Camera) &camera);
  INLINE PT(Camera) get_camera() const;

  INLINE void set_active(bool active);
  INLINE bool is_active() const;

  INLINE void compute_pixels(const int x, const int y);
  INLINE void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  INLINE void get_region_pixels(int &xo, int &yo, int &w, int &h) const;

  INLINE int get_pixel_width() const;
  INLINE int get_pixel_height() const;

  void output(ostream &out) const;

protected:
	
  float _l;
  float _r;
  float _b;
  float _t;

  int _pl;
  int _pr;
  int _pb;
  int _pt;

  GraphicsLayer *_layer;
  PT(Camera) _camera;

  bool _active;

private:
  DisplayRegion(const DisplayRegion &);
  DisplayRegion& operator=(const DisplayRegion &);

  friend class GraphicsLayer;
};

INLINE ostream &operator << (ostream &out, const DisplayRegion &dr);

#include "displayRegion.I"

#endif /* DISPLAYREGION_H */
