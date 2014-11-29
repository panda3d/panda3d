;Language: Luxembourgish (1031)
;By Snowloard, changes by Philo

!insertmacro LANGFILE "Luxembourgish" "L�tzebuergesch"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "W�llkomm beim Installatiouns-$\r$\nAssistent vun $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "D�sen Assistent w�rt dech duech d'Installatioun vun $(^NameDA) begleeden.$\r$\n$\r$\nEt g�tt ugeroden alleguer d'Programmer di am Moment lafen zouzemaan, datt best�mmt Systemdateien ouni Neistart ersat k�nne ginn.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "W�llkomm am Desinstallatiouns-$\r$\n\Assistent fir $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Desen Assistent begleet dech duech d'Desinstallatioun vun $(^NameDA).$\r$\n$\r$\nW.e.g. maach $(^NameDA) zu, ierts de mat der Desinstallatioun uf�nks.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Lizenzofkommes"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "W.e.g. d'Lizenzoofkommes liesen, ierts de mat der Installatioun weiderfiers."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Wanns de alleguer d'Bedengungen vum Ofkommes akzept�iers, klick op Unhuelen. Du muss alleguer d'Fuerderungen unerkennen, fir $(^NameDA) install�ieren ze k�nnen."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Wanns de alleguer d'Bedengungen vum Ofkommes akzept�iers, aktiv�ier d'K�schtchen. Du muss alleguer d'Fuerderungen unerkennen, fir $(^NameDA) install�ieren ze k�nnen. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Wanns de alleguer d'Bedengungen vum Ofkommes akzept�iers, wiel �nnen di entspriechend �ntwert aus. Du muss alleguer d'Fuerderungen unerkennen, fir $(^NameDA) install�ieren ze k�nnen. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Lizenzofkommes"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "W.e.g. lies d'Lizenzofkommes duech ierts de mat der Desinstallatioun vun $(^NameDA) weiderfiers."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Wanns de d'Fuerderungen vum Ofkommes akzept�iers, klick op unhuelen. Du muss d'Ofkommes akzept�ieren, fir $(^NameDA) k�nnen ze desinstall�ieren."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Wanns de d'Fuerderungen vum Ofkommes akzept�iers, aktiv�ier d'K�schtchen. Du muss d'Ofkommes akzept�ieren, fir $(^NameDA) k�nnen ze desinstall�ieren. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Wanns de d'Fuerderungen vum Ofkommes akzept�iers, wiel �nnen di entspriechend Optioun. Du muss d'Oofkommes akzept�ieren, fir $(^NameDA) kennen ze desinstall�ieren. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Dr�ck d'PageDown-Tast fir den Rescht vum Ofkommes ze liesen."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Komponenten auswielen"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Wiel d'Komponenten aus, d�is de w�lls install�ieren."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Komponenten auswielen"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Wiel eng Komponent aus, d�is de desinstall�ieren w�lls."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Beschreiwung"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Hal den Mausfeil iwwer eng Komponent, fir d'Beschreiwung dervun ze gesinn."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Hal den Mausfeil iwwer eng Komponent, fir d'Beschreiwung dervun ze gesinn."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Zielverzeechnes auswielen"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Wiel den Dossier aus, an deen $(^NameDA) install�iert soll ginn."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Dossier fir d'Desinstallatioun wielen"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Wiel den Dossier aus, aus dem $(^NameDA) desinstall�iert soll ginn."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Install�ieren..."
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Waard w.e.g w�hrend deem $(^NameDA) install�iert g�tt."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Installatioun f�rdeg"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "D'Installatioun ass feelerfr�i oofgeschloss ginn."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Installatioun ofgebrach"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "D'Installatioun ass net komplett ofgeschloss ginn."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Desinstall�ieren..."
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "W.e.g. waard, w�hrend deems $(^NameDA) desinstall�iert g�tt."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Desinstallatioun ofgeschloss"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "D'Desinstallatioun ass erfollegr�ich ofgeschloss ginn."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Desinstallatioun oofbriechen"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Desinstallatioun ass net erfollegr�ich ofgeschloss ginn."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "D'Installatioun vun $(^NameDA) g�tt ofgeschloss."
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) ass um Computer install�iert ginn.$\r$\n$\r$\nKlick op f�rdeg maan, fir den Installatiouns-Assistent zou ze maan.."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Den Windows muss nei gestart ginn, fir d'Installatioun vun $(^NameDA) ofzeschl�issen. W�lls de Windows lo n�i starten?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Desinstallatioun vun $(^NameDA) g�tt ofgeschloss"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) ass vum Computer desinstall�iert ginn.$\r$\n$\r$\nKlick op Ofschl�issen fir den Assistent zou ze maan."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Windows muss n�i gestart gin, fir d'Desinstallatioun vun $(^NameDA) ze vervollst�nnegen. W�lls de Windows lo n�i starten?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Lo n�i starten"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Sp�ider manuell n�i starten"
  ${LangFileString} MUI_TEXT_FINISH_RUN "$(^NameDA) op maan"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Liesmech op maan"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&F�rdeg man"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Startmen�-Dossier best�mmen"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Best�mm een Startman�-Dossier an deen d'Programmofkierzungen kommen."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Best�mm een Startman�-Dossier an deen d'Programmofkierzungen kommen. Wanns de een n�ien Dossier man wells, g�ff deem s�in zuk�nftegen Numm an."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Keng Ofkierzungen man"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Desinstallatioun vun $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "$(^NameDA) gett vum Computer desinstall�iert."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Bass de s�cher, dass de d'Installatioun vun $(^Name) ofbriechen w�lls?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Bass de s�cher, dass de d'Desinstallatioun vun $(^Name) ofbriechen w�lls?"
!endif
