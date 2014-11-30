;Language: Latvie�u [Latvian] - (1062)
;By Valdis Gri�is
;Corrections by Kristaps Me��elis / x-f (x-f 'AT' inbox.lv)

!insertmacro LANGFILE "Latvian" "Latvie�u"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Esiet sveicin�ti '$(^NameDA)' uzst�d��anas vedn�"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "�is uzst�d��anas vednis jums pal�dz�s veikt '$(^NameDA)' uzst�d��anu.$\r$\n$\r$\n�oti ieteicams aizv�rt citas programmas pirms ��s programmas uzst�d��anas veik�anas. Tas �aus atjaunot svar�gus sist�mas failus bez datora p�rstart��anas.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Esiet sveicin�ti '$(^NameDA)' atinstal��anas vedn�"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "�is vednis jums pal�dz�s veikt '$(^NameDA)' atinstal��anu.$\r$\n$\r$\nPirms s�kt atinstal��anas procesu, p�rliecinieties, vai '$(^NameDA)' pa�laik nedarbojas.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licences l�gums"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "L�dzu izlasiet licences l�gumu pirms '$(^NameDA)' uzst�d��anas."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Ja piekr�tat licences l�guma noteikumiem, spiediet 'Piekr�tu', lai turpin�tu uzst�d��anu. Jums ir j�piekr�t licences noteikumiem, lai uzst�d�tu '$(^NameDA)'."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ja piekr�tat licences l�guma noteikumiem, tad atz�m�jiet izv�les r�ti�u. Jums ir j�piekr�t licences noteikumiem, lai uzst�d�tu '$(^NameDA)'. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ja piekr�tat licences l�guma noteikumiem, tad izv�lieties pirmo zem�keso�o opciju. Jums ir j�piekr�t licences noteikumiem, lai uzst�d�tu '$(^NameDA)'. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licences l�gums"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "L�dzu izlasiet licences l�gumu pirms '$(^NameDA)' atinstal��anas."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Ja piekr�tat licences noteikumiem, spiediet 'Piekr�tu', lai turpin�tu. Jums ir j�piekr�t licences noteikumiem, lai atinstal�tu '$(^NameDA)'."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Ja piekr�tat licences l�guma noteikumiem, tad iez�m�jiet izv�les r�ti�u. Jums ir j�piekr�t licences noteikumiem, lai atinstal�tu '$(^NameDA)'. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Ja piekr�tat licences l�guma noteikumiem, tad izv�lieties pirmo zem�keso�o opciju. Jums ir j�piekr�t licences noteikumiem, lai atinstal�tu '$(^NameDA)'. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Spiediet 'Page Down', lai apl�kotu visu l�gumu."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Izv�lieties komponentus"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Izv�lieties nepiecie�am�s '$(^NameDA)' sast�vda�as, kuras uzst�d�t."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Izv�lieties komponentus"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Izv�lieties nepiecie�am�s '$(^NameDA)' sast�vda�as, kuras atinstal�t."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Apraksts"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Novietojiet peles kursoru uz komponenta, lai tiktu par�d�ts t� apraksts."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Novietojiet peles kursoru uz komponenta, lai tiktu par�d�ts t� apraksts."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Izv�lieties uzst�d��anas mapi"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Izv�lieties mapi, kur� uzst�d�t '$(^NameDA)'."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Izv�lieties atinstal��anas mapi"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Izv�lieties mapi, no kuras notiks '$(^NameDA)' atinstal��ana."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Notiek uzst�d��ana"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "L�dzu uzgaidiet, kam�r notiek '$(^NameDA)' uzst�d��ana."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Uzst�d��ana pabeigta"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Uzst�d��ana norit�ja veiksm�gi."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Uzst�d��ana atcelta"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Uzst�d��ana nenorit�ja veiksm�gi."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Notiek atinstal��ana"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "L�dzu uzgaidiet, kam�r '$(^NameDA)' tiek atinstal�ta."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Atinstal��ana pabeigta"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Atinstal��ana norit�ja veiksm�gi."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Atinstal��ana atcelta"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Atinstal��ana nenorit�ja veiksm�gi."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Tiek pabeigta '$(^NameDA)' uzst�d��ana"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "'$(^NameDA)' tika veiksm�gi uzst�d�ta j�su dator�.$\r$\n$\r$\nNospiediet 'Pabeigt', lai aizv�rtu vedni."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "J�su datoru ir nepiecie�ams p�rstart�t, lai pabeigtu '$(^NameDA)' uzst�d��anu. Vai v�laties p�rstart�t datoru t�l�t?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Tiek pabeigta '$(^NameDA)' atinstal�cija"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "'$(^NameDA)' tika veiksm�gi izdz�sta no j�su datora.$\r$\n$\r$\nNospiediet 'Pabeigt', lai aizv�rtu vedni."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "J�su datoru nepiecie�ams p�rstart�t, lai pabeigtu '$(^NameDA)' atinstal��anu. Vai v�laties p�rstart�t datoru t�l�t?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "P�rstart�t t�l�t"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Es v�los p�rstart�t pats v�l�k"
  ${LangFileString} MUI_TEXT_FINISH_RUN "P&alaist '$(^NameDA)'"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Pa&r�d�t LasiMani failu"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Pabeigt"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Izv�lieties 'Start Menu' folderi"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Izv�lieties 'Start Menu' mapi '$(^NameDA)' sa�sn�m."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Izv�lieties 'Start Menu' mapi, kur� tiks izveidotas programmas sa�snes. Varat ar� pats izveidot jaunu mapi."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Neveidot sa�snes"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "'$(^NameDA)' atinstal��ana"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Dz�st '$(^NameDA)' no j�su datora."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Vai tie��m v�laties p�rtraukt '$(^Name)' uzst�d��anu?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Vai tie��m v�laties p�rtraukt '$(^Name)' atinstal��anu?"
!endif
