// Filename: pStatCollectorDef.h
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATCOLLECTORDEF_H
#define PSTATCOLLECTORDEF_H

#include <pandabase.h>

#include <luse.h>

class Datagram;
class DatagramIterator;
class PStatClient;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatCollectorDef
// Description : Defines the details about the Collectors: the name,
//               the suggested color, etc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatCollectorDef {
public:
  PStatCollectorDef();
  PStatCollectorDef(int index, const string &name);
  void set_parent(const PStatCollectorDef &parent);

  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

  int _index;
  string _name;
  int _parent_index;
  RGBColorf _suggested_color;
  int _sort;
  string _level_units;
  float _suggested_scale;
};

#endif

