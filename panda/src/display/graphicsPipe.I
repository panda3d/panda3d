// Filename: graphicsPipe.I
// Created by:  frang (07Mar99)
// 
////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_all_pipes
//       Access: Private, Static
//  Description: Returns a reference to the static _all_pipes
//               container.  This wrapper function allows us to
//               guarantee creation of the container before we attempt
//               to reference it.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe::Pipes &GraphicsPipe::
get_all_pipes() {
  if (_all_pipes == NULL) {
    _all_pipes = new Pipes;
  }
  return *_all_pipes;
}
