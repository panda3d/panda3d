;Language: Serbian (3098)
;Translation by ���� ������ <obucina@srpskijezik.edu.yu>

!insertmacro LANGFILE "Serbian" "Serbian Cyrillic"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "���������� � ����� �� ���������� �������� $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "����� ����� ���� ������ ���������� �������� $(^NameDA).$\r$\n$\r$\n������������ �� �� �������� ��� ����� �������� ��� ������� ����������. ��� ���� �������� ��������� ���������� ������ ��� ������� �� �������� ���������� ��������.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "���������� � ������������ �������� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "����� ����� ���� ������ ������������ �������� $(^NameDA).$\r$\n$\r$\n��� ������� ������������, ������� �� �� �� ������� $(^NameDA) �������. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "������� � ����� ��������"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "������ ��������� ������� � ����� �������� ��� ���������� �������� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "��� ���������� ��� ������ ��������, ���������� ����� ���������� �� ��������. ������ ���������� ������� �� ����� ����������� ������� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "��� ���������� ��� ������ ��������, ��������� �������� �����. ������ ���������� ������� �� ����� ����������� ������� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "��� ���������� ��� ������ ��������, ��������� ���� ����� �����. ������ ���������� ������� �� ����� ����������� ������� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "������� � ����� ��������"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "������ ��������� ������� � ����� �������� ��� ������������ �������� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "��� ���������� ��� ������ ��������, ���������� ����� ���������� �� ��������. ������ ���������� ������� �� ����� ������������� ������� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "��� ���������� ��� ������ ��������, ��������� �������� �����. ������ ���������� ������� �� ����� ������������� ������� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "��� ���������� ��� ������ ��������, ��������� ���� ����� �����. ������ ���������� ������� �� ����� ������������� ������� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "���������� Page Down �� ����� ������ ������� ��������."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "����� ���������� �� ����������"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "��������� ���������� �� ����������. ���������� �� ���� �������� ����������."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "����� ���������� �� ������������"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "��������� ���������� �� ������������. ������������ �� ���� �������� ����������."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "����"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "������ �������� ���� ����� ����� ���������� �� ����� ������ ��� ����."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "������ �������� ���� ����� ����� ���������� �� ����� ������ ��� ����."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "����� ������� �� ����������"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "��������� ������ � ��� ���� ����������� ������� $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "����� ������� �� ��������a���"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "��������� ������ �� ���� ���� ������������� ������� $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "����������"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "�������� ��� �� ������� $(^NameDA) ���������."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "�������� ����������"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "���������� �� ������� ��������."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "��������� ����������"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "���������� �� ��������� � ��� ������� ��������."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "������������"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "�������� ��� �� ������� $(^NameDA) �����������."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "�������� ������������"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "������������ �� ������� ��������."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "��������� ������������"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "������������ �� ��������� � ��� ������� ��������."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "�������� ���������� �������� $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "������� $(^NameDA) �� ���������� �� �������.$\r$\n$\r$\n���������� ����� ���༓ �� ��������� ���� �������."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "������� ���� ���� ������ �������� �� �� �� ������ ���������� �������� $(^NameDA) ������� �������. ������ �� �� ����� �� �������?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "�������� ������������ �������� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "������� $(^NameDA) �� ������������ �� ��������.$\r$\n$\r$\n���������� ����� ���༓ �� ��������� ���� �������."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "������� ���� ���� ������ �������� �� �� �� ������ ������������ �������� $(^NameDA) ������� �������. ������ �� �� �� ������� �����?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "����� ������ ������� �������"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "��� �������� ���������"
  ${LangFileString} MUI_TEXT_FINISH_RUN "������� ������� $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "������� ��������� ���"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "���"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "����� ������� � ����� �����"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "��������� ������ � ����� ����� � ���� ���� �������� �������."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "��������� ������ � ����� ����� � ���� ������ �� ���� �������� ������� ��������. ������ ������� � ��� �� �������� ����� �������."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "��� �������� �������"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "������������ �������� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "������������ �������� $(^NameDA) �� ��������."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "������� ������ �� ��������� ���������� �������� $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "������� ������ �� ��������� ������������ �������� $(^Name)?"
!endif
