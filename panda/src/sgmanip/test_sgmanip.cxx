// Filename: test_sgmanip.cxx
// Created by:  drose (18Feb00)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "nodePath.h"

#include <pt_Node.h>
#include <notify.h>
#include <pointerTo.h>
#include <namedNode.h>
#include <renderRelation.h>

int
main() {
  PT_Node render = new NamedNode("render");

  PT_Node apples = new NamedNode("apples");
  PT(RenderRelation) apples_arc = new RenderRelation(render, apples);
  PT_Node mcintosh = new NamedNode("mcintosh");
  PT(RenderRelation) mcintosh_arc = new RenderRelation(apples, mcintosh);
  PT_Node grannysmith = new NamedNode("grannysmith");
  PT(RenderRelation) grannysmith_arc = new RenderRelation(apples, grannysmith);
  PT_Node delicious = new NamedNode("delicious");
  PT(RenderRelation) delicious_arc = new RenderRelation(apples, delicious);
  PT_Node golden = new NamedNode("golden");
  PT(RenderRelation) golden_arc = new RenderRelation(delicious, golden);
  PT_Node red = new NamedNode("red");
  PT(RenderRelation) red_arc = new RenderRelation(delicious, red);

  PT_Node oranges = new NamedNode("oranges");
  PT(RenderRelation) oranges_arc = new RenderRelation(render, oranges);
  PT_Node navel = new NamedNode("navel");
  PT(RenderRelation) navel_arc = new RenderRelation(oranges, navel);
  PT_Node red_orange = new NamedNode("red");
  PT(RenderRelation) red_orange_arc = new RenderRelation(oranges, red_orange);

  NodePath render_path(render);
  render_path.ls();

  NodePath del_path(render_path, "apples/delicious");
  nout << "\ndel_path = " << del_path << "\n";
  nout << "del_path.get_parent() = " << del_path.get_parent() << "\n";
  NodePath red_path(del_path, "red");
  nout << "red_path = " << red_path << "\n";
  NodePath del_path2(render_path, delicious);
  nout << "del_path2 = " << del_path2 << "\n";
  NodePath mc_path(NodePath(del_path, 1), "mcintosh");
  nout << "mc_path = " << mc_path << "\n";
  NodePath apples_path(del_path, 1);
  nout << "apples_path = " << apples_path << "\n";
  NodePath oranges_path(render_path, oranges);
  nout << "oranges_path = " << oranges_path << "\n";

  /*
  nout << "\ndel_path2.share_with(del_path) = " 
       << del_path2.share_with(del_path) << "\n";
  nout << "del_path2 = " << del_path2 << "\n";
  */

  nout << "\nChildren of del_path:\n";
  del_path.get_children().write(nout, 2);
  nout << "Sibs:\n";
  del_path.get_siblings().write(nout, 2);
  nout << "Descendants:\n";
  del_path.ls(nout);
  nout << "Nodes:\n";
  int i;
  for (i = 0; i < del_path.get_num_nodes(); i++) {
    nout << "  " << *del_path.get_node(i) << "\n";
  }
  nout << "Arcs:\n";
  for (i = 0; i < del_path.get_num_arcs(); i++) {
    nout << "  " << *del_path.get_arc(i) << "\n";
  }
  nout << "As strings:\n";
  for (i = 0; i <= del_path.get_num_nodes(); i++) {
    nout << "  " << i << ": " << del_path.as_string(i) << "\n";
  }
  nout << "Connectivity = " << del_path.verify_connectivity() << "\n";

  nout << "\nEquivalence to self = " << (del_path == del_path) << "\n";
  nout << "Equivalence to del_path2 = " << (del_path == del_path2) << "\n";
  nout << "Equivalence to mc_path = " << (del_path == mc_path) << "\n";

  nout << "\nAll paths to red_orange:\n";
  render_path.find_all_paths_down_to(red_orange).write(nout, 2);

  nout << "\nMoving del_path to oranges\n";
  del_path.reparent_to(oranges_path);
  nout << "del_path = " << del_path << "\n"
       << "red_path = " << red_path << "\n"
       << "del_path2 = " << del_path2 << "\n"
       << "mc_path = " << mc_path << "\n"
       << "apples_path = " << apples_path << "\n"
       << "oranges_path = " << oranges_path << "\n";
  
  render_path.ls(nout);

  nout << "\nInstancing del_path back under apples\n";
  NodePath new_del_path = del_path.instance_to(apples_path);
  nout << "del_path = " << del_path << "\n"
       << "new_del_path = " << new_del_path << "\n"
       << "red_path = " << red_path << "\n"
       << "del_path2 = " << del_path2 << "\n"
       << "mc_path = " << mc_path << "\n"
       << "apples_path = " << apples_path << "\n"
       << "oranges_path = " << oranges_path << "\n";

  render_path.ls(nout);

  nout << "\nRepairing del_path2, result is " 
       << del_path2.repair_connectivity(render_path) << "\n";
  nout << "del_path2 = " << del_path2 << "\n\n";

  return (0);
}

