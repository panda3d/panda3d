;Language: Irish (2108)
;By Kevin P. Scannell < scannell at slu dot edu >

!insertmacro LANGFILE "Irish" "Irish"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "F�ilte go dt� Draoi Suite�la $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "D�anfaidh an draoi seo treor� duit tr�d an suite�il de $(^NameDA).$\r$\n$\r$\nMoltar duit gach feidhmchl�r eile a dh�nadh sula dtosa�onn t� an Suite�la�. Cinnteoidh s� seo gur f�idir na comhaid oiri�nacha a nuashonr� gan do r�omhaire a atos�.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "F�ilte go dt� Draoi D�shuite�la $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "D�anfaidh an draoi seo treor� duit tr�d an d�shuite�il de $(^NameDA).$\r$\n$\r$\nB� cinnte nach bhfuil $(^NameDA) ag rith sula dtosa�onn t� an d�shuite�il.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Comhaont� um Chead�nas"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "D�an ini�chadh ar choinn�ollacha an chead�nais sula suite�lann t� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "M� ghlacann t� le coinn�ollacha an chomhaontaithe, clice�il $\"Glacaim Leis$\" chun lean�int ar aghaidh. Caithfidh t� glacadh leis an gcomhaont� chun $(^NameDA) a shuite�il."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "M� ghlacann t� le coinn�ollacha an chomhaontaithe, clice�il an ticbhosca th�os. Caithfidh t� glacadh leis an gcomhaont� chun $(^NameDA) a shuite�il. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "M� ghlacann t� le coinn�ollacha an chomhaontaithe, roghnaigh an ch�ad rogha th�os. Caithfidh t� glacadh leis an gcomhaont� chun $(^NameDA) a dh�shuite�il. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Comhaont� um Chead�nas"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "D�an ini�chadh ar choinn�ollacha an chead�nais sula nd�shuite�lann t� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "M� ghlacann t� le coinn�ollacha an chomhaontaithe, clice�il $\"Glacaim Leis$\" chun lean�int ar aghaidh. Caithfidh t� glacadh leis an gcomhaont� chun $(^NameDA) a dh�shuite�il."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "M� ghlacann t� le coinn�ollacha an chomhaontaithe, clice�il an ticbhosca th�os. Caithfidh t� glacadh leis an gcomhaont� chun $(^NameDA) a dh�shuite�il. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "M� ghlacann t� le coinn�ollacha an chomhaontaithe, roghnaigh an ch�ad rogha th�os. Caithfidh t� glacadh leis an gcomhaont� chun $(^NameDA) a dh�shuite�il. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Br�igh $\"Page Down$\" chun an chuid eile den chead�nas a l�amh."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Roghnaigh Comhph�irteanna"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Roghnaigh na gn�ithe $(^NameDA) ba mhaith leat suite�il."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Roghnaigh Comhph�irteanna"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Roghnaigh na gn�ithe $(^NameDA) ba mhaith leat d�shuite�il."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Cur S�os"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Cuir do luch os cionn comhph�irte chun cur s�os a fheice�il."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Cuir do luch os cionn comhph�irte chun cur s�os a fheice�il."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Roghnaigh Su�omh na Suite�la"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Roghnaigh an fillte�n inar mian leat $(^NameDA) a shuite�il."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Roghnaigh Su�omh na D�shuite�la"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Roghnaigh an fillte�n ar mian leat $(^NameDA) a dh�shuite�il as."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "� Shuite�il"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Fan go f�ill; $(^NameDA) � shuite�il."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Suite�il Cr�ochnaithe"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "D'�irigh leis an tsuite�il."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Suite�il Tobscortha"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "N�or �irigh leis an tsuite�il."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "� Dh�shuite�il"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Fan go f�ill; $(^NameDA) � dh�shuite�il."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "D�shuite�il Cr�ochnaithe"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "D'�irigh leis an d�shuite�il."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "D�shuite�il Tobscortha"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "N�or �irigh leis an d�shuite�il."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Draoi Suite�la $(^NameDA) � Chr�ochn�"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Suite�ladh $(^NameDA) ar do r�omhaire.$\r$\n$\r$\nClice�il $\"Cr�ochnaigh$\" chun an draoi seo a dh�nadh."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "N� m�r duit do r�omhaire a atos� chun suite�il $(^NameDA) a chur i gcr�ch. Ar mhaith leat atos� anois?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Draoi D�shuite�la $(^NameDA) � Chr�ochn�"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "D�shuite�ladh $(^NameDA) � do r�omhaire.$\r$\n$\r$\nClice�il $\"Cr�ochnaigh$\" chun an draoi seo a dh�nadh."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "N� m�r duit do r�omhaire a atos� chun d�shuite�il $(^NameDA) a chur i gcr�ch. Ar mhaith leat atos� anois?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Atosaigh anois"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Atos�idh m� de l�imh n�os d�ana�"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Rith $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "Tai&spe�in comhad README"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Cr�ochnaigh"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Roghnaigh Fillte�n sa Roghchl�r Tosaigh"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Roghnaigh fillte�n sa Roghchl�r Tosaigh a gcuirfear aicearra� $(^NameDA) ann."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Roghnaigh an fillte�n sa Roghchl�r Tosaigh inar mian leat aicearra� an chl�ir a chruth�. Is f�idir freisin fillte�n nua a chruth� tr� ainm nua a iontr�il."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "N� cruthaigh aicearra�"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "D�shuite�il $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Bain $(^NameDA) � do r�omhaire."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "An bhfuil t� cinnte gur mian leat Suite�la� $(^Name) a scor?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "An bhfuil t� cinnte gur mian leat D�shuite�la� $(^Name) a scor?"
!endif
