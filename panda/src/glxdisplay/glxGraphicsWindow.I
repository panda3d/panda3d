// Filename: glxGraphicsWindow.I
// Created by:  mike (07Mar99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: add_event_mask 
//       Access: Protected
//  Description: Ensures we only update window events once
////////////////////////////////////////////////////////////////////
INLINE void glxGraphicsWindow::add_event_mask(long event_mask)
{
  if ((_event_mask & event_mask) != event_mask) {
    _event_mask |= event_mask;  
    _change_mask |= GLXWIN_EVENT;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: remove_event_mask   
//       Access: Protected
//  Description: Ensures we only update window events once
////////////////////////////////////////////////////////////////////
INLINE void glxGraphicsWindow::remove_event_mask(long event_mask)
{
  if (_event_mask & event_mask) {
    _event_mask &= ~event_mask;
    _change_mask |= GLXWIN_EVENT;
  }
}
