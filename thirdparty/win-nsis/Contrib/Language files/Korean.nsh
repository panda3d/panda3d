;Language: Korean (1042)
;By linak linak@korea.com ( ~ V2.0 BETA3 ) By kippler@gmail.com(www.kipple.pe.kr) ( V2.0 BETA3 ~ ) (last update:2007/09/05)

!insertmacro LANGFILE "Korean" "Korean"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "$(^NameDA) ��ġ�� �����մϴ�."
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "�� ���α׷��� ����� ��ǻ�Ϳ� $(^NameDA)(��)�� ��ġ�� ���Դϴ�.$\r$\n$\r$\n��ġ�� �����ϱ� �� ������ �� ��� ���α׷��� �����Ͽ� �ֽñ� �ٶ��ϴ�. �̴� ������� ���� �ʰ��� �ý��� ������ ������ �� �ְ� ���ݴϴ�.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "$(^NameDA) ���Ÿ� �����մϴ�."
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "�� ���α׷��� ����� ��ǻ�Ϳ��� $(^NameDA)(��)�� ������ ���Դϴ�.$\r$\n$\r$\n���Ÿ� �����ϱ� ���� $(^NameDA)(��)�� �����Ͽ� �ֽñ� �ٶ��ϴ�.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "���� ���"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "$(^NameDA)(��)�� ��ġ�Ͻñ� ���� ���� ��� ������ ���캸�ñ� �ٶ��ϴ�."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "���뿡 �����ϼ̴ٸ� '������'�� ���� �ּ���. $(^NameDA)(��)�� ��ġ�ϱ� ���ؼ��� �ݵ�� ���뿡 �����ϼž� �մϴ�."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "���뿡 �����ϼ̴ٸ� �Ʒ� ������ ������ �ּ���. $(^NameDA)(��)�� ��ġ�ϱ� ���ؼ��� �ݵ�� ���뿡 �����ϼž� �մϴ�. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "���뿡 �����ϼ̴ٸ� ù ��° ������ ������ �ּ���. $(^NameDA)(��)�� ��ġ�ϱ� ���ؼ��� �ݵ�� ���뿡 �����ϼž� �մϴ�. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "���� ��� ����"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "$(^NameDA)(��)�� �����Ͻñ� ���� ���� ��� ������ ���캸�ñ� �ٶ��ϴ�."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "���뿡 �����ϼ̴ٸ� '������'�� ���� �ּ���. $(^NameDA)(��)�� �����ϱ� ���ؼ��� �ݵ�� ���뿡 �����ϼž� �մϴ�."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "���뿡 �����ϼ̴ٸ� �Ʒ� ������ ������ �ּ���. $(^NameDA)(��)�� �����ϱ� ���ؼ��� �ݵ�� ���뿡 �����ϼž� �մϴ�. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "���뿡 �����ϼ̴ٸ� ù ��° ������ ������ �ּ���. $(^NameDA)(��)�� �����ϱ� ���ؼ��� �ݵ�� ���뿡 �����ϼž� �մϴ�. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "���� ��� ���� ������ ������ �κ��� ���÷��� [Page Down] Ű�� ���� �ּ���."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "���� ��� ����"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "��ġ�ϰ��� �ϴ� $(^NameDA)�� ���� ��Ҹ� ������ �ּ���."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "���� ��� ����"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "�����ϰ��� �ϴ� $(^NameDA)�� ���� ��Ҹ� ������ �ּ���."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "�� ����"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "�� ������ ���� ������ �κп� ���콺�� �÷���������."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "�� ������ ���� ������ �κп� ���콺�� �÷���������."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "��ġ ��ġ ����"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "$(^NameDA)(��)�� ��ġ�� ������ ������ �ּ���."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "���� ��ġ ����"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "$(^NameDA)(��)�� ������ ������ ������ �ּ���."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "��ġ��"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "$(^NameDA)(��)�� ��ġ�ϴ� ���� ��� ��ٷ� �ּ���."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "��ġ �Ϸ�"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "��ġ�� ���������� �Ϸ�Ǿ����ϴ�."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "��ġ ���"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "���α׷� ��ġ�� ��ҵǾ����ϴ�."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "������"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "$(^NameDA)(��)�� �����ϴ� ���� ��� ��ٷ� �ֽñ� �ٶ��ϴ�."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "���� ��ħ"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "���α׷��� ���������� �����Ͽ����ϴ�."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "���α׷� ���� ���"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "���α׷� ���Ű� ��ҵǾ����ϴ�."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "$(^NameDA) ��ġ �Ϸ�"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA)�� ��ġ�� �Ϸ�Ǿ����ϴ�. ��ġ ���α׷��� ��ġ���� '��ħ' ��ư�� ���� �ּ���."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "$(^NameDA)�� ��ġ�� �Ϸ��ϱ� ���ؼ��� �ý����� �ٽ� �����ؾ� �մϴ�. ���� ����� �Ͻðڽ��ϱ�?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "���� �Ϸ�"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA)�� ���Ű� �Ϸ� �Ǿ����ϴ�."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "$(^NameDA)�� ���Ÿ� �Ϸ��ϱ� ���ؼ��� �ý����� �ٽ� �����ؾ� �մϴ�. ���� ����� �Ͻðڽ��ϱ�?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "���� ����� �ϰڽ��ϴ�."
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "���߿� ����� �ϰڽ��ϴ�."
  ${LangFileString} MUI_TEXT_FINISH_RUN "$(^NameDA) �����ϱ�(&R)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Readme ���� ����(&S)"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "��ħ"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "���� �޴� ���� ����"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "���α׷��� �ٷ� ���� �������� ������ ���� �޴� ���� ����."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "���α׷��� �ٷ� ���� �������� ������ ���� �޴� ������ �����ϼ���. ���ο� ������ �����Ϸ��� ���� �̸��� �Է��ϼ���."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "�ٷ� ���� �������� ������ �ʰڽ��ϴ�."
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "$(^NameDA) ����"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "$(^NameDA) �����ϱ�"
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "$(^Name) ��ġ�� ����Ͻðڽ��ϱ�?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "$(^Name) ���Ÿ� ����Ͻðڽ��ϱ�?"
!endif
