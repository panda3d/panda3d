;Language: Catalan (1027)
;By falanko, corrections by Toni Hermoso Pulido

!insertmacro LANGFILE "Catalan" "Catal�"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Benvinguts a l'auxiliar d'instal�laci� de l'aplicaci� $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Aquest auxiliar us guiar� durant el proc�s d'instal�laci� de l'aplicaci� $(^NameDA).$\r$\n$\r$\nEs recomana tancar la resta d'aplicacions abans de comen�ar la instal�laci�. Aix� permetr� al programa d'instal�aci� actualitzar fitxers del sistema rellevants sense haver de reiniciar l'ordinador.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Benvinguts a l'auxiliar de desinstal�laci� de l'aplicaci� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Aquest auxiliar us guiar� a trav�s de la desinstal�laci� de l'aplicaci� $(^NameDA).$\r$\n\rAbans de comen�ar la desinstal�laci�, assegureu-vos que l'aplicaci� $(^NameDA) no s'est� executant.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Acord de Llic�ncia"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Reviseu els termes de la llic�ncia abans d'instal�lar l'aplicaci� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Si accepteu tots els termes de l'acord, premeu Hi estic d'acord per a continuar. Heu d'acceptar l'acord per a poder instal�lar l'aplicaci� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Si accepteu tots els termes de l'acord, activeu la casella de sota. Heu d'acceptar l'acord per poder instal�lar l'aplicaci� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Si accepteu tots els termes de l'acord, seleccioneu la primera opci� de sota. Heu d'acceptar l'acord per a poder instal�lar $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Acord de llic�ncia"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Reviseu els termes de la llic�ncia abans de desinstal�lar l'aplicaci� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Si accepteu tots els termes de l'acord, premeu Hi estic d'Acord per a continuar. Heu d'acceptar l'acord per a poder desinstal�lar l'aplicaci� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Si accepteu tots els termes de l'acord, activeu la casella de sota. Heu d'acceptar l'acord per a poder desinstal�lar l'aplicaci� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Si accepteu tots els termes de l'acord, seleccioneu la primera opci� de sota. Heu d'acceptar l'acord per a poder desinstal�lar l'aplicaci� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Premeu AvP�g per a veure la resta de l'acord."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Selecci� de components"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Seleccioneu quines caracter�stiques de l'aplicaci� $(^NameDA) desitgeu instal�lar."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Selecci� de components"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Seleccioneu quines caracter�stiques de l'aplicaci� $(^NameDA) desitgeu desinstal�lar."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Descripci�"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Situeu el ratol� damunt d'un component per a veure'n la descripci�."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Situeu el ratol� damunt d'un component per a veure'n la descripci�."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Trieu una ubicaci� d'instal�laci�"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Trieu la carpeta on instal�lar-hi l'aplicaci� $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Trieu la ubicaci� de desinstal�laci�"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Trieu la carpeta d'on desinstal�lar l'aplicaci� $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "S'est� instal�lant"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Si us plau, espereu mentre l'aplicaci� $(^NameDA) s'instal�la."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "S'ha acabat la instal�laci�"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "La instal�laci� ha acabat correctament."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "S'ha abandonat la instal�laci�"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "La instal�laci� no ha acabat correctament."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "S'est� desinstal�lant"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Si us plau, espereu mentre l'aplicaci� $(^NameDA) es desinstal�la."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "S'ha acabat la desinstal�laci�"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "La desinstal�laci� s'ha realitzat correctament."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "S'ha abandonat la desinstal�laci�"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "La desinstal�laci� no ha acabat correctament."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "S'est� acabant l'auxiliar d'instal�laci� de l'aplicaci� $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "L'aplicaci� $(^NameDA) s'ha instal�lat a l'ordinador.$\r$\n$\r$\nFeu clic a Finalitza per a tancar aquest auxiliar."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Cal reiniciar l'ordinador perqu� pugui acabar-se la instal�laci� de l'aplicaci� $(^NameDA). Voleu reiniciar-lo ara?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "S'est� acabant l'auxiliar de desinstal�laci� de l'aplicaci� $(^NameDA)."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "L'aplicaci� $(^NameDA) s'ha desinstal�lat de l'ordinador.$\r$\n$\r$\nFeu clic a Finalitza per a tancar aquest auxiliar."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Cal reiniciar l'ordinador perqu� pugui acabar-se la desinstal�laci� de l'aplicaci� $(^NameDA). Voleu reiniciar-lo ara?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Reinicia ara"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Reinicia m�s tard manualment"
  ${LangFileString} MUI_TEXT_FINISH_RUN "Executa l'aplicaci� $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Mostra el Llegeix-me"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Finalitza"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Tria la carpeta del men� Inicia"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Tria una carpeta del men� Inicia per a les dreceres de l'aplicaci� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Seleccioneu la carpeta del Men� Inicia en la que hi vulgueu crear les dreceres del programa. Podeu introduir-hi un altre nom si voleu crear una carpeta nova."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "No cre�s les dreceres"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Desinstal�la l'aplicaci� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Suprimeix l'aplicaci� $(^NameDA) de l'ordinador."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Esteu segur que voleu sortir del programa d'instal�laci� de l'aplicaci� $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Esteu segur que voleu sortir del programa de desinstal�laci� de l'aplicaci� $(^Name)?"
!endif
