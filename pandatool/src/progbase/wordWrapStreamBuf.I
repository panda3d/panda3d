// Filename: wordWrapStreamBuf.I
// Created by:  drose (01Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: WordWrapStreamBuf::set_literal_mode
//       Access: Private
//  Description: An internal function called to update the internal
//               state according to the current value of the
//               Notify::literal flag, which might or might not be set
//               of the ostream at any time.  When the literal flag is
//               true, we should not word-wrap, so toggling this flag
//               means we need to flush the current buffer.
////////////////////////////////////////////////////////////////////
INLINE void WordWrapStreamBuf::
set_literal_mode(bool mode) {
  if (mode != _literal_mode) {
    flush_data();
    _literal_mode = mode;
  }
}

