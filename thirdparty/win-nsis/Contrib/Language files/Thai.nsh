;Language: Thai (1054)
;By SoKoOLz, TuW@nNu (asdfuae)

!insertmacro LANGFILE "Thai" "Thai"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "�Թ�յ�͹�Ѻ�������õԴ�������� $(^NameDA) "
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "��ǵԴ����ѵ���ѵԨйӤس�����õԴ��駢ͧ $(^NameDA).$\r$\n$\r$\n��Ң��й����Դ�����������������͹����������Դ���, �����繡���ѻഷ�������¢���¤س�����繵�ͧ�ӡ���պٷ����������ͧ�س$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "�Թ�յ�͹�Ѻ�����¡��ԡ��õԴ����ѵ���ѵԢͧ $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "��ǵԴ����ѵ���ѵԹ��йӤس������¡��ԡ��õԴ��駢ͧ $(^NameDA).$\r$\n$\r$\n��è���������¡��ԡ��õԴ��駹��, �ô��Ǩ�ͺ��� $(^NameDA) �����������$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "��͵�ŧ����ͧ�Ԣ�Է���"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "�ô��ҹ�ǹ�Ԣ�Է������Ǣ�͵�ҧ��ա���駡�͹���س�зӡ�õԴ��� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "��Ҥس����Ѻ��͵�ŧ����ͧ�Ԣ�Է���, �� �ѹ����Ѻ ���ͷӵ���, �س��ͧ����Ѻ㹢�͵�ŧ�Ԣ�Է������ͷ��зӡ�õԴ��� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "��Ҥس����Ѻ��͵�ŧ����ͧ�Ԣ�Է�, �����͡㹡��ͧ��ҧ��ҧ���  �س��ͧ����Ѻ㹢�͵�ŧ�Ԣ�Է������ͷ��зӡ�õԴ��� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "��Ҥس����Ѻ��͵�ŧ����ͧ�Ԣ�Է�,  ���͡������͡�á��ҹ��ҧ��� �س��ͧ����Ѻ㹢�͵�ŧ�Ԣ�Է������ͷ��зӡ�õԴ��� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "��͵�ŧ����ͧ�Ԣ�Է���"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "��س���ҹ��͵�ŧ��ҹ�Ԣ�Է����͹�Դ�������� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "��Ҥس����Ѻ㹢�͵�ŧ��� ��سҡ����� �ѹ����Ѻ ��Фس�е�ͧ��ŧ��͹������������¡��ԡ�Դ�������� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "��Ҥس����Ѻ��͵�ŧ����ͧ�Ԣ�Է���, �����͡㹡��ͧ��ҧ��ҧ��� �س��ͧ����Ѻ㹢�͵�ŧ�Ԣ�Է������ͷ��зӡ�õԴ��� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "��Ҥس����Ѻ��͵�ŧ����ͧ�Ԣ�Է���, ���͡������͡�á��ҹ��ҧ��� �س��ͧ����Ѻ㹢�͵�ŧ�Ԣ�Է������ͷ��зӡ�õԴ��� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "�� Page Down ������ҹ��͵�ŧ������"
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "���͡��ǹ��Сͺ"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "���͡��觷��س��ͧ�����ҹ�ҡ $(^NameDA) ���س��ͧ��õԴ���"
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "���͡��ǹ��Сͺ"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "���͡��觷��س��ͧ�����ҹ�ҡ $(^NameDA) ���س��ͧ¡��ԡ��õԴ���"
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "��������´"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "��Ѻ�����ͧ�س�˹����ǹ��Сͺ���ʹ���������´"
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "��Ѻ�����ͧ�س�˹����ǹ��Сͺ���ʹ���������´"
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "���͡������ͧ��õԴ���"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "���͡�������ͧ��õԴ��� $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "���͡�������ͧ���¡��ԡ��õԴ���"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "���͡������س��ͧ���¡��ԡ��õԴ��駢ͧ $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "���ѧ�Դ���"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "�ô��㹢�з�� $(^NameDA) ���ѧ�١�Դ���"
  ${LangFileString} MUI_TEXT_FINISH_TITLE "��õԴ����������"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "��õԴ�����������ó�"
  ${LangFileString} MUI_TEXT_ABORT_TITLE "��õԴ��駶١¡��ԡ"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "��õԴ��������������ó�"
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "���ѧ¡��ԡ��õԴ���"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "�ô��㹢�з�� $(^NameDA) ���ѧ�١¡��ԡ��õԴ���."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "���¡��ԡ��õԴ����������"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "���¡��ԡ��õԴ����������������ó�"
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "���¡��ԡ��õԴ��駶١¡��ԡ"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "���¡��ԡ��õԴ�����������"
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "��õԴ����ѵ���ѵԢͧ  $(^NameDA) ���ѧ�������"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) ��١�Դ���ŧ�����ͧ����������ͧ�س����$\r$\n$\r$\n�� ����������ͻԴ��ǵԴ����ѵ���ѵ�"
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "����ͧ����������ͧ�س���繵�ͧ��ʵ�÷����͡�õԴ��駢ͧ $(^NameDA) �����º����, �س��ͧ��è� �պٷ ����ǹ�����?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "���¡��ԡ��õԴ����ѵ���ѵԢͧ $(^NameDA) ���ѧ��������ó�"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) ��١¡��ԡ�͡�ҡ����ͧ����������ͧ�س���� $\r$\n$\r$\n�� ������� ���ͻԴ˹�Ҩ͵Դ����ѵ���ѵ�"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "����ͧ����������ͧ�س���繵�ͧ��ʵ���㹡�÷��зӡ��¡��ԡ��õԴ��駢ͧ $(^NameDA) �������, �س��ͧ��è��պٷ����ǹ�����?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "�պٷ ����ǹ��"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "�ѹ��ͧ��� �պٷ���µ��ͧ ����ѧ"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&�ѹ $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&�ʴ���������´"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&�������"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "���͡��� Start Menu"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "���͡��� Start Menu �������ҧ���쵤ѷ�ͧ $(^NameDA). "
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "���͡��� Start Menu ���س��ͧ��è����ҧ���쵤ѷ�ͧ�����, �س�ѧ����ö��˹������������ҧ����������ա����"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "����ͧ���ҧ ���쵤ѷ"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "¡��ԡ��õԴ��� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "¡��ԡ��õԴ��� $(^NameDA) �ҡ����ͧ����������ͧ�س"
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "�س���������Ҥس��ͧ��è��͡�ҡ��õԴ��駢ͧ $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "�س���������Ҥس��ͧ����͡�ҡ���¡��ԡ��õԴ��駢ͧ $(^Name)?"
!endif
