// Filename: test_graphWrite.cxx
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
  BamWriter manager(&stream);
  stream.open(file::FILE_WRITE);
  
  PT_NamedNode r = new NamedNode("r");

  PT_NamedNode a = new NamedNode("a");
  PT_NamedNode b = new NamedNode("b");

  PT_NamedNode aa = new NamedNode("aa");
  PT_NamedNode ab = new NamedNode("ab");
  PT_NamedNode ba = new NamedNode("ba");

  NodeRelation *r_a =
    new NodeRelation(r, a, 0, NodeRelation::get_class_type());
  NodeRelation *r_b =
    new NodeRelation(r, b, 0, NodeRelation::get_class_type());

  NodeRelation *a_aa =
    new NodeRelation(a, aa, 0, NodeRelation::get_class_type());
  NodeRelation *a_ab =
    new NodeRelation(a, ab, 0, NodeRelation::get_class_type());
  NodeRelation *b_ba =
    new NodeRelation(b, ba, 0, NodeRelation::get_class_type());
  
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

  if (manager.init())
  {
    manager.write_object(r);
    manager.write_object(a);
    manager.write_object(b);
    manager.write_object(aa);
    manager.write_object(ab);
    manager.write_object(ba);

    manager.write_object(r_a);
    manager.write_object(r_b);
    manager.write_object(a_aa);
    manager.write_object(a_ab);
    manager.write_object(b_ba);
  }

  stream.close();
  return 0;
}
