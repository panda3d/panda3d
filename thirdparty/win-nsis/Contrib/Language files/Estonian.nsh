;Language: Estonian (1061)
;Translated by johnny izzo (izzo@hot.ee)

!insertmacro LANGFILE "Estonian" "Eesti keel"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "$(^NameDA) paigaldamine!"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "See abiline aitab paigaldada $(^NameDA).$\r$\n$\r$\nEnne paigaldamise alustamist on soovitatav k�ik teised programmid sulgeda, see v�imaldab teatud s�steemifaile uuendada ilma arvutit taask�ivitamata.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "$(^NameDA) eemaldamine!"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "See abiline aitab eemaldada $(^NameDA).$\r$\n$\r$\nEnne eemaldamist vaata, et $(^NameDA) oleks suletud.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Litsentsileping"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Enne $(^NameDA) paigaldamist vaata palun litsentsileping �le."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Kui sa oled lepingu tingimustega n�us, vali j�tkamiseks N�ustun. $(^NameDA) paigaldamiseks pead sa lepinguga n�ustuma."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Kui n�ustud lepingu tingimustega, vali allolev m�rkeruut. $(^NameDA) paigaldamiseks pead lepinguga n�ustuma. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Kui n�ustud lepingu tingimustega, m�rgi allpool esimene valik. $(^NameDA) paigaldamiseks pead lepinguga n�ustuma. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Litsentsileping"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Enne $(^NameDA) eemaldamist vaata palun litsentsileping �le."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Kui sa oled lepingu tingimustega n�us, vali j�tkamiseks N�ustun. $(^NameDA) eemaldamiseks pead sa lepinguga n�ustuma."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Kui n�ustud lepingu tingimustega, vali allolev m�rkeruut. $(^NameDA) eemaldamiseks pead lepinguga n�ustuma. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Kui n�ustud lepingu tingimustega, m�rgi allpool esimene valik. $(^NameDA) eemaldamiseks pead lepinguga n�ustuma. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Vajuta Page Down, et n�ha �lej��nud teksti."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Vali komponendid"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Vali millised $(^NameDA) osad sa soovid paigaldada."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Vali komponendid"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Vali millised $(^NameDA) osad sa soovid eemaldada."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Kirjeldus"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Nihuta hiir komponendile, et n�ha selle kirjeldust."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Nihuta hiir komponendile, et n�ha selle kirjeldust."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Vali asukoht"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Vali kaust kuhu paigaldada $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Vali asukoht"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Vali kaust kust $(^NameDA) eemaldada."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Paigaldan..."
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Palun oota kuni $(^NameDA) on paigaldatud."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Programm paigaldatud"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Paigaldus edukalt sooritatud."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Paigaldus katkestatud"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Paigaldamine eba�nnestus."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Eemaldan..."
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Palun oota kuni $(^NameDA) on eemaldatud."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Eemaldamine l�petatud"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Eemaldamine edukalt l�pule viidud."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Eemaldamine katkestatud"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Eemaldamine eba�nestus."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "$(^NameDA) paigalduse l�pule viimine."
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) on sinu arvutisse paigaldatud.$\r$\n$\r$\nAbilise sulgemiseks vajuta L�peta."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "$(^NameDA) t�ielikuks paigaldamiseks tuleb arvuti taask�ivitada. Kas soovid arvuti kohe taask�ivitada ?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "$(^NameDA) eemaldamise l�pule viimine."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) on sinu arvutist eemaldatud.$\r$\n$\r$\nAbilise sulgemiseks vajuta L�peta."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "$(^NameDA) t�ielikuks eemaldamiseks tuleb arvuti taask�ivitada. Kas soovid arvuti kohe taask�ivitada ?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Taask�ivita kohe"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Taask�ivitan hiljem k�sitsi"
  ${LangFileString} MUI_TEXT_FINISH_RUN "K�ivita $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Kuva Loemind"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "L�peta"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Vali Start-men�� kaust"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Vali $(^NameDA) otseteede jaoks Start-men�� kaust."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Vali Start-men�� kaust, kuhu soovid paigutada programmi otseteed. V�id ka sisestada nime, et luua uus kaust."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "�ra loo otseteid"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Eemalda $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Eemalda $(^NameDA) oma arvutist."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Oled sa kindel et soovid $(^Name) paigaldamise katkestada?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Oled sa kindel et soovid $(^Name) eemaldamise katkestada?"
!endif
