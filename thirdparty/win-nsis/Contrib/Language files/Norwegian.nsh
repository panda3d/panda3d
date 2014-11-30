;Language: Norwegian (2068)
;By Jonas Lindsr�m (jonasc_88@hotmail.com) Reviewed and fixed by Jan Ivar Beddari, d0der at online.no

!insertmacro LANGFILE "Norwegian" "Norwegian"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Velkommen til veiviseren for installasjon av $(^NameDA) "
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Denne veiviseren vil lede deg gjennom installasjonen av $(^NameDA).$\r$\n$\r$\nDet anbefales at du avslutter alle andre programmer f�r du fortsetter. Dette vil la installasjonsprogrammet forandre p� systemfiler uten at du m� starte datamaskinen p� nytt.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Velkommen til veiviseren for avinstallasjon av $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Denne veiviseren vil lede deg gjennom avinstallasjonen av $(^NameDA).$\r$\n$\r$\nF�r du fortsetter m� du forsikre deg om at $(^NameDA) ikke kj�rer.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Lisensavtale"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Vennligst les gjennom lisensavtalen f�r du starter installasjonen av $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Hvis du godtar lisensavtalen trykk Godta for � fortsette. Du m� godta lisensavtalen for � installere $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Hvis du godtar lisensavtalen, kryss av p� merket under. Du m� godta lisensavtalen for � installere $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Hvis du godtar lisensavtalen, velg det f�rste alternativet ovenfor. Du m� godta lisensavtalen for � installere $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Lisensavtale"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Vennligst les gjennom lisensavtalen f�r du avinstallerer $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Hvis du godtar lisensavtalen trykk Godta for � fortsette.  Du m� godta lisensavtalen for � avintallere $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Hvis du godtar lisensavtalen, kryss av p� merket under. Du m� godta lisensavtalen for � avinstallere $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Hvis du godtar lisensavtalen, velg det f�rste alternativet ovenfor. Du m� godta lisensavtalen for � avinstallere $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Trykk Page Down knappen for � se resten av lisensavtalen."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Velg komponenter"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Velg hvilke deler av $(^NameDA) du �nsker � installere."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Velg komponenter"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Velg hvilke deler av $(^NameDA) du �nsker � avinstallere."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Beskrivelse"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Beveg musen over komponentene for � se beskrivelsen."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Beveg musen over komponentene for � se beskrivelsen."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Velg installasjonsmappe"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Velg hvilken mappe du vil installere $(^NameDA) i."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Velg mappe for avinstallasjon"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Velg mappen du vil avinstallere $(^NameDA) fra."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Installasjonen p�g�r"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Vennligst vent mens $(^NameDA) blir installert."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Installasjonen er ferdig"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Installasjonen ble fullf�rt uten feil."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Installasjonen er avbrutt"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Installasjonen ble ikke fullf�rt riktig."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Avinstallasjon p�g�r"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Vennligst vent mens $(^NameDA) blir avinstallert."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Avinstallasjon ferdig"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Avinstallasjonen ble utf�rt uten feil."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Avinstallasjon avbrutt"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Avinstallasjonen ble ikke utf�rt riktig."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Avslutter $(^NameDA) installasjonsveiviser"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) er klart til bruk p� din datamskin.$\r$\n$\r$\nTrykk Ferdig for � avslutte installasjonsprogrammet."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Du m� starte datamaskinen p� nytt for � fullf�re installasjonen av $(^NameDA). Vil du starte datamaskinen p� nytt n�?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Fullf�rer avinstallasjonen av $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) har blitt avinstallert fra din datamaskin.$\r$\n$\r$\nTrykk p� ferdig for � avslutte denne veiviseren."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Datamaskinen m� starte p� nytt for � fullf�re avinstallasjonen av $(^NameDA). Vil du starte datamaskinen p� nytt n�?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Ja. Start datamaskinen p� nytt n�"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Nei. Jeg vil starte datamaskinen p� nytt senere"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Kj�r $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Vis Readme filen"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Ferdig"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Velg plassering p� startmenyen"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Velg hvilken mappe snarveiene til $(^NameDA) skal ligge i."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Velg mappe for snarveiene til programmet. Du kan ogs� skrive inn et nytt navn for � lage en ny mappe."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Ikke lag snarveier"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Avinstaller $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Fjern $(^NameDA) fra din datamaskin."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Er du sikker p� at du vil avslutte installasjonen av $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Er du sikker p� at du vil avbryte avinstallasjonen av $(^Name)?"
!endif
