;Language: Icelandic (15)
;By Gretar Orri Kristinsson

!insertmacro LANGFILE "Icelandic" "Icelandic"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Velkominn til $(^NameDA) uppsetningarhj�lparinnar"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "�essi hj�lp mun lei�a �ig � gegnum uppsetninguna � $(^NameDA).$\r$\n$\r$\nM�lt er me� �v� a� �� lokir �llum ��rum forritum ��ur en uppsetningin hefst. �etta mun gera uppsetningarforritinu kleyft a� uppf�ra kerfiskr�r �n �ess a� endurr�sa t�lvuna.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Velkomin(n) til $(^NameDA) fjarl�gingarhj�lparinnar"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "�essi hj�lp mun lei�a �ig � gegnum fjarl�ginguna � $(^NameDA).$\r$\n$\r$\n��ur en fjarl�ging hefst skal ganga �r skugga um a� $(^NameDA) s� ekki opi�.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Notandaleyfissamningur"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Vinsamlegast sko�a�u Notandaleyfissamninginn vel ��ur en uppsetning � $(^NameDA) hefst."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Ef �� sam�ykkir skilm�la samningsins, smelltu �� � '�g sam�ykki' til a� halda �fram. �� ver�ur a� sam�ykkja samninginn til �ess a� setja upp $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ef �� sam�ykkir skilm�la samningsins, haka�u �� � kassann h�r a� ne�an. �� ver�ur a� sam�ykkja samninginn til �ess a� setja upp $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ef �� sam�ykkir skilm�la samningsins, veldu �� fyrsta valm�guleikann h�r a� ne�an. �� ver�ur a� sam�ykkja samninginn til �ess a� setja upp $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Leyfissamningur"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Vinsamlegast sko�a�u leyfissamninginn vel ��ur en fjarl�ging � $(^NameDA) hefst."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Ef �� sam�ykkir skilm�la samningsins, smelltu �� � '�g sam�ykki' til a� halda �fram. �� ver�ur a� sam�ykkja samninginn til �ess a� fjarl�gja $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ef �� sam�ykkir skilm�la samningsins, haka�u �� � kassann h�r a� ne�an. �� ver�ur a� sam�ykkja samninginn til �ess a� fjarl�gja $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ef �� sam�ykkir skilm�la samningsins, veldu �� fyrsta valm�guleikann h�r a� ne�an. �� ver�ur a� sam�ykkja samninginn til �ess a� fjarl�gja $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Smelltu � 'PageDown' takkann � lyklabor�inu til a� sj� afganginn af samningnum."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Velja �hluti"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Veldu hva�a $(^NameDA) �hluti �� vilt setja upp."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Velja �hluti"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Velja hva�a $(^NameDA) �hluti �� vilt fjarl�gja."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "L�sing"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "F�r�u m�sina yfir �hlut til a� f� l�singuna � honum."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "F�r�u m�sina yfir �hlut til a� f� l�singuna � honum."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Veldu uppsetningarsk�arsafn"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Veldu �a� skr�arsafn sem �� vilt setja $(^NameDA) upp �."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Velja fjarl�gingarsk�arsafn"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Veldu �a� skr�arsafn sem �� vilt fjarl�gja $(^NameDA) �r."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Set upp"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Vinsamlegast doka�u vi� me�an $(^NameDA) er sett upp."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Uppsetningu loki�"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Uppsetning t�kst."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "H�tt vi� uppsetningu"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Uppsetningu lauk ekki sem skildi."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Fjarl�gi"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Vinsamlegast doka�u vi� � me�an $(^NameDA) er fjarl�gt."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Fjarl�gingu loki�"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Fjarl�ging t�kst."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "H�tt vi� fjarl�gingu"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Fjarl�gingu lauk ekki sem skildi."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Lj�ka $(^NameDA) uppsetningarhj�lpinni"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) er n� upp sett � t�lvunni �inni.$\r$\n$\r$\nSmelltu � 'Lj�ka' til a� loka �essari hj�lp."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Til a� lj�ka uppsetningunni � $(^NameDA) ver�ur a� endurr�sa t�lvuna. Viltu endurr�sa n�na?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Lj�ka $(^NameDA) fjarl�gingarhj�lpinni"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) hefur n� veri� fjarl�gt �r t�lvunni.$\r$\n$\r$\nSmelltu � 'Lj�ka' til a� loka �essari hj�lp."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Til a� lj�ka fjarl�gingunni � $(^NameDA) ver�ur a� endurr�sa t�lvuna. Viltu endurr�sa n�na?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Endurr�sa n�na"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "�g vil endurr�sa seinna"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Keyra $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Sko�a LestuMig"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Lj�ka"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Velja skr�arsafn 'Start' valmyndar"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Veldu skr�arsafn $(^NameDA) fl�tilei�a fyrir 'Start' valmyndina."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Veldu skr�arsafn fl�tilei�a forritsins fyrir 'Start' valmyndina. �� getur einnig b�i� til n�tt skr�arsafn me� �v� a� setja inn n�tt nafn."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Ekki b�a til fl�tilei�ir � 'Start' valmyndinni"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Fjarl�gja $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Fjarl�gja $(^NameDA) �r t�lvunni."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Ertu viss um a� �� viljir loka $(^Name) uppsetningarhj�lpinni?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Ertu viss um a� �� viljir loka $(^Name) fjarl�gingarhj�lpinni?"
!endif
