// Filename: bamInfo.h
// Created by:  drose (02Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BAMINFO_H
#define BAMINFO_H

#include <pandatoolbase.h>

#include <eggToSomething.h>
#include <filename.h>
#include <pt_Node.h>
#include <sceneGraphAnalyzer.h>

#include <vector>

class TypedWriteable;

////////////////////////////////////////////////////////////////////
// 	 Class : BamInfo
// Description : 
////////////////////////////////////////////////////////////////////
class BamInfo : public ProgramBase {
public:
  BamInfo();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  bool get_info(const Filename &filename);
  void describe_scene_graph(Node *node);
  void describe_general_object(TypedWriteable *object);
  void list_hierarchy(Node *node, int indent_level);

  typedef vector<Filename> Filenames;
  Filenames _filenames;

  bool _verbose_geoms;
  bool _verbose_transitions;

  int _num_scene_graphs;
  SceneGraphAnalyzer _analyzer;
};

#endif
