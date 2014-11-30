;Language: Serbian Latin (2074)
;Translation by Sr�an Obu�ina <obucina@srpskijezik.edu.yu>

!insertmacro LANGFILE "SerbianLatin" "Serbian Latin"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Dobrodo�li u vodi� za instalaciju programa $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Bi�ete vo�eni kroz proces instalacije programa $(^NameDA).$\r$\n$\r$\nPreporu�ljivo je da isklju�ite sve druge programe pre po�etka instalacije. Ovo mo�e omogu�iti a�uriranje sistemskih fajlova bez potrebe za ponovnim pokretanjem ra�unara.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Dobrodo�li u deinstalaciju programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Bi�ete vo�eni kroz proces deinstalacije programa $(^NameDA).$\r$\n$\r$\nPre po�etka deinstalacije, uverite se da je program $(^NameDA) isklju�en. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Dogovor o pravu kori��enja"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Pa�ljivo pro�itajte dogovor o pravu kori��enja pre instalacije programa $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Ako prihvatate sve uslove dogovora, pritisnite dugme �Prihvatam� za nastavak. Morate prihvatiti dogovor da biste instalirali program $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ako prihvatate sve uslove dogovora, obele�ite kvadrati� ispod. Morate prihvatiti dogovor da biste instalirali program $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ako prihvatate sve uslove dogovora, izaberite prvu opciju ispod. Morate prihvatiti dogovor da biste instalirali program $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Dogovor o pravu kori��enja"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Pa�livo pro�itajte dogovor o pravu kori��enja pre deinstalacije programa $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Ako prihvatate sve uslove dogovora, pritisnite dugme �Prihvatam� za nastavak. Morate prihvatiti dogovor da biste deinstalirali program $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ako prihvatate sve uslove dogovora, obele�ite kvadrati� ispod. Morate prihvatiti dogovor da biste deinstalirali program $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ako prihvatate sve uslove dogovora, izaberite prvu opciju ispod. Morate prihvatiti dogovor da biste deinstalirali program $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Pritisnite Page Down da biste videli ostatak dogovora."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Izbor komponenti za instalaciju"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Izaberite komponente za instalaciju. Instaliraju se samo ozna�ene komponente."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Izbor komponenti za deinstalaciju"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Izaberite komponente za deinstalaciju. Deinstaliraju se samo ozna�ene komponente."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Opis"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Pre�ite kursorom mi�a preko imena komponente da biste videli njen opis."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Pre�ite kursorom mi�a preko imena komponente da biste videli njen opis."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Izbor foldera za instalaciju"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Izaberite folder u koji �ete instalirati program $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Izbor foldera za deinstalaciju"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Izaberite folder iz koga �ete deinstalirati program $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instalacija"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Sa�ekajte dok se program $(^NameDA) instalira."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Zavr�ena instalacija"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Instalacija je uspe�no zavr�ena."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Prekinuta instalacija"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Instalacija je prekinuta i nije uspe�no zavr�ena."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Deinstalacija"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Sa�ekajte dok se program $(^NameDA) deinstalira."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Zavr�ena deinstalacija"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Deinstalacija je uspe�no zavr�ena."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Prekinuta deinstalacija"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Deinstalacija je prekinuta i nije uspe�no zavr�ena."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Zavr�ena instalacija programa $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Program $(^NameDA) je instaliran na ra�unar.$\r$\n$\r$\nPritisnite dugme �Kraj� za zatvaranje ovog prozora."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Ra�unar mora biti ponovo pokrenut da bi se proces instalacije programa $(^NameDA) uspe�no zavr�io. �elite li to odmah da uradite?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Zavr�ena deinstalacija programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "Program $(^NameDA) je deinstaliran sa ra�unara.$\r$\n$\r$\nPritisnite dugme �Kraj� za zatvaranje ovog prozora."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Ra�unar mora biti ponovo pokrenut da bi se proces deinstalacije programa $(^NameDA) uspe�no zavr�io. �elite li to da uradite odmah?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Odmah ponovo pokreni ra�unar"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Bez ponovnog pokretanja"
  ${LangFileString} MUI_TEXT_FINISH_RUN "Pokreni program $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Prika�i Pro�itajMe fajl"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "Kraj"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Izbor foldera u Start meniju"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Izaberite folder u Start meniju u kome �ete kreirati pre�ice."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Izaberite folder u Start meniju u kome �elite da budu kreirane pre�ice programa. Mo�ete upisati i ime za kreiranje novog foldera."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Bez kreiranja pre�ica"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Deinstalacija programa $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Deinstalacija programa $(^NameDA) sa ra�unara."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Sigurno �elite da prekinete instalaciju programa $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Sigurno �elite da prekinete deinstalaciju programa $(^Name)?"
!endif
