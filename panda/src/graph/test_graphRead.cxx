// Filename: test_graphRead.cxx
// Created by:  jason (15Jun00)

#include "nodeRelation.h"
#include "namedNode.h"
#include "pt_NamedNode.h"
#include <bamWriter.h>
#include <ipc_file.h>

#include <indent.h>

int main() {
  string test_file = "graphTest.out";
  datagram_file stream(test_file);
  BamReader manager(&stream);
  stream.open(file::FILE_READ);
  
  manager.init();
  
  PT_NamedNode r = DCAST(NamedNode, manager.read_object());
  PT_NamedNode a = DCAST(NamedNode, manager.read_object());
  PT_NamedNode b = DCAST(NamedNode, manager.read_object());
  PT_NamedNode aa = DCAST(NamedNode, manager.read_object());
  PT_NamedNode ab = DCAST(NamedNode, manager.read_object());
  PT_NamedNode ba = DCAST(NamedNode, manager.read_object());
  
  NodeRelation *r_a = DCAST(NodeRelation, manager.read_object());
  NodeRelation *r_b = DCAST(NodeRelation, manager.read_object());
  NodeRelation *a_aa = DCAST(NodeRelation, manager.read_object());
  NodeRelation *a_ab = DCAST(NodeRelation, manager.read_object());
  NodeRelation *b_ba = DCAST(NodeRelation, manager.read_object());
  
  manager.resolve();

  r_a->output(nout);
  nout << endl;
  r_b->output(nout);
  nout << endl;
  a_aa->output(nout);
  nout << endl;
  a_ab->output(nout);
  nout << endl;
  b_ba->output(nout);
  nout << endl;

  stream.close();

  return 0;
}
