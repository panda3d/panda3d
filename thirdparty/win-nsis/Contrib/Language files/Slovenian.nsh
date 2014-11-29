;Language: Slovenian (1060)
;By Janez Dolinar, edited by Martin Srebotnjak - Lugos.si

!insertmacro LANGFILE "Slovenian" "Slovenski jezik"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Dobrodo�li v �arovniku namestitve $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Ta �arovnik vas vodi skozi namestitev programa $(^NameDA).$\r$\n$\r$\nPred namestitvijo je priporo�eno zapreti vsa ostala okna in programe. S tem omogo�ite nemoteno namestitev programa in potrebnih sistemskih datotek brez ponovnega zagona ra�unalnika.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Dobrodo�li v �arovniku za odstranitev $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Ta �arovnik vas bo vodil skozi odstranitev $(^NameDA).$\r$\n$\r$\nPreden pri�nete z odstranitvijo, se prepri�ajte, da program $(^NameDA) ni zagnan.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licen�na pogodba"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Prosimo, da si ogledate pogoje licen�ne pogodbe pred namestitvijo $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "�e se strinjate s pogoji, pritisnite Se strinjam. Da bi lahko namestili $(^NameDA), se morate s pogodbo strinjati."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "�e se strinjate z licen�nimi pogoji pogodbe, spodaj izberite ustrezno okence. Za namestitev $(^NameDA) se morate strinjati s pogoji pogodbe. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "�e se strinjate z licen�nimi pogoji pogodbe, spodaj izberite prvo mo�nost. Za namestitev $(^NameDA) se morate strinjati s pogoji pogodbe. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licen�na pogodba"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Prosimo, da pred odstranitvijo $(^NameDA) pregledate pogoje licen�ne pogodbe."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "�e se strinjate s pogoji licen�ne pogodbe, izberite Se strinjam. Za odstranitev $(^NameDA) se morate strinjati s pogoji."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "�e se strinjate s pogoji licen�ne pogodbe, kliknite na okence spodaj. Za odstranitev $(^NameDA) se morate strinjati s pogoji. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "�e se strinjate s pogoji licen�ne pogodbe, spodaj izberite prvo podano mo�nost. Za odstranitev $(^NameDA) se morate strinjati s pogoji. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Za preostali del pogodbe pritisnite tipko 'Page Down'."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Izbor komponent"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Izberite, katere komponente izdelka $(^NameDA) �elite namestiti."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Izbor komponent"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Izberite komponente $(^NameDA), ki jih �elite odstraniti."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Opis"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Povlecite mi�ko nad komponento, da vidite njen opis."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Povlecite mi�ko nad komponento, da vidite njen opis."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Izberite pot namestive"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Izberite mapo, v katero �elite namestiti $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Izbor mape"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Izberite mapo, iz katere �elite odstraniti $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Name��anje poteka"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Prosimo, po�akajte, $(^NameDA) se name��a."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Dokon�ana namestitev"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Namestitev je uspe�no zaklju�ena."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Prekinjena namestitev"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Namestitev ni bila uspe�no zaklju�ena."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Odstranjevanje poteka"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Prosimo, po�akajte, dokler se program $(^NameDA) odstranjuje."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Odstranitev kon�ana"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Odstranitev je uspe�no kon�ana."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Odstranitev prekinjena"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Odstranitev ni bila kon�ana uspe�no."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Zaklju�evanje namestitve $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Program $(^NameDA) je bil name��en na va� ra�unalnik.$\r$\n$\r$\nPritisnite Dokon�aj za zaprtje �arovnika."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Za dokon�anje namestitve $(^NameDA) morate ponovno zagnati ra�unalnik. �elite zdaj ponovno zagnati ra�unalnik?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "�arovnik za odstranitev $(^NameDA) se zaklju�uje"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "Program $(^NameDA) je odstranjen z va�ega ra�unalnika.$\r$\n$\r$\nKliknite Dokon�aj, da zaprete �arovnika."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Da bi se namestitev $(^NameDA) dokon�ala, morate ponovno zagnati ra�unalnik. �elite zdaj znova zagnati ra�unalnik?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Ponovni zagon"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Ra�unalnik �elim znova zagnati kasneje"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Za�eni $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Poka�i BeriMe"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "Do&kon�aj"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Izberite mapo menija Start"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Izberite mapo menija Start za bli�njice do $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Izberite mapo menija Start, kjer �elite ustvariti bli�njico do programa. �e vpi�ete novo ime, boste ustvarili istoimensko mapo."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Ne ustvari bli�njic"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Odstranitev $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Odstrani $(^NameDA) z va�ega ra�unalnika."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Ste prepri�ani, da �elite prekiniti namestitev $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Ste prepri�ani, da �elite zapustiti odstranitev $(^Name)?"
!endif

!ifdef MULTIUSER_INSTALLMODEPAGE
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_TITLE "Izberite uporabnike"
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_SUBTITLE "Izberite uporabnike, za katere �elite namestiti $(^NameDA)."
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_TOP "Izberite, ali �elite namestiti $(^NameDA) le zase ali za vse uporabnike tega ra�unalnika. $(^ClickNext)"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS "Namesti za vse uporabnike tega ra�unalnika"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER "Namesti le zame"
!endif
