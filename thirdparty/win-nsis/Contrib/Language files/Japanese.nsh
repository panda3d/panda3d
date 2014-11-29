;Language: Japanese (1041)
;By Dnanako
;Translation updated by Takahiro Yoshimura <takahiro_y@monolithworks.co.jp>

!insertmacro LANGFILE "Japanese" "Japanese"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "$(^NameDA) �Z�b�g�A�b�v �E�B�U�[�h�ւ悤����"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "���̃E�B�U�[�h�́A$(^NameDA)�̃C���X�g�[�����K�C�h���Ă����܂��B$\r$\n$\r$\n�Z�b�g�A�b�v���J�n����O�ɁA���̂��ׂẴA�v���P�[�V�������I�����邱�Ƃ𐄏����܂��B����ɂ���ăZ�b�g�A�b�v���R���s���[�^���ċN�������ɁA�V�X�e�� �t�@�C�����X�V���邱�Ƃ��o����悤�ɂȂ�܂��B$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "$(^NameDA) �A���C���X�g�[�� �E�B�U�[�h�ւ悤����"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "���̃E�B�U�[�h�́A$(^NameDA)�̃A���C���X�g�[�����K�C�h���Ă����܂��B$\r$\n$\r$\n�A���C���X�g�[�����J�n����O�ɁA$(^NameDA)���N�����Ă��Ȃ����Ƃ��m�F���ĉ������B$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "���C�Z���X�_��"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "$(^NameDA)���C���X�g�[������O�ɁA���C�Z���X�������m�F���Ă��������B"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "�_�񏑂̂��ׂĂ̏����ɓ��ӂ���Ȃ�΁A[���ӂ���] ��I��ŃC���X�g�[���𑱂��Ă��������B$(^NameDA) ���C���X�g�[������ɂ́A�_�񏑂ɓ��ӂ���K�v������܂��B"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "�_�񏑂̂��ׂĂ̏����ɓ��ӂ���Ȃ�΁A���̃`�F�b�N�{�b�N�X���N���b�N���Ă��������B$(^NameDA) ���C���X�g�[������ɂ́A�_�񏑂ɓ��ӂ���K�v������܂��B $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "�_�񏑂̂��ׂĂ̏����ɓ��ӂ���Ȃ�΁A���ɕ\������Ă���I�v�V�����̂����A�ŏ��̂��̂�I��ŉ������B$(^NameDA) ���C���X�g�[������ɂ́A�_�񏑂ɓ��ӂ���K�v������܂��B $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "���C�Z���X�_��"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "$(^NameDA)���A���C���X�g�[������O�ɁA���C�Z���X�������m�F���Ă��������B"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "�_�񏑂̂��ׂĂ̏����ɓ��ӂ���Ȃ�΁A[���ӂ���] ��I��ŃA���C���X�g�[���𑱂��Ă��������B$(^NameDA) ���A���C���X�g�[������ɂ́A�_�񏑂ɓ��ӂ���K�v������܂��B"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "�_�񏑂̂��ׂĂ̏����ɓ��ӂ���Ȃ�΁A���̃`�F�b�N�{�b�N�X���N���b�N���Ă��������B$(^NameDA) ���A���C���X�g�[������ɂ́A�_�񏑂ɓ��ӂ���K�v������܂��B $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "�_�񏑂̂��ׂĂ̏����ɓ��ӂ���Ȃ�΁A���ɕ\������Ă���I�v�V�����̂����A�ŏ��̂��̂�I��ŉ������B$(^NameDA) ���A���C���X�g�[������ɂ́A�_�񏑂ɓ��ӂ���K�v������܂��B $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "[Page Down]�������Č_�񏑂����ׂĂ��ǂ݂��������B"
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "�R���|�[�l���g��I��ł��������B"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "$(^NameDA)�̃C���X�g�[�� �I�v�V������I��ł��������B"
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "�R���|�[�l���g��I��ł��������B"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "$(^NameDA)�̃A���C���X�g�[�� �I�v�V������I��ł��������B"
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "����"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "�R���|�[�l���g�̏�Ƀ}�E�X �J�[�\�����ړ�����ƁA�����ɐ������\������܂��B"
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "�R���|�[�l���g�̏�Ƀ}�E�X �J�[�\�����ړ�����ƁA�����ɐ������\������܂��B"
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "�C���X�g�[�����I��ł��������B"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "$(^NameDA)���C���X�g�[������t�H���_��I��ł��������B"
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "�A���C���X�g�[������I��ł��������B"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "$(^NameDA)���A���C���X�g�[������t�H���_��I��ł��������B"
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "�C���X�g�[��"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "$(^NameDA)���C���X�g�[�����Ă��܂��B���΂炭���҂����������B"
  ${LangFileString} MUI_TEXT_FINISH_TITLE "�C���X�g�[���̊���"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "�C���X�g�[���ɐ������܂����B"
  ${LangFileString} MUI_TEXT_ABORT_TITLE "�C���X�g�[���̒��~"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "�Z�b�g�A�b�v�͐���Ɋ�������܂���ł����B"
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "�A���C���X�g�[��"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "$(^NameDA)���A���C���X�g�[�����Ă��܂��B���΂炭���҂����������B"
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "�A���C���X�g�[���̊���"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "�A���C���X�g�[���ɐ������܂����B"
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "�A���C���X�g�[���̒��~"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "�A���C���X�g�[���͐���Ɋ�������܂���ł����B"
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "$(^NameDA) �Z�b�g�A�b�v �E�B�U�[�h�͊������܂����B"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA)�́A���̃R���s���[�^�ɃC���X�g�[������܂����B$\r$\n$\r$\n�E�B�U�[�h�����ɂ� [����] �������Ă��������B"
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "$(^NameDA) �̃C���X�g�[������������ɂ́A���̃R���s���[�^���ċN������K�v������܂��B�������ċN�����܂����H"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "$(^NameDA) �A���C���X�g�[�� �E�B�U�[�h�͊������܂����B"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA)�́A���̃R���s���[�^����A���C���X�g�[������܂����B$\r$\n$\r$\n�E�B�U�[�h�����ɂ� [����] �������Ă��������B"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "$(^NameDA) �̃A���C���X�g�[������������ɂ́A���̃R���s���[�^���ċN������K�v������܂��B�������ċN�����܂����H"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "�������ċN������"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "��Ŏ蓮�ōċN������"
  ${LangFileString} MUI_TEXT_FINISH_RUN "$(^NameDA)�����s(&R)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Readme ��\������(&S)"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "����(&F)"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "�X�^�[�g���j���[ �t�H���_��I��ł��������B"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "$(^NameDA)�̃V���[�g�J�b�g���쐬����X�^�[�g���j���[ �t�H���_��I��ŉ������B"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "���̃v���O�����̃V���[�g�J�b�g���쐬�������X�^�[�g���j���[ �t�H���_��I�����Ă��������B�܂��A�쐬����V�����t�H���_�ɖ��O�����邱�Ƃ��ł��܂��B"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "�V���[�g�J�b�g���쐬���Ȃ�"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "$(^NameDA)�̃A���C���X�g�[��"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "$(^NameDA)�����̃R���s���[�^����폜���܂��B"
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "$(^Name) �Z�b�g�A�b�v�𒆎~���܂����H"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "$(^Name) �A���C���X�g�[���𒆎~���܂����H"
!endif
