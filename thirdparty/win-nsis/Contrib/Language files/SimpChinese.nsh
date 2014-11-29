;Language: 'Chinese (Simplified)' (2052)
;Translator: Kii Ali <kiiali@cpatch.org>
;Revision date: 2004-12-15
;Verified by: QFox <qfox99@gmail.com>

!insertmacro LANGFILE "SimpChinese" "Chinese (Simplified)"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "��ӭʹ�á�$(^NameDA)����װ��"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "����򵼽�ָ������ɡ�$(^NameDA)���İ�װ���̡�$\r$\n$\r$\n�ڿ�ʼ��װ֮ǰ�������ȹر���������Ӧ�ó����⽫������װ���򡱸���ָ����ϵͳ�ļ���������Ҫ����������ļ������$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "��ӭʹ�á�$(^NameDA)��ж����"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "����򵼽�ȫ��ָ���㡰$(^NameDA)����ж�ؽ��̡�$\r$\n$\r$\n�ڿ�ʼж��֮ǰ��ȷ�ϡ�$(^NameDA)����δ���е��С�$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "���֤Э��"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "�ڰ�װ��$(^NameDA)��֮ǰ�����Ķ���ȨЭ�顣"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "��������Э���е�������� [�ҽ���(I)] ������װ�������ѡ�� [ȡ��(C)] ����װ���򽫻�رա��������Э����ܰ�װ��$(^NameDA)����"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "��������Э���е���������·��Ĺ�ѡ�򡣱���Ҫ����Э����ܰ�װ $(^NameDA)��$_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "��������Э���е����ѡ���·���һ��ѡ�����Ҫ����Э����ܰ�װ $(^NameDA)��$_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "���֤Э��"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "��ж�ء�$(^NameDA)��֮ǰ���������Ȩ���"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "��������Э���е�������� [�ҽ���(I)] ����ж�ء������ѡ�� [ȡ��(C)] ����װ���򽫻�رա�����Ҫ����Э�����ж�ء�$(^NameDA)����"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "��������Э���е���������·��Ĺ�ѡ�򡣱���Ҫ����Э�����ж�� $(^NameDA)��$_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "��������Э���е����ѡ���·���һ��ѡ�����Ҫ����Э�����ж�� $(^NameDA)��$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "�� [PgDn] �Ķ�����ȨЭ�顱�����ಿ�֡�"
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "ѡ�����"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "ѡ������Ҫ��װ��$(^NameDA)������Щ���ܡ�"
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "ѡ�����"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "ѡ����$(^NameDA)����������Ҫж�صĹ��ܡ�"
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "����"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "�ƶ�������ָ�뵽���֮�ϣ���ɼ�������������"
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "�ƶ�������ָ�뵽���֮�ϣ���ɼ�������������"
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "ѡ��װλ��"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "ѡ��$(^NameDA)���İ�װ�ļ��С�"
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "ѡ��ж��λ��"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "ѡ����$(^NameDA)��Ҫж�ص��ļ��С�"
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "���ڰ�װ"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "��$(^NameDA)�����ڰ�װ����Ⱥ�..."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "��װ���"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "��װ�ѳɹ���ɡ�"
  ${LangFileString} MUI_TEXT_ABORT_TITLE "��װ����ֹ"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "��װû�гɹ���"
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "����ж��"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "��$(^NameDA)������ж�أ���Ⱥ�..."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "ж�������"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "ж���ѳɹ�����ɡ�"
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "ж������ֹ"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "ж�س���δ�ɹ�����ɡ�"
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "������ɡ�$(^NameDA)����װ��"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "��$(^NameDA)���Ѱ�װ�����ϵͳ��$\r$\n���� [���(F)] �رմ��򵼡�"
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "���ϵͳ��Ҫ�����������Ա���ɡ�$(^NameDA)���İ�װ������Ҫ����������"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "������ɡ�$(^NameDA)��ж����"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "��$(^NameDA)���Ѵ���ļ����ж�ء�$\r$\n$\r$\n���� [���] �ر�����򵼡�"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "�������Ҫ�����������Ա���ɡ�$(^NameDA)����ж�ء�������Ҫ����������"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "�ǣ�������������(&Y)"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "�����Ժ���������������(&N)"
  ${LangFileString} MUI_TEXT_FINISH_RUN "���� $(^NameDA)(&R)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "��ʾ�������ļ���(&M)"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "���(&F)"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "ѡ�񡰿�ʼ�˵����ļ���"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "ѡ�񡰿�ʼ�˵����ļ��У����ڳ���Ŀ�ݷ�ʽ��"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "ѡ�񡰿�ʼ�˵����ļ��У��Ա㴴������Ŀ�ݷ�ʽ����Ҳ�����������ƣ��������ļ��С�"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "��Ҫ������ݷ�ʽ(&N)"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "ж�� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "����ļ����ж�ء�$(^NameDA)��"
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "��ȷʵҪ�˳���$(^Name)����װ����"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "��ȷʵҪ�˳���$(^Name)��ж����"
!endif
