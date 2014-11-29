;Language: Kurdish
;By R�zan Tovj�n
;Updated by Erdal Ronah� (erdal.ronahi@gmail.com)

!insertmacro LANGFILE "Kurdish" "Kurd�"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "$(^NameDA) Tu bi x�r hat� s�rbaziya sazkirin�"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Ev s�rbaz d� di dema sazkirina $(^NameDA) de r�beriya te bike.$\r$\n$\r$\nBer� tu dest bi sazkirin� bik�, em p��niyar dikin tu hem� bernamey�n vekir� bigir�. Bi v� reng� bey� tu komputera ji n� ve vek� d� hinek dosiy�n pergal� b�pirsgir�k werin sazkirin.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Tu bi x�r hat� s�rbaziya rakirina bernameya $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Ev s�rbaz ji bo rakirina bernameya $(^NameDA) d� al�kariya te bike.$\r$\n$\r$\nBer� tu dest bi rakirina bernamey� bik�, bernamey�n vekir� hem�yan bigire. Bi v� reng� d� re tu mecb�r nam�n� ku komputera xwe bigir� � ji n� ve veki.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Peymana L�sans�"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Ji kerema xwe re ber� tu bernameya $(^NameDA) saz bik�, peymana l�sans� bixw�ne."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Heke tu �ert�n peyman� dipejir�n�, 'Ez Dipejir�nim'� bitik�ne. Ji bo sazkirina bernameya $(^NameDA) div� tu �ert�n peyman� bipejir�n�."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Heke tu �ert�n peyman� dipejir�n�, zeviya pi�trastkirin� ya j�r�n dagire. Ji bo tu bikar� bernameya $(^NameDA) saz bik� div� tu �ert�n peyman� bipejir�n�. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Heke tu �ert�n peyman� dipejir�n�, bi�kojka er�kirin� ya j�r�n bitik�ne. Ji bo sazkirina bernameya $(^NameDA) div� tu �ert�n peyman� bipejir�n�. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Peymana L�sans�"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Ber� tu bernameya $(^NameDA) ji pergala xwe rak� peyman� bixw�ne."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Heke tu �ert�n peyman� dipejir�n�, 'Dipejir�nim'� bitik�ne. Ji bo rakirina bernameya  $(^NameDA) div� tu �ert�n peyman� bipejir�n�."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Heke tu �ert�n peyman� dipejir�n�, zeviya er�kirin� ya j�r�n dagire. Ji bo tu bernameya $(^NameDA) ji pergala xwe rak� div� tu peyman� bipejir�n�. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Heke tu �ert�n peyman� dipejir�n�, bi�kojka er�kirin� ya j�r�n hilbij�re. Ji bo tu bernameya  $(^NameDA) ji pergala xwe rak� div� tu �ert�n peyman� bipejir�n�. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Ji bo d�mah�ka peyman� bi�kojka 'page down' bitik�ne."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Hilbijartina pareyan"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Ji bo sazkirina $(^NameDA) parey�n tu dixwaz� hilbij�re."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Hilbijartina Pareyan"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Pareya bernameya $(^NameDA) ku tu dixwaz� rak� hilbij�re."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Dazan�n"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Ji bo tu der bar� pareyan de agahiyan bist�n� n��anek� bibe ser pareyek�."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Ji bo tu der bar� pareyan de agahiyan bist�n� n��anek� bibe ser pareyek�."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Hilbijartina peldanka armanckir�"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Peldanka tu dixwaz� bernameya $(^NameDA) t� de were sazkirin hilbij�re."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Hilbijartina Peldanka D� Were Rakirin"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Peldanka bernameya $(^NameDA) ku tudixwaz� rak� hilbij�re."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "T� sazkirin"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Ji kerema xwe re heta sazkirina $(^NameDA) biqede raweste."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Sazkirin Qediya"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Sazkirin bi serkeftin� qediya."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Sazkirin hate betalkirin"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Sazkirin be tevah� qediya."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "T� rakirin"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Heta bernameya $(^NameDA) ji pergala te were rakirin raweste."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Rakirina Bernamey� Biqed�ne"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Rakirina bernamey� bi serkeftin p�k hat."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Rakirina bernamey� hate betalkirin"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Rakirina bernamey� neqediya."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "S�rbaziya sazkirina $(^NameDA) diqede."
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) li komputera te hate barkirin.$\r$\n$\r$\n'Biqed�ne'y� bitik�ne � sazkirin� bi daw� b�ne."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Ji bo bidaw�kirina sazkirina $(^NameDA) div� tu komputer� ji n� ve vek�.Tu dixwaz� komputer� ji n� ve vek�?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "S�rbaziya Rakirina Bernameya $(^NameDA) T� Temamkirin"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "Bernameya $(^NameDA) ji pergale hate rakirin.$\r$\n$\r$\nJi bo girtina s�rbaz 'biqed�ne'y� bitik�ne."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Ji bo rakirina bernameya $(^NameDA) biqede div� tu komputera xwe ji n� ve vek�. Tu dixwaz� niha komputera te were girtin � ji n� ve dest p� bike?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Ji n� ve veke"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Ez� pa�� ji n� ve vekim."
  ${LangFileString} MUI_TEXT_FINISH_RUN "Bernameya $(^NameDA) bixebit�ne"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Dosiya min bixw�ne/readme &n��an bide"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Biqed�ne"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Hilbijartina Peldanka P��eka Destp�k�"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Peldanka p��eka destp�k� ya ku d� kineriya $(^NameDA) t� de were bikaran�n hilbij�re."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Peldanka p��eka destp�k� ya ku d� kineriya bernamey� t� de were bicihkirin hilbij�re.  Tu dikar� bi navek� n� peldankeke n� ava bik�."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "B�y� ��kirina kineriy� bidom�ne"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Bernameya $(^NameDA) Rake"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Rakirina bernameya $(^NameDA) ji pergala te."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Tu bawer � ku dixwaz� ji sazkirina $(^Name) derkev�?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Tu bawer � ku dixwaz� dest ji rakirina bernameya $(^Name) berd�?"
!endif
