#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET graph
  #define LOCAL_LIBS \
    putil mathutil

  #define SOURCES \    
	allAttributesWrapper.I	      nodeAttributes.T  \
	allAttributesWrapper.T	      nodeAttributes.cxx  \
	allAttributesWrapper.cxx      nodeAttributes.h  \
	allAttributesWrapper.h	      nodeRelation.I  \
	allTransitionsWrapper.I       nodeRelation.N  \
	allTransitionsWrapper.T       nodeRelation.T  \
	allTransitionsWrapper.cxx     nodeRelation.cxx  \
	allTransitionsWrapper.h       nodeRelation.h  \
	arcChain.I		      nodeTransition.I  \
	arcChain.cxx		      nodeTransition.N  \
	arcChain.h		      nodeTransition.cxx  \
	bitMask32Transition.cxx       nodeTransition.h  \
	bitMask32Transition.h	      nodeTransitionCache.I  \
	bitMaskAttribute.T	      nodeTransitionCache.cxx  \
	bitMaskAttribute.h	      nodeTransitionCache.h  \
	bitMaskTransition.T	      nodeTransitionCacheEntry.I  \
	bitMaskTransition.h	      nodeTransitionCacheEntry.cxx  \
	boundedObject.I		      nodeTransitionCacheEntry.h  \
	boundedObject.N		      nodeTransitionWrapper.I  \
	boundedObject.cxx	      nodeTransitionWrapper.T  \
	boundedObject.h		      nodeTransitionWrapper.cxx  \
	config_graph.cxx	      nodeTransitionWrapper.h  \
	config_graph.h		      nodeTransitions.I  \
	dftraverser.T		      nodeTransitions.T  \
	dftraverser.h		      nodeTransitions.cxx  \
	graphReducer.cxx	      nodeTransitions.h  \
	graphReducer.h		      nullAttributeWrapper.I  \
	immediateAttribute.cxx	      nullAttributeWrapper.cxx  \
	immediateAttribute.h	      nullAttributeWrapper.h  \
	immediateTransition.I	      nullLevelState.cxx  \
	immediateTransition.cxx       nullLevelState.h  \
	immediateTransition.h	      nullTransitionWrapper.I  \
	lmatrix4fTransition.cxx       nullTransitionWrapper.cxx  \
	lmatrix4fTransition.h	      nullTransitionWrapper.h  \
	matrixAttribute.T	      onAttribute.cxx  \
	matrixAttribute.h	      onAttribute.h  \
	matrixTransition.T	      onOffAttribute.I  \
	matrixTransition.h	      onOffAttribute.cxx  \
	multiAttribute.T	      onOffAttribute.h  \
	multiAttribute.h	      onOffTransition.I  \
	multiNodeAttribute.cxx	      onOffTransition.cxx  \
	multiNodeAttribute.h	      onOffTransition.h  \
	multiNodeTransition.cxx       onTransition.I  \
	multiNodeTransition.h	      onTransition.cxx  \
	multiTransition.I	      onTransition.h  \
	multiTransition.T	      pointerNameClass.h  \
	multiTransition.h	      pt_NamedNode.N  \
	multiTransitionHelpers.I      pt_NamedNode.cxx  \
	multiTransitionHelpers.h      pt_NamedNode.h  \
	namedNode.I		      pt_Node.N  \
	namedNode.cxx		      pt_Node.cxx  \
	namedNode.h		      pt_Node.h  \
	node.I			      setTransitionHelpers.T  \
	node.cxx		      setTransitionHelpers.h  \
	node.h			      \
	nodeAttribute.I		      \
	nodeAttribute.N		      \
	nodeAttribute.cxx	      transitionDirection.h  \
	nodeAttribute.h		      traverserVisitor.T  \
	nodeAttributeWrapper.I	      traverserVisitor.h  \
	nodeAttributeWrapper.T	      vector_PT_Node.cxx  \
	nodeAttributeWrapper.cxx      vector_PT_Node.h  \
	nodeAttributeWrapper.h	      wrt.I  \
	nodeAttributes.I	      wrt.cxx  \
	nodeAttributes.N	      wrt.h


  #define INSTALL_HEADERS \
	allAttributesWrapper.I	    nodeAttributeWrapper.T  \
	allAttributesWrapper.T	    nodeAttributeWrapper.h  \
	allAttributesWrapper.h	    nodeAttributes.I  \
	allTransitionsWrapper.I     nodeAttributes.T  \
	allTransitionsWrapper.T     nodeAttributes.h  \
	allTransitionsWrapper.h     nodeRelation.I  \
	arcChain.I		    nodeRelation.T  \
	arcChain.h		    nodeRelation.h  \
	bitMask32Transition.h	    nodeTransition.I  \
	bitMaskAttribute.T	    nodeTransition.h  \
	bitMaskAttribute.h	    nodeTransitionCache.I  \
	bitMaskTransition.T	    nodeTransitionCache.h  \
	bitMaskTransition.h	    nodeTransitionCacheEntry.I  \
	boundedObject.I		    nodeTransitionCacheEntry.h  \
	boundedObject.h		    nodeTransitionWrapper.I  \
	config_graph.h		    nodeTransitionWrapper.T  \
	dftraverser.T		    nodeTransitionWrapper.h  \
	dftraverser.h		    nodeTransitions.I  \
	graphReducer.h		    nodeTransitions.T  \
	immediateAttribute.h	    nodeTransitions.h  \
	immediateTransition.I	    nullAttributeWrapper.I  \
	immediateTransition.h	    nullAttributeWrapper.h  \
	lmatrix4fTransition.h	    nullLevelState.h  \
	matrixAttribute.T	    nullTransitionWrapper.I  \
	matrixAttribute.h	    nullTransitionWrapper.h  \
	matrixTransition.T	    onAttribute.h  \
	matrixTransition.h	    onOffAttribute.I  \
	multiAttribute.T	    onOffAttribute.h  \
	multiAttribute.h	    onOffTransition.I  \
	multiNodeAttribute.h	    onOffTransition.h  \
	multiNodeTransition.h	    onTransition.I  \
	multiTransition.I	    onTransition.h  \
	multiTransition.T	    pointerNameClass.h  \
	multiTransition.h	    pt_NamedNode.h  \
	multiTransitionHelpers.I    pt_Node.h  \
	multiTransitionHelpers.h    setTransitionHelpers.T  \
	namedNode.I		    setTransitionHelpers.h  \
	namedNode.h		    transitionDirection.h  \
	node.I			    traverserVisitor.T  \
	node.h			    traverserVisitor.h  \
	nodeAttribute.I		    vector_PT_Node.h  \
	nodeAttribute.h		    wrt.I  \
	nodeAttributeWrapper.I	    wrt.h
  
  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_graph
  #define LOCAL_LIBS \
    graph putil
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_graph.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_graphRead
  #define LOCAL_LIBS \
    putil graph
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_graphRead.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_graphWrite
  #define LOCAL_LIBS \
    graph putil
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_graphWrite.cxx

#end test_bin_target

