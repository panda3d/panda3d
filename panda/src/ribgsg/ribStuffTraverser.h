// Filename: ribStuffTraverser.h
// Created by:  drose (16Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef RIBSTUFFTRAVERSER_H
#define RIBSTUFFTRAVERSER_H

#include <pandabase.h>

#include <traverserVisitor.h>
#include <renderRelation.h>
#include <allAttributesWrapper.h>
#include <allTransitionsWrapper.h>
#include <nullLevelState.h>

class RIBGraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : RibStuffTraverser
// Description :
////////////////////////////////////////////////////////////////////
class RibStuffTraverser : 
  public TraverserVisitor<AllTransitionsWrapper, NullLevelState> {
public:
  RibStuffTraverser(RIBGraphicsStateGuardian *gsg) : _gsg(gsg) { }
  bool reached_node(Node *node, AllAttributesWrapper &state,
		    NullLevelState &);

public:
  RIBGraphicsStateGuardian *_gsg;
};

#endif

