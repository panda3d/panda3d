// Filename: dcClassParameter.h
// Created by:  drose (29Jun04)
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

#ifndef DCSWITCHPARAMETER_H
#define DCSWITCHPARAMETER_H

#include "dcbase.h"
#include "dcParameter.h"

class DCSwitch;

////////////////////////////////////////////////////////////////////
//       Class : DCSwitchParameter
// Description : This represents a switch object used as a
//               parameter itself, which packs the appropriate fields
//               of the switch into the message.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCSwitchParameter : public DCParameter {
public:
  DCSwitchParameter(DCSwitch *dswitch);
  DCSwitchParameter(const DCSwitchParameter &copy);

PUBLISHED:
  virtual DCSwitchParameter *as_switch_parameter();
  virtual DCParameter *make_copy() const;
  virtual bool is_valid() const;

  DCSwitch *get_switch() const;

public:
  virtual DCPackerInterface *get_nested_field(int n) const;

  const DCPackerInterface *apply_switch(const char *value_data, size_t length) const;

  virtual void output_instance(ostream &out, bool brief, const string &prename, 
                               const string &name, const string &postname) const;
  virtual void write_instance(ostream &out, bool brief, int indent_level,
                              const string &prename, const string &name,
                              const string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

private:
  DCSwitch *_dswitch;
};

#endif
