// Filename: vrml2egg.h
// Created by:  drose (24Jun99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#ifndef VRML2EGG_H
#define VRML2EGG_H

#include <eggBase.h>

#include <mstring.h>
#include <map.h>

class VrmlNode;
struct SFNodeRef;
class EggGroup;
class pfMatrix;

////////////////////////////////////////////////////////////////////
// 	 Class : MainProgram
// Description : The vrml2egg program class.  This handles the user
//               input and gets things going.
////////////////////////////////////////////////////////////////////
class MainProgram : public EggBase {
public:
  MainProgram() : EggBase("") {
  }

  virtual void Help();
  virtual void Usage();
  virtual void ShowOpts();
  
  virtual boolean
  HandleGetopts(char flag, char *optarg, int &optind,
		int argc, char **argv);

  virtual boolean
  HandleArgs(int &argc, char **&argv);

  void vrml_group(const VrmlNode *node, EggGroup *group,
		  const pfMatrix &net_transform);
  void vrml_transform(const VrmlNode *node, EggGroup *group,
		      const pfMatrix &net_transform);
  void vrml_shape(const VrmlNode *node, EggGroup *group,
		  const pfMatrix &net_transform);

  void vrml_grouping_node(const SFNodeRef &vrml, EggGroup *egg,
			  const pfMatrix &net_transform,
			  void (MainProgram::*process_func)
			  (const VrmlNode *node, EggGroup *group,
			   const pfMatrix &net_transform));

  void vrml_node(const SFNodeRef &vrml,
		 EggGroup *egg, const pfMatrix &net_transform);

  typedef map<String, VrmlNode *> Nodes;

  void get_all_defs(SFNodeRef &vrml, Nodes &nodes);

  void DoIt();

  typedef map<const VrmlNode *, EggGroup *> Instances;
  Instances _instances;

  string _filename;
};


#endif
