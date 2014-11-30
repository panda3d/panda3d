;Compatible with Modern UI 1.72
;Language: Swedish (1053)
;By Magnus Bonnevier (magnus.bonnevier@telia.com), updated by Rickard Angbratt (r.angbratt@home.se), updated by Ulf Axelsson (ulf.axelsson@gmail.com)

!insertmacro LANGFILE "Swedish" "Svenska"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "V�lkommen till installationsguiden f�r $(^NameDA)."
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Denna guide tar dig igenom installationen av $(^NameDA).$\r$\n$\r$\nDet rekommenderas att du avslutar alla andra program innan du forts�tter installationen. Detta till�ter att installationen uppdaterar n�dv�ndiga systemfiler utan att beh�va starta om din dator.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "V�lkommen till avinstallationsguiden f�r $(^NameDA)."
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Denna guide tar dig igenom avinstallationen av $(^NameDA).$\r$\n$\r$\nInnan du startar avinstallationen, f�rs�kra dig om att $(^NameDA) inte k�rs.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licensavtal"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Var v�nlig l�s igenom licensvillkoren innan du installerar $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Om du accepterar villkoren i avtalet, klicka Jag Godk�nner f�r att forts�tta. Du m�ste acceptera avtalet f�r att installera $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Om du accepterar villkoren i avtalet, klicka i checkrutan nedan. Du m�ste acceptera avtalet f�r att installera $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Om du accepterar villkoren i avtalet, v�lj det f�rsta alternativet nedan. Du m�ste acceptera avtalet f�r att installera $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licensavtal"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Var v�nlig l�s igenom licensvillkoren innan du avinstallerar $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Om du accepterar villkoren i avtalet, klicka Jag Godk�nner f�r att forts�tta. Du m�ste acceptera avtalet f�r att avinstallera $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Om du accepterar villkoren i avtalet, klicka i checkrutan nedan. Du m�ste acceptera avtalet f�r att avinstallera $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Om du accepterar villkoren i avtalet, v�lj det f�rsta alternativet nedan. Du m�ste acceptera avtalet f�r att avinstallera $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Tryck Page Down f�r att se resten av licensavtalet."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "V�lj komponenter"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "V�lj vilka alternativ av $(^NameDA) som du vill installera."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "V�lj komponenter"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "V�lj vilka alternativ av $(^NameDA) som du vill avinstallera."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Beskrivning"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "H�ll muspekaren �ver ett alternativ f�r att se dess beskrivning."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "H�ll muspekaren �ver ett alternativ f�r att se dess beskrivning."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "V�lj installationsv�g"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "V�lj katalog att installera $(^NameDA) i."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "V�lj avinstallationsv�g"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "V�lj katalog att avinstallera $(^NameDA) fr�n."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Installerar"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Var v�nlig v�nta medan $(^NameDA) installeras."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Installationen �r klar"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Guiden avslutades korrekt."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Installationen avbr�ts"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Guiden genomf�rdes inte korrekt."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Avinstallerar"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Var v�nlig v�nta medan $(^NameDA) avinstalleras."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Avinstallationen genomf�rd"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Avinstallationen genomf�rdes korrekt."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Avinstallationen avbruten"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Avinstallationen genomf�rdes inte korrekt."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Avslutar installationsguiden f�r $(^NameDA)."
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) har installerats p� din dator.$\r$\n$\r$\nKlicka p� Slutf�r f�r att avsluta guiden."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Din dator m�ste startas om f�r att fullborda installationen av $(^NameDA). Vill du starta om nu?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Avslutar avinstallationsguiden f�r $(^NameDA)."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) komponenter har avinstallerats fr�n din dator.$\r$\n$\r$\nKlicka p� Slutf�r f�r att avsluta guiden."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Din dator m�ste startas om f�r att fullborda avinstallationen av $(^NameDA). Vill du starta om nu?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Starta om nu"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Jag vill starta om sj�lv senare"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&K�r $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Visa Readme-filen"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Slutf�r"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "V�lj Startmenykatalog"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "V�lj en Startmenykatalog f�r programmets genv�gar."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "V�lj startmenykatalog i vilken du vill skapa programmets genv�gar. Du kan ange ett eget namn f�r att skapa en ny katalog."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Skapa ej genv�gar"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Avinstallera $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Ta bort $(^NameDA) fr�n din dator."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "�r du s�ker p� att du vill avbryta installationen av $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "�r du s�ker p� att du vill avbryta avinstallationen av $(^Name)?"
!endif
