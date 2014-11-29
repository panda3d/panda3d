;Language: Norwegian nynorsk (2068)
;By Vebjoern Sture and H�vard Mork (www.firefox.no)

!insertmacro LANGFILE "NorwegianNynorsk" "Norwegian nynorsk"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Velkommen til $(^NameDA) innstallasjonsvegvisar"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Denne vegvisaren vil leie deg gjennom installeringa av $(^NameDA).$\n$\nDet er tilr�dd at du avsluttar alle andre program f�r du held fram. Dette vil la installeringsprogrammet oppdatera systemfiler utan at du m� starta datamaskinen p� nytt.$\n$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Velkommen til avinstallering av $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Denne vegvisaren vil leie deg gjennom avinstalleringen av $(^NameDA).$\n$\nF�r du fortsetter m� du forsikre deg om at $(^NameDA) ikkje er opent.$\n$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Lisensavtale"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Les gjennom lisensavtalen f�r du startar installeringa av $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Trykk p� �Godta� dersom du godtar betingelsane i avtala. Du m� godta avtala for � installere $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Trykk p� avkryssingsboksen nedanfor nedanfor dersom du godtar betingelsane i avtala. Du m� godta avtala for � installere $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Vel det f�rste alternativet nedanfor dersom du godtek vilk�ra i avtala. Du m� godta avtala for � installera $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Lisensavtale"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Les gjennom lisensavtalen f�r du startar avinstalleringa av $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Trykk p� �Godta� dersom du godtar betingelsane i avtala. Du m� godta avtala for � avinstallera $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Trykk p� avkryssingsboksen nedanfor nedanfor dersom du godtar betingelsane i avtala. Du m� godta avtala for � avinstallera $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Vel det f�rste alternativet nedanfor dersom du godtar betingelsane i avtala. Du m� godta avtala for � avinstallera $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Trykk Page Down-knappen for � sj� resten av lisensavtala."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Vel komponentar"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Vel kva delar av $(^NameDA) du ynskjer � installera."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Vel funksjonar"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Vel kva for funksjonar du vil avinstallera i $(^NameDA)."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Beskriving"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Beveg musa over komponentene for � sj� beskrivinga."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Beveg musa over komponentene for � sj� beskrivinga."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Vel installasjonsmappe"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Vel kva mappe du vil installera $(^NameDA) i."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Vel avinstalleringplassering"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Vel mappa du vil avinstallere $(^NameDA) fr�."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Installerer"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Vent mens $(^NameDA) blir installert."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Installeringa er fullf�rt"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Installeringa vart fullf�rt."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Installeringa vart avbroten"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Installeringa vart ikkje fullf�rt."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Avinstallerer"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Vent medan $(^NameDA) vert avinstallert."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Avinstallering ferdig"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Avinstallering ble utf�rt uten feil."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Avinstallering broten"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Avinstallering ble ikkje utf�rt riktig."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Installering fullf�rt"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) er installert og klar til bruk.$\n$\nTrykk p� �Fullf�r� for � avslutte installeringa."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Du m� starta datamaskinen p� nytt for � fullf�ra installeringa av $(^NameDA). Vil du starta p� nytt no?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Fullf�rer avinstalleringa av $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) er no avinstallert fr� datamaskina di.$\n$\nTrykk p� �Fullf�r� for � avslutta denne vegvisaren."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Datamaskinen m� starta p� nytt for � fullf�ra avinstalleringa av $(^NameDA). Vil du starta datamaskina p� nytt no?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Start p� nytt no"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Eg vil starta p� nytt seinare"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&K�yr $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Syn lesmeg"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Fullf�r"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Vel mappe p� startmenyen"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Vel kva mappe snarvegane til $(^NameDA) skal liggja i."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Vel mappa du vil oppretta snarvegane til programmet i. Du kan �g skriva inn eit nytt namn for � laga ei ny mappe."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Ikkje opprett snarvegar"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Avinstaller $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Slett $(^NameDA) fr� datamaskinen."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Er du viss p� at du vil avslutta installeringa av $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Er du viss p� at du vil avbryta avinstalleringa av $(^Name)?"
!endif
