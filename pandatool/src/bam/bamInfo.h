// Filename: bamInfo.h
// Created by:  drose (02Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BAMINFO_H
#define BAMINFO_H

#include <pandatoolbase.h>

#include <programBase.h>
#include <filename.h>
#include <pt_Node.h>
#include <sceneGraphAnalyzer.h>

#include <vector>

class TypedWritable;

////////////////////////////////////////////////////////////////////
//       Class : BamInfo
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
  void describe_general_object(TypedWritable *object);
  void list_hierarchy(Node *node, int indent_level);

  typedef vector<Filename> Filenames;
  Filenames _filenames;

  bool _ls;
  bool _verbose_transitions;
  bool _verbose_geoms;

  int _num_scene_graphs;
  SceneGraphAnalyzer _analyzer;
};

#endif
