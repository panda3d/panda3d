;Language: Brazilian Portuguese (1046)
;By Felipe

!insertmacro LANGFILE "PortugueseBR" "Portugu�s Brasileiro"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Bem-vindo ao Assistente de Instala��o do $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Este assistente guiar� voc� atrav�s da instala��o do $(^NameDA).$\r$\n$\r$\n� recomendado que voc� feche todos os outros aplicativos antes de iniciar o Instalador. Isto tornar� poss�vel atualizar os arquivos de sistema relevantes sem ter que reiniciar seu computador.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Bem-vindo ao Assistente de Desinstala��o do $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Este assistente guiar� voc� atrav�s da desinstala��o do $(^NameDA).$\r$\n$\r$\nAntes de iniciar a desinstala��o, tenha certeza de que o $(^NameDA) n�o est� em execu��o.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Acordo da licen�a"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Por favor, reveja os termos da licen�a antes de instalar o $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Se voc� aceita os termos do acordo, clique em Eu Concordo para continuar. Voc� deve aceitar o acordo para instalar o $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Se voc� aceita os termos do acordo, clique na caixa de sele��o abaixo. Voc� deve aceitar o acordo para instalar o $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Se voc� aceita os termos do acordo, selecione a primeira op��o abaixo. Voc� deve aceitar o acordo para instalar o $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Acordo da licen�a"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Por favor, reveja os termos da licen�a antes de desinstalar o $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Se voc� aceita os termos do acordo, clique em Eu Concordo para continuar. Voc� deve aceitar o acordo para desinstalar o $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Se voc� aceita os termos do acordo, clique na caixa de sele��o abaixo. Voc� deve aceitar o acordo para desinstalar o $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Se voc� aceita os termos do acordo, selecione a primeira op��o abaixo. Voc� deve aceitar o acordo para desinstalar o $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Pressione Page Down para ver o resto do acordo."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Escolher Componentes"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Escolha quais fun��es do $(^NameDA) voc� quer instalar."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Escolher Componentes"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Escolha quais fun��es do $(^NameDA) voc� quer desinstalar."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Descri��o"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Posicione seu mouse sobre um componente para ver sua descri��o."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Posicione seu mouse sobre um componente para ver sua descri��o."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Escolher o Local da Instala��o"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Escolha a pasta na qual instalar o $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Escolher o Local da Desinstala��o"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Escolha a pasta da qual desinstalar o $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Instalando"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Por favor espere enquanto o $(^NameDA) est� sendo instalado."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Instala��o Completa"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "O Instalador completou com sucesso."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instala��o Abortada"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "O Instalador n�o completou com sucesso."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Desinstalando"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Por favor espere enquanto o $(^NameDA) est� sendo desinstalado."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Desinstala��o Completa"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "A desinstala��o foi completada com sucesso."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Desinstala��o Abortada"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "A desinstala��o n�o foi completada com sucesso."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Completando o Assistente de Instala��o do $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "O $(^NameDA) foi instalado no seu computador.$\r$\n$\r$\nClique em Terminar para fechar este assistente."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Seu computador deve ser reiniciado para completar a instala��o do $(^NameDA). Voc� quer reiniciar agora?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Completando o Assistente de Desinstala��o do $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) foi desinstalado do seu computador.$\r$\n$\r$\nClique em Terminar para fechar este assistente."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Seu computador tem que ser reiniciado para completar a desinstala��o do $(^NameDA). Voc� quer reiniciar agora?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Reiniciar agora"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Eu quero reiniciar manualmente depois"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Executar $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Mostrar o Readme"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Terminar"
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Escolher a Pasta do Menu Iniciar"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Escolher uma pasta do Menu Iniciar para os atalhos do $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Selecione a pasta do Menu Iniciar na qual voc� gostaria de criar os atalhos do programa. Voc� pode tamb�m inserir um nome para criar uma nova pasta."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "N�o criar atalhos"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Desinstalar o $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Remover o $(^NameDA) do seu computador."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Voc� tem certeza de que quer sair do Instalador do $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Voc� tem certeza de que quer sair da Desinstala��o do $(^Name)?"
!endif

!ifdef MULTIUSER_INSTALLMODEPAGE
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_TITLE "Escolher Usu�rios"
  ${LangFileString} MULTIUSER_TEXT_INSTALLMODE_SUBTITLE "Escolher para quais usu�rios voc� quer instalar o $(^NameDA)."
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_TOP "Selecione se voc� quer instalar o $(^NameDA) para si mesmo ou para todos os usu�rios deste computador. $(^ClickNext)"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS "Instalar para qualquer um usando este computador"
  ${LangFileString} MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER "Instalar apenas para mim"
!endif
