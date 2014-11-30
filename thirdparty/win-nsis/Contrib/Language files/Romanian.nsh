;Language: Romanian (1048)
;Translated by Cristian Pirvu (pcristip@yahoo.com)
;Updates by Sorin Sbarnea - INTERSOL SRL (sbarneasorin@intersol.ro) - ROBO Design (www.robodesign.ro)
;New revision by George Radu (georadu@hotmail.com) http://mediatae.3x.ro
;New revision by Vlad Rusu (vlad@bitattack.ro)
;	- Use Romanian letters ����
;	- ".. produsului" removed as unnecessary
;	- "Elimin�" related terms replaced with more appropiate "Dezinstaleaz�"
;	- Misc language tweaks
!insertmacro LANGFILE "Romanian" "Romana"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Bine a�i venit la instalarea $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Aceast� aplica�ie va instala $(^NameDA).$\r$\n$\r$\nEste recomandat s� �nchide�i toate aplica�iile �nainte de �nceperea procesului de instalare. Acest lucru v� poate asigura un proces de instalare f�r� erori sau situa�ii neprev�zute.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Bine a�i venit la dezinstalarea $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Aceast� aplica�ie va dezinstala $(^NameDA).$\r$\n$\r$\nEste recomandat s� �nchide�i toate aplica�iile �nainte de �nceperea procesului de dezinstalare. Acest lucru v� poate asigura un proces de dezinstalare f�r� erori sau situa�ii neprev�zute.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_WELCOMEPAGE | MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Terminare"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Contract de licen��"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Citi�i cu aten�ie termenii contractului de licen�� �nainte de a instala $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Dac� accepta�i termenii contractului de licen��, ap�sati De Acord. Pentru a instala $(^NameDA) trebuie s� accepta�i termenii din contractul de licen��."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Dac� accepta�i termenii contractului de licen��, bifa�i c�su�a de mai jos. Pentru a instala $(^NameDA) trebuie s� accepta�i termenii din contractul de licen��. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Dac� accepta�i termenii contractului de licen��, selecta�i prima op�iune de mai jos. Pentru a instala $(^NameDA) trebuie s� accepta�i termenii din contractul de licen��. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Contract de licen��"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Citi�i cu aten�ie termenii contractului de licen�� �nainte de a dezinstala $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Dac� accepta�i termenii contractului de licen��, ap�sati De Acord. Pentru a dezinstala $(^NameDA) trebuie s� accepta�i termenii din contractul de licen��."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Dac� accepta�i termenii contractului de licen��, bifa�i c�su�a de mai jos. Pentru a dezinstala $(^NameDA) trebuie s� accepta�i termenii din contractul de licen��. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Dac� accepta�i termenii contractului de licen��, selecta�i prima op�iune de mai jos. Pentru a dezinstala $(^NameDA) trebuie s� accepta�i termenii din contractul de licen��. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Ap�sa�i Page Down pentru a vizualiza restul contractului de licen��."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Selectare componente"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Selecta�i componentele $(^NameDA) pe care dori�i s� le instala�i."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Selectare componente"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Selecta�i componentele $(^NameDA) pe care dori�i s� le dezinstala�i."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Descriere"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "A�eza�i mouse-ul deasupra fiec�rei componente pentru a vizualiza descrierea acesteia."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "A�eza�i mouse-ul deasupra fiec�rei componente pentru a vizualiza descrierea acesteia."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Selectare director destina�ie"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Selecta�i directorul �n care dori�i s� instala�i $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Selectare director de dezinstalat"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Selecta�i directorul din care dori�i s� dezinstala�i $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "�n curs de instalare"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "V� rug�m s� a�tepta�i, $(^NameDA) se instaleaz�."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Instalare terminat�"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Instalarea s-a terminat cu succes."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalare anulat�"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Instalarea a fost anulat� de utilizator."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "�n curs de dezinstalare"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "V� rug�m s� a�tepta�i, $(^NameDA) se dezinstaleaz�."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Dezinstalare terminat�"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Dezinstalarea s-a terminat cu succes."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Dezinstalare anulat�"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Dezinstalarea fost anulat� de utilizator."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Terminare instalare $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) a fost instalat.$\r$\n$\r$\nAp�sa�i Terminare pentru a �ncheia instalarea."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Trebuie s� reporni�i calculatorul pentru a termina instalarea. Dori�i s�-l reporni�i acum?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Terminare dezinstalare $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) a fost dezinstalat.$\r$\n$\r$\nAp�sa�i Terminare pentru a �ncheia dezinstalarea."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Trebuie s� reporni�i calculatorul pentru a termina dezinstalarea. Dori�i s�-l reporni�i acum?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Reporne�te acum"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Repornesc eu mai t�rziu"
  ${LangFileString} MUI_TEXT_FINISH_RUN "Executare $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Afi�are fi�ier readme (cite�te-m�)."
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Selectare grup Meniul Start"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Selecta�i un grup in Meniul Start pentru a crea comenzi rapide pentru produs."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Selecta�i grupul din Meniul Start �n care vor fi create comenzi rapide pentru produs. Pute�i de asemenea s� crea�i un grup nou."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nu doresc comenzi rapide"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Dezinstalare $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Dezinstalare $(^NameDA) din calculatorul dumneavoastr�."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Sunte�i sigur(�) c� dori�i s� anula�i instalarea $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Sunte�i sigur(�) c� dori�i s� anula�i dezinstalarea $(^Name)?"
!endif
