// Filename: lwoToEggConverter.h
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOTOEGGCONVERTER_H
#define LWOTOEGGCONVERTER_H

#include <pandatoolbase.h>

#include <lwoHeader.h>
#include <eggData.h>
#include <pointerTo.h>

class CLwoLayer;
class CLwoPoints;
class CLwoPolygons;

////////////////////////////////////////////////////////////////////
// 	 Class : LwoToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from the data represented by the LwoHeader.
//               Reading and writing the egg and lwo structures is
//               left to the user.
////////////////////////////////////////////////////////////////////
class LwoToEggConverter {
public:
  LwoToEggConverter(EggData &egg_data);
  ~LwoToEggConverter();

  bool convert_lwo(const LwoHeader *lwo_header);

  INLINE EggGroupNode *get_egg_root() const;
  CLwoLayer *get_layer(int number) const;

private:
  void collect_lwo();
  void make_egg();
  void connect_egg();

  void slot_layer(int number);
  CLwoLayer *make_generic_layer();

  EggData &_egg_data;
  CPT(LwoHeader) _lwo_header;
  
  CLwoLayer *_generic_layer;
  typedef vector<CLwoLayer *> Layers;
  Layers _layers;

  typedef vector<CLwoPoints *> Points;
  Points _points;

  typedef vector<CLwoPolygons *> Polygons;
  Polygons _polygons;

  bool _error;
};

#include "lwoToEggConverter.I"

#endif


