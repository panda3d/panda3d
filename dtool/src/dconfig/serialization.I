// Filename: serialization.I
// Created by:  cary (20Mar00)
//
////////////////////////////////////////////////////////////////////

INLINE int Length(char) {
   return 1;
}

INLINE int Length(char* x) {
   return strlen(x);
}
