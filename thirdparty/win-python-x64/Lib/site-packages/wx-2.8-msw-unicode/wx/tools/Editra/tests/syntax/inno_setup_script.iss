; Syntax Highlighting Test File for Inno Setup Scripts
; Some comments about this file

; Preprocessor 
#pragma verboselevel 9
#define Debug

; Sections
[Setup]
AppName=My Program
AppVerName=My Program version 1.5
DefaultDirName={code:MyConst}\My Program
DefaultGroupName=My Program
UninstallDisplayIcon={app}\MyProg.exe
InfoBeforeFile=Readme.txt
OutputDir=userdocs:Inno Setup Examples Output

[Files]
Source: "MyProg.exe"; DestDir: "{app}"; Check: MyProgCheck; BeforeInstall: BeforeMyProgInstall('MyProg.exe'); AfterInstall: AfterMyProgInstall('MyProg.exe')
Source: "MyProg.chm"; DestDir: "{app}"; Check: MyProgCheck; BeforeInstall: BeforeMyProgInstall('MyProg.chm'); AfterInstall: AfterMyProgInstall('MyProg.chm')
Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme

[Code]
var
  MyProgChecked: Boolean;
  MyProgCheckResult: Boolean;
  FinishedInstall: Boolean;

function InitializeSetup(): Boolean;
begin
  Result := MsgBox('InitializeSetup:' #13#13 'Setup is initializing. Do you really want to start setup?', mbConfirmation, MB_YESNO) = idYes;
  if Result = False then
    MsgBox('InitializeSetup:' #13#13 'Ok, bye bye.', mbInformation, MB_OK);
end;
