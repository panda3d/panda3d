;Language: Croatian (1050)
;By Igor Ostriz

!insertmacro LANGFILE "Croatian" "Hrvatski"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Dobrodo�li u instalaciju programa $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Instalacija programa $(^NameDA) na Va�e ra�unalo sastoji se od nekoliko jednostavnih koraka kroz koje �e Vas provesti ovaj �arobnjak.$\r$\n$\r$\nPreporu�amo zatvaranje svih ostalih aplikacija prije samog po�etka instalacije. To �e omogu�iti nadogradnju nekih sistemskih datoteka bez potrebe za ponovnim pokretanjem Va�eg ra�unala. U svakom trenutku instalaciju mo�ete prekinuti pritiskom na 'Odustani'.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Dobrodo�li u postupak uklanjanja programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Ovaj �e Vas �arobnjak provesti kroz postupak uklanjanja programa $(^NameDA).$\r$\n$\r$\nPrije samog po�etka, molim zatvorite program $(^NameDA) ukoliko je slu�ajno otvoren.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licen�ni ugovor"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Molim pro�itajte licencu prije instalacije programa $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Ukoliko prihva�ate uvjete licence, odaberite 'Prihva�am' za nastavak. Morate prihvatiti licencu za instalaciju programa $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ukoliko prihva�ate uvjete licence, ozna�ite donji kvadrati�. Morate prihvatiti licencu za instalaciju programa $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ukoliko prihva�ate uvjete licence, odaberite prvu donju opciju. Morate prihvatiti licencu za instalaciju programa $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licen�ni ugovor"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Molim pro�itajte licencu prije uklanjanja programa $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Ukoliko prihva�ate uvjete licence, odaberite 'Prihva�am' za nastavak. Morate prihvatiti licencu za uklanjanje programa $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ukoliko prihva�ate uvjete licence, ozna�ite donji kvadrati�. Morate prihvatiti licencu za uklanjanje programa $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ukoliko prihva�ate uvjete licence, odaberite prvu donju opciju. Morate prihvatiti licencu za uklanjanje programa $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "'Page Down' za ostatak licence."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Izbor komponenti"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Odaberite komponente programa $(^NameDA) koje �elite instalirati."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Izbor komponenti"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Odaberite koje komponente programa $(^NameDA) �elite ukloniti."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Opis"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Postavite pokaziva� iznad komponente za njezin opis."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Postavite pokaziva� iznad komponente za njezin opis."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Odaberite odredi�te za instalaciju"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Odaberite mapu u koju �elite instalirati program $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Odaberite polazi�te za uklanjanje"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Odaberite mapu iz koje �elite ukloniti program $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instaliranje"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Molim pri�ekajte na zavr�etak instalacije programa $(^NameDA)."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Kraj instalacije"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Instalacija je u potpunosti zavr�ila uspje�no."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalacija je prekinuta"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Instalacija nije zavr�ila uspje�no."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Uklanjanje"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Molim pri�ekajte na zavr�etak uklanjanja programa $(^NameDA)."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Uklanjanje zavr�eno"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Uklanjanje je u potpunosti zavr�ilo uspje�no."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Uklanjanje je prekinuto"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Uklanjanje nije zavr�ilo uspje�no."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Dovr�enje instalacije programa $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Program $(^NameDA) je instaliran na Va�e ra�unalo.$\r$\n$\r$\nOdaberite 'Kraj' za zavr�etak."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Va�e ra�unalo treba ponovno pokrenuti za dovr�enje instalacije programa $(^NameDA). �elite li to u�initi sada?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Zavr�etak uklanjanja programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "Program $(^NameDA) je uklonjen s Va�eg ra�unala.$\r$\n$\r$\nOdaberite 'Kraj' za zatvaranje ovog �arobnjaka."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Va�e ra�unalo treba ponovno pokrenuti za dovr�enje postupka uklanjanja programa $(^NameDA). �elite li to u�initi sada?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Ponovno pokreni ra�unalo sada"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Ponovno �u pokrenuti ra�unalo kasnije"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Pokreni program $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Prika�i &Readme"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Kraj"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Izbor mape u Start meniju"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Odaberite ime za programsku mapu unutar Start menija."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Program �e pripadati odabranoj programskoj mapi u Start meniju. Mo�ete odrediti novo ime za mapu ili odabrati ve� postoje�u."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nemoj napraviti pre�ace"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Uklanjanje programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Program $(^NameDA) �e biti uklonjen s Va�eg ra�unala."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Jeste li sigurni da �elite prekinuti instalaciju programa $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Jeste li sigurni da �elite prekinuti uklanjanje programa $(^Name)?"
!endif
