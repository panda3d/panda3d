// Filename: sceneGraphReducer.I
// Created by:  drose (22May00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedTransitions::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::AccumulatedTransitions::
AccumulatedTransitions() {
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedTransitions::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::AccumulatedTransitions::
AccumulatedTransitions(const SceneGraphReducer::AccumulatedTransitions &copy) :
  _transform(copy._transform),
  _color(copy._color),
  _texture_matrix(copy._texture_matrix)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedTransitions::Copy Assignment
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::AccumulatedTransitions::
operator = (const SceneGraphReducer::AccumulatedTransitions &copy) {
  _transform = copy._transform;
  _color = copy._color;
  _texture_matrix = copy._texture_matrix;
}
