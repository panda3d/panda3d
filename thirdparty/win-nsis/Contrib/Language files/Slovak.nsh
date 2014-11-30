;Language: Slovak (1051)
;Translated by:
;  Kypec (peter.dzugas@mahe.sk)
;edited by:
;  Mari�n Hikan�k (podnety@mojepreklady.net)
;  Ivan Mas�r <helix84@centrum.sk>, 2008.

!insertmacro LANGFILE "Slovak" "Slovensky"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Vitajte v sprievodcovi in�tal�ciou programu $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Tento sprievodca v�s prevedie in�tal�ciou $(^NameDA).$\r$\n$\r$\nPred za�iatkom in�tal�cie sa odpor��a ukon�i� v�etky ostatn� programy. T�m umo�n�te aktualizovanie syst�mov�ch s�borov bez potreby re�tartovania v�ho po��ta�a.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Vitajte v sprievodcovi odin�talovan�m programu $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Tento sprievodca v�s prevedie procesom odin�talovania programu $(^NameDA).$\r$\n$\r$\nPred spusten�m procesu odin�talovania sa uistite, �e program $(^NameDA) nie je pr�ve akt�vny.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licen�n� zmluva"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Pred in�tal�ciou $(^NameDA) si pros�m pre�tudujte licen�n� podmienky."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Ak s�hlas�te s podmienkami zmluvy, kliknite na tla�idlo S�hlas�m a m��ete pokra�ova� v in�tal�cii. Ak chcete v in�tal�cii pokra�ova�, mus�te ods�hlasi� podmienky licen�nej zmluvy $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ak s�hlas�te s podmienkami zmluvy, za�krtnite ni��ie uveden� pol��ko. Ak chcete v in�tal�cii pokra�ova�, mus�te ods�hlasi� podmienky licen�nej zmluvy $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ak s�hlas�te s podmienkami zmluvy, ozna�te prv� z ni��ie uveden�ch mo�nost�. Ak chcete v in�tal�cii pokra�ova�, mus�te ods�hlasi� podmienky licen�nej zmluvy $(^NameDA)."
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licen�n� zmluva"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Pred odin�talovan�m programu $(^NameDA) si pros�m pre��tajte licen�n� podmienky."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Ak s�hlas�te s podmienkami zmluvy, zvo�te S�hlas�m. Licen�n� zmluvu mus�te ods�hlasi�, ak chcete v odin�talovan� programu $(^NameDA) pokra�ova�."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ak s�hlas�te s podmienkami zmluvy, za�krtnite ni��ie uveden� pol��ko. Licen�n� zmluvu mus�te ods�hlasi�, ak chcete pokra�ova� v odin�talovan� programu $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ak s�hlas�te s podmienkami licen�nej zmluvy, ozna�te prv� z ni��ie uveden�ch mo�nost�. Licen�n� zmluvu mus�te ods�hlasi�, ak chcete pokra�ova� v odin�talovan� programu $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Stla�en�m kl�vesu Page Down posuniete text licen�nej zmluvy."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Vo�ba s��ast� programu"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Zvo�te si tie s��asti programu $(^NameDA), ktor� chcete nain�talova�."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Vo�ba s��ast�"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Zvo�te s��asti programu $(^NameDA), ktor� chcete odin�talova�."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Popis"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Pri prejden� kurzorom my�i nad n�zvom s��asti sa zobraz� jej popis."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Pri prejden� kurzorom my�i nad n�zvom s��asti sa zobraz� jej popis."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Vo�ba umiestnenia programu"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Vyberte si prie�inok, do ktor�ho chcete nain�talova� program $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Umiestenie programu pre odin�talovanie"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Vyberte si prie�inok, z ktor�ho chcete odin�talova� program $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "In�tal�cia"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Po�kajte pros�m, k�m prebehne in�tal�cia programu $(^NameDA)."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Ukon�enie in�tal�cie"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "In�tal�cia bola dokon�en� �spe�ne."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Preru�enie in�tal�cie"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "In�tal�ciu sa nepodarilo dokon�i�."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Odin�talovanie"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "�akajte pros�m, k�m prebehne odin�talovanie programu $(^NameDA)."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Ukon�enie odin�talovania"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Odin�talovanie bolo �spe�ne dokon�en�."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Preru�enie odin�talovania"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Odin�talovanie sa neukon�ilo �spe�ne."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Dokon�enie in�tal�cie programu $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Program $(^NameDA) bol nain�talovan� do v�ho po��ta�a.$\r$\nKliknite na tla�idlo Dokon�i� a tento sprievodca sa ukon��."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Pre �pln� dokon�enie in�tal�cie programu $(^NameDA) je potrebn� re�tartova� v� po��ta�. Chcete ho re�tartova� ihne�?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Dokon�enie sprievodcu odin�talovan�m"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "Program $(^NameDA) bol odin�talovan� z v�ho po��ta�a.$\r$\n$\r$\nKliknite na tla�idlo Dokon�i� a tento sprievodca sa ukon��."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Pre �pln� dokon�enie odin�talovania programu $(^NameDA) je nutn� re�tartova� v� po��ta�. Chcete ho re�tartova� ihne�?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Re�tartova� teraz"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Re�tartova� nesk�r (manu�lne)"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Spusti� program $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Zobrazi� s�bor s inform�ciami"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Dokon�i�"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Vo�ba umiestnenia v ponuke �tart"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Vyberte si prie�inok v ponuke �tart, kam sa umiestnia odkazy na program $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Vyberte si prie�inok v ponuke �tart, v ktorom chcete vytvori� odkazy na program. Takisto m��ete nap�sa� n�zov nov�ho prie�inka."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nevytv�ra� odkazy"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Odin�talovanie programu $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Odstr�nenie programu $(^NameDA) z v�ho po��ta�a."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Naozaj chcete ukon�i� in�tal�ciu programu $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Naozaj chcete ukon�i� proces odin�talovania programu $(^Name)?"
!endif

!ifdef MULTIUSER_INSTALLMODEPAGE
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_TITLE "Vybra� pou��vate�ov"
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_SUBTITLE "Vyberte pre ktor�ch pou��vate�ov chcete nain�talova� $(^NameDA)."
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_TOP "Vyberte, �i chcete nain�talova� program $(^NameDA) iba pre seba alebo pre v�etk�ch pou��vate�ov tohto po��ta�a. $(^ClickNext)"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS "Nain�talova� pre v�etk�ch pou��vate�ov tohto po��ta�a"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER "Nain�talova� iba pre m�a"
!endif