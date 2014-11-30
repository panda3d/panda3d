;Language: Polish (1045)
;By Piotr Murawski & Rafa� Lampe
;Updated by cube and SYSTEMsoft Group
;Updated by Pawe� Porwisz, http://www.pepesoft.tox.pl
;Corrected by Mateusz Gola (aka Prozac) - http://www.videopedia.pl/avirecomp

!insertmacro LANGFILE "Polish" "Polski"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Witamy w kreatorze instalacji programu $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Kreator ten pomo�e Ci zainstalowa� program $(^NameDA).$\r$\n$\r$\nZalecane jest zamkni�cie wszystkich uruchomionych program�w przed rozpocz�ciem instalacji. Pozwoli to na uaktualnienie niezb�dnych plik�w systemowych bez konieczno�ci ponownego uruchamiania komputera.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Witamy w kreatorze deinstalacji $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Kreator poprowadzi Ci� przez proces deinstalacji $(^NameDA).$\r$\n$\r$\nPrzed rozpocz�ciem deinstalacji programu, upewnij si�, czy $(^NameDA) NIE jest w�a�nie uruchomiony.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Umowa licencyjna"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Przed instalacj� programu $(^NameDA) zapoznaj si� z warunkami licencji."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Je�eli akceptujesz warunki umowy, wybierz Zgadzam si�, aby kontynuowa�. Musisz zaakceptowa� warunki umowy, aby zainstalowa� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Je�eli akceptujesz warunki umowy, zaznacz pole wyboru poni�ej, aby kontynuowa�. Musisz zaakceptowa� warunki umowy, aby zainstalowa� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Je�eli akceptujesz warunki umowy, wybierz pierwsz� opcj� poni�ej, aby kontynuowa�. Musisz zaakceptowa� warunki umowy, aby zainstalowa� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Umowa licencyjna"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Przed deinstalacj� programu $(^NameDA) zapoznaj si� z warunkami licencji."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Je�eli akceptujesz warunki umowy, wybierz Zgadzam si�, aby kontynuowa�. Musisz zaakceptowa� warunki umowy, aby odinstalowa� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Je�eli akceptujesz warunki umowy, zaznacz pole wyboru poni�ej, aby kontynuowa�. Musisz zaakceptowa� warunki umowy, aby odinstalowa� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Je�eli akceptujesz warunki umowy, wybierz pierwsz� opcj� poni�ej, aby kontynuowa�. Musisz zaakceptowa� warunki umowy, aby odinstalowa� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Naci�nij klawisz Page Down, aby zobaczy� reszt� umowy."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Wybierz komponenty"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Wybierz komponenty programu $(^NameDA), kt�re chcesz zainstalowa�."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Wybierz komponenty"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Wybierz, kt�re elementy $(^NameDA) chcesz odinstalowa�."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Opis"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Przesu� kursor myszy nad komponent, aby zobaczy� jego opis."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Przesu� kursor myszy nad komponent, aby zobaczy� jego opis."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Wybierz lokalizacj� dla instalacji"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Wybierz folder, w kt�rym ma by� zainstalowany $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Wyb�r miejsca deinstalacji"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Wybierz folder, z kt�rego chcesz odinstalowa� $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instalacja"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Prosz� czeka�, podczas gdy $(^NameDA) jest instalowany."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Zako�czono instalacj�"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Instalacja zako�czona pomy�lnie."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalacja przerwana"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Instalacja nie zosta�a zako�czona pomy�lnie."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Deinstalacja"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Prosz� czeka�, $(^NameDA) jest odinstalowywany."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Zako�czono odinstalowanie"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Odinstalowanie zako�czone pomy�lnie."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Deinstalacja przerwana"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Deinstalacja nie zosta�a zako�czona pomy�lnie."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Ko�czenie pracy kreatora instalacji $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) zosta� pomy�lnie zainstalowany na Twoim komputerze.$\r$\n$\r$\nKliknij Zako�cz, aby zako�czy� dzia�anie kreatora."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Tw�j komputer musi zosta� ponownie uruchomiony, aby zako�czy� instalacj� programu $(^NameDA). Czy chcesz zrobi� to teraz?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Ko�czenie pracy kreatora deinstalacji $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) zosta� odinstalowany z Twojego komputera.$\r$\n$\r$\nKliknij Zako�cz, aby zako�czy� dzia�anie kreatora."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Tw�j komputer musi zosta� ponownie uruchomiony w celu zako�czenia deinstalacji programu $(^NameDA). Czy chcesz zrobi� to teraz?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Uruchom ponownie teraz"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Sam uruchomi� ponownie komputer p�niej"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Uruchom program $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Poka� plik ReadMe"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Zako�cz"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Wybierz folder w menu Start"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Wybierz folder menu Start, w kt�rym zostan� umieszczone skr�ty do programu"
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Wybierz folder w menu Start, w kt�rym chcia�by� umie�ci� skr�ty do programu. Mo�esz tak�e utworzy� nowy folder wpisuj�c jego nazw�."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nie tw�rz skr�t�w"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Odinstaluj $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Usu� $(^NameDA) z Twojego komputera."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Czy na pewno chcesz zako�czy� dzia�anie instalatora $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Czy na pewno chcesz przerwa� proces deinstalacji $(^Name)?"
!endif

!ifdef MULTIUSER_INSTALLMODEPAGE
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_TITLE "Wybierz u�ytkownik�w"
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_SUBTITLE "Wybierz, dla kt�rych u�ytkownik�w chcesz zainstalowa� program $(^NameDA)."
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_TOP "Wybierz, czy chcesz zainstalowa� program $(^NameDA) tylko dla siebie, czy dla wszystkich u�ytkownik�w tego komputera. $(^ClickNext)"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS "Zainstaluj dla wszystkich u�ytkownik�w tego komputera"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER "Zainstaluj tylko dla mnie"
!endif
