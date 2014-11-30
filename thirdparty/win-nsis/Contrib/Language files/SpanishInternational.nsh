;Language: Spanish International (3082)
;By Darwin Rodrigo Toledo C�ceres - www.winamp-es.com - niwrad777@gmail.com
;Base by Monki y Joel

!insertmacro LANGFILE "SpanishInternational" "Espa�ol (Alfabetizaci�n Internacional)"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Bienvenido al Asistente de Instalaci�n de $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Este asistente le guiar� a trav�s de la instalaci�n de $(^NameDA).$\r$\n$\r$\nSe recomienda que cierre todas la dem�s aplicaciones antes de iniciar la instalaci�n. Esto har� posible actualizar archivos de sistema sin tener que reiniciar su computadora.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Bienvenido al Asistente de Desinstalaci�n de $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Este asistente le guiar� durante la desinstalaci�n de $(^NameDA).$\r$\n$\r$\nAntes de iniciar la desinstalaci�n, aseg�rese de que $(^NameDA) no se est� ejecutando.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Acuerdo de licencia"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Por favor revise el acuerdo de licencia antes de instalar $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Si acepta todas las condiciones del acuerdo, seleccione Acepto para continuar. Debe aceptar el acuerdo para instalar $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Si acepta las condiciones del acuerdo, marque abajo la casilla. Debe aceptar las condiciones para instalar $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Si acepta las condiciones del acuerdo, seleccione abajo la primera opci�n. Debe aceptar las condiciones para instalar $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Acuerdo de licencia"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Por favor revise el acuerdo de licencia antes de desinstalar $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Si acepta todas las condiciones del acuerdo, seleccione Acepto para continuar. Debe aceptar el acuerdo para desinstalar $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Si acepta los t�rminos del acuerdo, marque abajo la casilla. Debe aceptar los t�rminos para desinstalar $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Si acepta los t�rminos del acuerdo, seleccione abajo la primera opci�n. Debe aceptar los t�rminos para desinstalar $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Presione Avanzar P�gina para ver el resto del acuerdo."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Selecci�n de componentes"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Seleccione qu� caracter�sticas de $(^NameDA) desea instalar."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Selecci�n de componentes"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Seleccione qu� caracter�sticas de $(^NameDA) desea desinstalar."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Descripci�n"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Sit�e el rat�n encima de un componente para ver su descripci�n."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Sit�e el rat�n encima de un componente para ver su descripci�n."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Elegir lugar de instalaci�n"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Elija la carpeta para instalar $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Elegir lugar de desinstalaci�n"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Elija la carpeta desde la cual desinstalar� $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instalando"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Por favor espere mientras $(^NameDA) se instala."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Instalaci�n Finalizada"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "La instalaci�n se ha finalizado correctamente."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalaci�n Abortada"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "La instalaci�n no se termin� correctamente."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Desinstalando"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Por favor espere mientras $(^NameDA) se desinstala."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Desinstalaci�n Finalizada"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "La desinstalaci�n se ha finalizado correctamente."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Desinstalaci�n Abortada"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "La desinstalaci�n no se termin� correctamente."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Finalizando el Asistente de Instalaci�n de $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) ha sido instalado en su sistema.$\r$\n$\r$\nPresione Terminar para cerrar este asistente."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Su sistema debe ser reiniciado para poder finalizar la instalaci�n de $(^NameDA). �Desea reiniciar ahora?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Completando el Asistente de Desinstalaci�n de $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) ha sido desinstalado de su sistema.$\r$\n$\r$\nPresione Terminar para cerrar este asistente."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Su computadora debe ser reiniciada para finalizar la desinstalaci�n de $(^NameDA). �Desea reiniciar ahora?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Reiniciar ahora"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Deseo reiniciar manualmente m�s tarde"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Ejecutar $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Mostrar L�ame"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Terminar"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Elegir Carpeta del Men� Inicio"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Elija una Carpeta del Men� Inicio para los accesos directos de $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Seleccione una carpeta del Men� Inicio en la que quiera crear los accesos directos del programa. Tambi�n puede introducir un nombre para crear una nueva carpeta."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "No crear accesos directos"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Desinstalar $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Elimina $(^NameDA) de su sistema."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "�Est� seguro de que desea salir de la instalaci�n de $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "�Est� seguro de que desea salir de la desinstalaci�n de $(^Name)?"
!endif

!ifdef MULTIUSER_INSTALLMODEPAGE
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_TITLE "Elegir Usuarios"
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_SUBTITLE "Elija los usuarios para los cuales Ud. desea instalar $(^NameDA)."
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_TOP "Elija una opci�n si desea instalar $(^NameDA) para s�lo para Ud., o para todos los usuarios de esta computadora.$(^ClickNext)"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS "Instaci�n para cualquier usuario de esta computadora"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER "Instalaci�n solo para m�"
!endif
