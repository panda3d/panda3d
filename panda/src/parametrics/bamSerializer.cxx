/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamSerializer.h
 * @author TidalGator
 * @date 2026-03-10
 */



#include "bamSerializer.h"


/**
* Basically retrieves info from the given BAM file.
*/
void NurbsSurfaceEvaluatorBamSerializer::fillin(NurbsSurfaceEvaluator &evaluator, DatagramIterator &di)
{
  //Read from DatagramIterator
  int uOrder = di.get_int32();
  evaluator.set_u_order(uOrder);
  int numberOfUKnots = di.get_int32();
  for(int i = 0; i != numberOfUKnots; i++) {evaluator.set_u_knot(i, di.get_BeFloat32());}

  //-------------------------------------------------------------------------//

  int vOrder = di.get_int32();
  evaluator.set_v_order(vOrder);
  int numberOfVKnots = di.get_int32();
  for(int i = 0; i != numberOfVKnots; i++) {evaluator.set_v_knot(i, di.get_BeFloat32());}

  //=========================================================================//

  int numberOfUVertices = di.get_int32();
  int numberOfVVertices = di.get_int32();
  evaluator.reset(numberOfUVertices, numberOfVVertices);

  for(int i = 0; i != numberOfUVertices; i++)
  {
    for(int j = 0; j != numberOfVVertices; j++)
    {
      LVecBase4 position;
      position.set_x(di.get_BeFloat32());
      position.set_y(di.get_BeFloat32());
      position.set_z(di.get_BeFloat32());
      position.set_w(di.get_BeFloat32());

      evaluator.set_vertex(i, j, position);

      evaluator.set_vertex_space(i, j, di.get_node_path());

      int DataCount = di.get_int32();
      for(int k = 0; k != DataCount; k++) {evaluator.set_extended_vertex(i, j, k, di.get_BeFloat32());}
    }
  }
  return;
}

/**
* Writes an object to BAM; basically just the inverse of fillin. Follows the same order as the input.
*/
void NurbsSurfaceEvaluatorBamSerializer::write_datagram(const NurbsSurfaceEvaluator &evaluator, Datagram &datagram)
{
  datagram.add_int32(evaluator.get_u_order());
  datagram.add_int32(evaluator.get_num_u_knots());
  for(int i = 0; i != evaluator.get_num_u_knots(); i++) {datagram.add_float32(evaluator.get_u_knot(i));}

  //-------------------------------------------------------------------------//

  datagram.add_int32(evaluator.get_v_order());
  datagram.add_int32(evaluator.get_num_v_knots());
  for(int i = 0; i != evaluator.get_num_v_knots(); i++) {datagram.add_float32(evaluator.get_v_knot(i));}

  //=========================================================================//

  int numberOfUVertices = evaluator.get_num_u_vertices();
  int numberOfVVertices = evaluator.get_num_v_vertices();

  datagram.add_int32(numberOfUVertices);
  datagram.add_int32(numberOfVVertices);

  for(int i = 0; i != numberOfUVertices; i++)
  {
    for(int j = 0; j != numberOfVVertices; j++)
    {
      LVecBase4 position = evaluator.get_vertex(i, j);
      datagram.add_float32(position.get_x());
      datagram.add_float32(position.get_y());
      datagram.add_float32(position.get_z());
      datagram.add_float32(position.get_w());

      datagram.write(evaluator.get_vertex_space(i, j));

      const NurbsVertex &TheVertex = evaluator.vert(i,j);
      int DataCount = TheVertex.get_num_extended();
      datagram.add_int32(DataCount);

      for(int k = 0; k != DataCount; k++) {datagram.add_float32(evaluator.get_extended_vertex(i, j, k));}
    }
  }
  return;
}

/**
* Simply put, it reads an object from BAM.
*/
TypedWritable* NurbsSurfaceEvaluatorBamSerializer::make_from_bam(DatagramIterator &di)
{
  NurbsSurfaceEvaluator *evaluator = new NurbsSurfaceEvaluator();
  NurbsSurfaceEvaluatorBamSerializer::fillin(*evaluator, di); //no more resolution errors >:(
  return evaluator;
}

/**
* resolves the paths, specifically the NodePaths.
*/
void NurbsSurfaceEvaluatorBamSerializer::complete_pointers(NurbsSurfaceEvaluator &evaluator)
{
  int numberOfUVertices = evaluator.get_num_u_vertices();
  int numberOfVVertices = evaluator.get_num_v_vertices();

  for(int i = 0; i != numberOfUVertices; i++)
  {
    for(int j = 0; j != numberOfVVertices; j++)
    {
      evaluator.get_vertex_space(i,j).get_node()->complete_pointers();
    }
  }
}





