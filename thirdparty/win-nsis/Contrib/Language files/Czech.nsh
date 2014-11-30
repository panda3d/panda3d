;Language: Czech (1029)
;By SELiCE (ls@selice.cz - http://ls.selice.cz)
;Corrected by Ond�ej Vani� - http://www.vanis.cz/ondra

!insertmacro LANGFILE "Czech" "Cesky"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "V�tejte v pr�vodci instalace programu $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Tento pr�vodce V�s provede instalac� $(^NameDA).$\r$\n$\r$\nP�ed za��tkem instalace je doporu�eno zav��t v�echny ostatn� aplikace. Toto umo�n� aktualizovat d�le�it� syst�mov� soubory bez restartov�n� Va�eho po��ta�e.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "V�tejte v $(^NameDA) odinstala�n�m pr�vodci"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Tento pr�vodce V�s provede odinstalac� $(^NameDA).$\r$\n$\r$\nP�ed za��tkem odinstalace, se p�esv�d�te, �e $(^NameDA) nen� spu�t�n.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licen�n� ujedn�n�"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "P�ed instalac� programu $(^NameDA) si pros�m prostudujte licen�n� podm�nky."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Jestli�e souhlas�te se v�emi podm�nkami ujedn�n�, zvolte 'Souhlas�m' pro pokra�ov�n�. Pro instalaci programu $(^NameDA) je nutn� souhlasit s licen�n�m ujedn�n�m."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Jestli�e souhlas�te se v�emi podm�nkami ujedn�n�, za�krtn�te n�e uvedenou volbu. Pro instalaci programu $(^NameDA) je nutn� souhlasit s licen�n�m ujedn�n�m. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Jestli�e souhlas�te se v�emi podm�nkami ujedn�n�, zvolte prvn� z mo�nost� uveden�ch n�e. Pro instalaci programu $(^NameDA) je nutn� souhlasit s licen�n�m ujedn�n�m. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licen�n� ujedn�n�"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "P�ed odinstalov�n�m programu $(^NameDA) si pros�m prostudujte licen�n� podm�nky."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Jestli�e souhlas�te se v�emi podm�nkami ujedn�n�, zvolte 'Souhlas�m' pro pokra�ov�n�. Pro odinstalov�n� programu $(^NameDA) je nutn� souhlasit s licen�n�m ujedn�n�m."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Jestli�e souhlas�te se v�emi podm�nkami ujedn�n�, za�krtn�te n�e uvedenou volbu. Pro odinstalov�n� programu $(^NameDA) je nutn� souhlasit s licen�n�m ujedn�n�m. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Jestli�e souhlas�te se v�emi podm�nkami ujedn�n�, zvolte prvn� z n�e uveden�ch mo�nost�. Pro odinstalov�n� programu $(^NameDA) je nutn� souhlasit s licen�n�m ujedn�n�m. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Stisknut�m kl�vesy Page Down posunete text licen�n�ho ujedn�n�."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Volba sou��st�"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Zvolte sou��sti programu $(^NameDA), kter� chcete nainstalovat."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Volba sou��st�"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Zvolte sou��sti programu $(^NameDA), kter� chcete odinstalovat."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Popis"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "P�i pohybu my�� nad instal�torem programu se zobraz� jej� popis."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "P�i pohybu my�� nad instal�torem programu se zobraz� jej� popis."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Zvolte um�st�n� instalace"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Zvolte slo�ku, do kter� bude program $(^NameDA) nainstalov�n."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Zvolte um�st�n� odinstalace"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Zvolte slo�ku, ze kter� bude program $(^NameDA) odinstalov�n."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instalace"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Vy�kejte, pros�m, na dokon�en� instalace programu $(^NameDA)."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Instalace dokon�ena"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Instalace prob�hla v po��dku."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalace p�eru�ena"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Instalace nebyla dokon�ena."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Odinstalace"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Vy�kejte, pros�m, na dokon�en� odinstalace programu $(^NameDA)."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Odinstalace dokon�ena"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Odinstalace prob�hla v po��dku."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Odinstalace p�eru�ena"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Odinstalace nebyla dokon�ena."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Dokon�en� pr�vodce programu $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Program $(^NameDA) byl nainstalov�n na V� po��ta�.$\r$\n$\r$\nKlikn�te 'Dokon�it' pro ukon�en� pr�vodce."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Pro dokon�en� instalace programu $(^NameDA) je nutno restartovat po��ta�. Chcete restatovat nyn�?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Dokon�uji odinstala�n�ho pr�vodce $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) byl odinstalov�n z Va�eho po��ta�e.$\r$\n$\r$\nKlikn�te na 'Dokon�it' pro ukon�en� tohoto pr�vodce."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Pro dokon�en� odinstalace $(^NameDA) mus� b�t V� po��ta� restartov�n. Chcete restartovat nyn�?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Restartovat nyn�"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Restartovat ru�n� pozd�ji"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Spustit program $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Zobrazit �ti-mne"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Dokon�it"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Zvolte slo�ku v Nab�dce Start"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Zvolte slo�ku v Nab�dce Start pro z�stupce programu $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Zvolte slo�ku v Nab�dce Start, ve kter� chcete vytvo�it z�stupce programu. M��ete tak� zadat nov� jm�no pro vytvo�en� nov� slo�ky."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nevytv��et z�stupce"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Odinstalovat program $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Odebrat program $(^NameDA) z Va�eho po��ta�e."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Opravdu chcete ukon�it instalaci programu $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Skute�n� chcete ukon�it odinstalaci $(^Name)?"
!endif
