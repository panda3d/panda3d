; Syntax Highlighting Test File for NSIS
; Comments are Like this
# Comments can also be like this

; Function and String
!define PRODUCT_NAME "HelloWorld"

; Function and Label
SetCompressor lzma

; Section Definition
Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite try
  
SectionEnd

; Section, Function, Variable, String
Section -Post
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
SectionEnd

; Function Definition
Function LaunchHello
  Exec '"$INSTDIR\hello.exe"'
FunctionEnd
