;Language: Bosnian (5146)
;By Salih �avki�, cavkic@skynet.be

!insertmacro LANGFILE "Bosnian" "Bosanski"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Dobrodo�li u program za instalaciju $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Ovaj program �e instalirati $(^NameDA) na Va� sistem. $\r$\n$\r$\nPreporu�ujemo da neizostavno zatvorite sve druge otvorene programe prije nego �to definitivno zapo�nete sa instaliranjem. To �e omogu�iti bolju nadogradnju odre�enih sistemskih datoteka bez potrebe da Va� ra�unar ponovo startujete. Instaliranje programa mo�ete prekinuti pritiskom na dugme 'Odustani'.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Dobrodo�li u postupak uklanjanja programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Ovaj �e Vas vodi� provesti kroz postupak uklanjanja programa $(^NameDA).$\r$\n$\r$\nPrije samog po�etka, molim zatvorite program $(^NameDA) ukoliko je slu�ajno otvoren.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licencni ugovor"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Molim pro�itajte licencni ugovor $(^NameDA) prije instalacije programa."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Ako prihvatate uslove licence, odaberite 'Prihvatam' za nastavak. Morate prihvatiti licencu za instalaciju programa $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ako prihvatate uslove licence, ozna�ite donji kvadrati�. Morate prihvatiti licencu za instalaciju programa $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ako prihvatate uslove licence, odaberite prvu donju opciju. Morate prihvatiti licencu za instalaciju programa $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licencni ugovor o pravu kori�tenja"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Molim pro�itajte licencu prije uklanjanja programa $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Ako prihvatate uslove licence, odaberite 'Prihvatam' za nastavak. Morate prihvatiti licencu za uklanjanje programa $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ako prihvatate uslove licence, ozna�ite donji kvadrati�. Morate prihvatiti licencu za uklanjanje programa $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ako prihvatate uslove licence, odaberite prvu donju opciju. Morate prihvatiti licencu za uklanjanje programa $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Pritisnite 'Page Down' na tastaturi za ostatak licence."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Izbor komponenti za instalaciju"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Izaberite komponente programa $(^NameDA) koje �elite instalirati."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Izbor komponenti za uklanjanje"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Izaberite komponente programa $(^NameDA) koje �elite ukloniti."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Opis"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Postavite kursor od mi�a iznad komponente da biste vidjeli njezin opis."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Postavite kursor od mi�a iznad komponente da biste vidjeli njezin opis."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Odaberite odredi�te za instalaciju"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Odaberite mapu u koju �elite instalirati program $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Izaberite polazi�te za uklanjanje"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Selektirajte mapu iz koje �elite ukloniti program $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instaliranje"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Molim pri�ekajte na zavr�etak instalacije programa $(^NameDA)."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Kraj instalacije"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Instalacija je u potpunosti uspje�no zavr�ila."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalacija je prekinuta"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Instalacija nije zavr�ila uspje�no."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Uklanjanje"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Molim Vas pri�ekajte da vodi� zavr�i uklanjanje $(^NameDA) programa."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Uklanjanje je zavr�eno"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Uklanjanje je u potpunosti zavr�ilo uspje�no."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Uklanjanje je prekinuto"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Uklanjanje nije zavr�ilo uspje�no."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Dovr�avanje instalacije programa $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Program $(^NameDA) je instaliran na Va�e ra�unar.$\r$\n$\r$\nPritisnite dugme 'Kraj' za zavr�etak."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Ra�unar treba ponovno startovati za dovr�avanje instalacije programa $(^NameDA). �elite li to u�initi sada?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Zavr�etak uklanjanja programa $(^NameDA) sa Va�eg sistema."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "Program $(^NameDA) je uklonjen sa Va�eg ra�unara.$\r$\n$\r$\nPritisnite dugme 'Kraj' za zatvaranje ovog prozora."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Va� ra�unar trebate ponovno startovati da dovr�ite uklanjanje programa $(^NameDA). �elite li da odmah sad ponovo startujete ra�unar?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Startuj ra�unar odmah sad"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Ponovno �u pokrenuti ra�unar kasnije"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Pokreni program $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Prika�i datoteku &Readme"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Kraj"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Izbor mape u Start meniju"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Izaberite ime za programsku mapu unutar Start menija."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Izaberite jednu mapu u Start meniju u kojoj �elite da se kreiraju pre�ice programa. Mo�ete tako�er unijeti ime za novu mapu ili selektirati ve� postoje�u."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nemojte praviti pre�ice"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Uklanjanje programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Program $(^NameDA) �e biti uklonjen sa Va�eg ra�unara."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Jeste li sigurni da �elite prekinuti instalaciju programa $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Jeste li sigurni da �elite prekinuti uklanjanje $(^Name) programa?"
!endif
