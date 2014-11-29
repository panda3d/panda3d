;Language: 'Chinese (Traditional)' (1028)
;Translator: Kii Ali <kiiali@cpatch.org>
;Revision date: 2004-12-15

!insertmacro LANGFILE "TradChinese" "Chinese (Traditional)"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "�w��ϥ� $(^NameDA) �w�˺��F"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "�o�Ӻ��F�N���ާA���� $(^NameDA) ���w�˶i�{�C$\r$\n$\r$\n�b�}�l�w�ˤ��e�A��ĳ��������L�Ҧ����ε{���C�o�N���\\�u�w�˵{���v��s���w���t���ɮסA�Ӥ��ݭn���s�ҰʧA���q���C$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "�w��ϥ� $(^NameDA) �Ѱ��w�˺��F"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "�o�Ӻ��F�N���{���ާA $(^NameDA) ���Ѱ��w�˶i�{�C$\r$\n$\r$\n�b�}�l�Ѱ��w�ˤ��e�A�T�{ $(^NameDA) �å�������C$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "���v��ĳ"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "�b�w�� $(^NameDA) ���e�A���˾\���v���ڡC"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "�p�G�A������ĳ�������ڡA���@�U [�ڦP�N(I)] �~��w�ˡC�p�G�A��� [����(C)] �A�w�˵{���N�|�����C�����n������ĳ�~��w�� $(^NameDA) �C"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "�p�G�A������ĳ�������ڡA���@�U�U�誺�Ŀ�ءC�����n������ĳ�~��w�� $(^NameDA)�C$_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "�p�G�A������ĳ�������ڡA��ܤU��Ĥ@�ӿﶵ�C�����n������ĳ�~��w�� $(^NameDA)�C$_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "���v��ĳ"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "�b�Ѱ��w�� $(^NameDA) ���e�A���˾\���v���ڡC"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "�p�G�A������ĳ�������ڡA���@�U [�ڦP�N(I)] �~��Ѱ��w�ˡC�p�G�A��� [����(C)] �A�w�˵{���N�|�����C�����n������ĳ�~��Ѱ��w�� $(^NameDA) �C"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "�p�G�A������ĳ�������ڡA���@�U�U�誺�Ŀ�ءC�����n������ĳ�~��Ѱ��w�� $(^NameDA)�C$_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "�p�G�A������ĳ�������ڡA��ܤU��Ĥ@�ӿﶵ�C�����n������ĳ�~��Ѱ��w�� $(^NameDA)�C$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "�˾\��ĳ����l�����A�Ы� [PgDn] ���U���ʭ����C"
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "��ܤ���"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "��ܧA�Q�n�w�� $(^NameDA) �����ǥ\��C"
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "�������"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "��� $(^NameDA) ���A�Q�n�Ѱ��w�˪��\��C"
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "�y�z"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "���ʧA���ƹ����Ш줸�󤧤W�A�K�i���쥦���y�z�C"
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "���ʧA���ƹ����Ш줸�󤧤W�A�K�i���쥦���y�z�C"
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "����w�˦�m"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "��� $(^NameDA) �n�w�˪���Ƨ��C"
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "����Ѱ��w�˦�m"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "��� $(^NameDA) �n�Ѱ��w�˪���Ƨ��C"
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "���b�w��"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "$(^NameDA) ���b�w�ˡA�е��ԡC"
  ${LangFileString} MUI_TEXT_FINISH_TITLE "�w�˧���"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "�w�˵{���w���\�a���槹���C"
  ${LangFileString} MUI_TEXT_ABORT_TITLE "�w�ˤv����"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "�w�˵{���å����\�a���槹���C"
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "���b�Ѱ��w��"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "$(^NameDA) ���b�Ѱ��w�ˡA�е��ԡC"
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "�Ѱ��w�ˤw����"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "�Ѱ��w�˵{���w���\�a���槹���C"
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "�Ѱ��w�ˤw����"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "�Ѱ��w�˵{���å����\�a���槹���C"
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "���b���� $(^NameDA) �w�˺��F"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) �w�w�˦b�A���t�ΡC$\r$\n���@�U [����(F)] ���������F�C"
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "�A���t�λݭn���s�ҰʡA�H�K���� $(^NameDA) ���w�ˡC�{�b�n���s�ҰʶܡH"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "���b���� $(^NameDA) �Ѱ��w�˺��F"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) �w�q�A���q���Ѱ��w�ˡC$\r$\n$\r$\n���@�U [����] �����o�Ӻ��F�C"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "�q���ݭn���s�ҰʡA�H�K���� $(^NameDA) ���Ѱ��w�ˡC�{�b�Q�n���s�ҰʶܡH"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "�O�A�{�b���s�Ұ�(&Y)"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "�_�A�ڵy��A�ۦ歫�s�Ұ�(&N)"
  ${LangFileString} MUI_TEXT_FINISH_RUN "���� $(^NameDA)(&R)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "��ܡuŪ���ɮסv(&M)"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "����(&F)"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "��ܡu�}�l�\���v��Ƨ�"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "��ܡu�}�l�\���v��Ƨ��A�Ω�{�������|�C"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "��ܡu�}�l�\���v��Ƨ��A�H�K�إߵ{�������|�C�A�]�i�H��J�W�١A�إ߷s��Ƨ��C"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "���n�إ߱��|(&N)"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "�Ѱ��w�� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "�q�A���q���Ѱ��w�� $(^NameDA) �C"
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "�A�T�w�n���} $(^Name) �w�˵{���H"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "�A�T�w�n���} $(^Name) �Ѱ��w�˶ܡH"
!endif
