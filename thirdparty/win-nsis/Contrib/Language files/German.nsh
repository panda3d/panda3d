;Language: German (1031)
;By L.King, changes by K. Windszus & R. Bisswanger & M. Simmack & D. Wei�

!insertmacro LANGFILE "German" "Deutsch"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Willkommen beim Installations-$\r$\nAssistenten f�r $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Dieser Assistent wird Sie durch die Installation von $(^NameDA) begleiten.$\r$\n$\r$\nEs wird empfohlen, vor der Installation alle anderen Programme zu schlie�en, damit bestimmte Systemdateien ohne Neustart ersetzt werden k�nnen.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Willkommen beim Deinstallations-$\r$\nAssistenten f�r $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Dieser Assistent wird Sie durch die Deinstallation von $(^NameDA) begleiten.$\r$\n$\r$\nBitte beenden Sie $(^NameDA), bevor Sie mit der Deinstallation fortfahren.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Lizenzabkommen"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Bitte lesen Sie die Lizenzbedingungen durch, bevor Sie mit der Installation fortfahren."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Falls Sie alle Bedingungen des Abkommens akzeptieren, klicken Sie auf Annehmen. Sie m�ssen die Lizenzvereinbarungen anerkennen, um $(^NameDA) installieren zu k�nnen."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Falls Sie alle Bedingungen des Abkommens akzeptieren, aktivieren Sie das K�stchen. Sie m�ssen die Lizenzvereinbarungen anerkennen, um $(^NameDA) installieren zu k�nnen. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Falls Sie alle Bedingungen des Abkommens akzeptieren, w�hlen Sie unten die entsprechende Option. Sie m�ssen die Lizenzvereinbarungen anerkennen, um $(^NameDA) installieren zu k�nnen. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Lizenzabkommen"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Bitte lesen Sie die Lizenzbedingungen durch, bevor Sie mit der Deinstallation von $(^NameDA) fortfahren."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Falls Sie alle Bedingungen des Abkommens akzeptieren, klicken Sie auf Annehmen. Sie m�ssen die Lizenzvereinbarungen anerkennen, um $(^NameDA) deinstallieren zu k�nnen."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Falls Sie alle Bedingungen des Abkommens akzeptieren, aktivieren Sie das K�stchen. Sie m�ssen die Lizenzvereinbarungen anerkennen, um $(^NameDA) deinstallieren zu k�nnen. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Falls Sie alle Bedingungen des Abkommens akzeptieren, w�hlen Sie unten die entsprechende Option. Sie m�ssen die Lizenzvereinbarungen anerkennen, um $(^NameDA) deinstallieren zu k�nnen. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Dr�cken Sie die Bild-nach-unten Taste, um den Rest des Abkommens zu sehen."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Komponenten ausw�hlen"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "W�hlen Sie die Komponenten aus, die Sie installieren m�chten."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Komponenten ausw�hlen"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "W�hlen Sie die Komponenten aus, die Sie entfernen m�chten."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Beschreibung"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Bewegen Sie den Mauszeiger �ber eine Komponente, um ihre Beschreibung zu sehen."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Bewegen Sie den Mauszeiger �ber eine Komponente, um ihre Beschreibung zu sehen."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Zielverzeichnis ausw�hlen"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "W�hlen Sie das Verzeichnis aus, in das $(^NameDA) installiert werden soll."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Verzeichnis f�r Deinstallation ausw�hlen"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "W�hlen Sie das Verzeichnis aus, aus dem $(^NameDA) entfernt werden soll."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Installiere..."
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Bitte warten Sie, w�hrend $(^NameDA) installiert wird."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Die Installation ist vollst�ndig"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Die Installation wurde erfolgreich abgeschlossen."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Abbruch der Installation"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Die Installation wurde nicht vollst�ndig abgeschlossen."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Deinstalliere..."
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Bitte warten Sie, w�hrend $(^NameDA) entfernt wird."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Die Deinstallation ist vollst�ndig"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Die Deinstallation wurde erfolgreich abgeschlossen."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Abbruch der Deinstallation"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Die Deinstallation wurde nicht vollst�ndig abgeschlossen."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Die Installation von $(^NameDA) wird abgeschlossen"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) wurde auf Ihrem Computer installiert.$\r$\n$\r$\nKlicken Sie auf Fertig stellen, um den Installations-Assistenten zu schlie�en."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Windows muss neu gestartet werden, um die Installation von $(^NameDA) zu vervollst�ndigen. M�chten Sie Windows jetzt neu starten?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Die Deinstallation von $(^NameDA) wird abgeschlossen"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) ist von Ihrem Computer entfernt worden.$\r$\n$\r$\nKlicken Sie auf Fertig stellen, um den Assistenten zu schlie�en."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Windows muss neu gestartet werden, um die Deinstallation von $(^NameDA) zu vervollst�ndigen. M�chten Sie Windows jetzt neu starten?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Jetzt neu starten"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Windows sp�ter selbst neu starten"
  ${LangFileString} MUI_TEXT_FINISH_RUN "$(^NameDA) ausf�hren"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Liesmich anzeigen"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Fertig stellen"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Startmen�-Ordner bestimmen"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Bestimmen Sie einen Startmen�-Ordner f�r die Programmverkn�pfungen."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "W�hlen Sie den Startmen�-Ordner f�r die Programmverkn�pfungen aus. Falls Sie einen neuen Ordner erstellen m�chten, geben Sie dessen Namen ein."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Keine Verkn�pfungen erstellen"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Deinstallation von $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "$(^NameDA) wird von Ihrem Computer entfernt."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Sind Sie sicher, dass Sie die Installation von $(^Name) abbrechen wollen?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Sind Sie sicher, dass Sie die Deinstallation von $(^Name) abbrechen m�chten?"
!endif

!ifdef MULTIUSER_INSTALLMODEPAGE
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_TITLE "Benutzer ausw�hlen"
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_SUBTITLE "W�hlen Sie die Benutzer aus, f�r die Sie $(^NameDA) installieren wollen."
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_TOP "W�hlen Sie aus, ob Sie $(^NameDA) nur f�r den eigenen Gebrauch oder f�r die Nutzung durch alle Benutzer dieses Systems installieren m�chten. $(^ClickNext)"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS "Installation f�r alle Benutzer dieses Computers"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER "Installation nur f�r mich"
!endif
