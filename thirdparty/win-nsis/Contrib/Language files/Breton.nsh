;Language: Breton (1150)
;By KAD-Korvigello� An Drouizig

!insertmacro LANGFILE "Breton" "Brezhoneg"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Degemer mat e skoazeller stalia� $(^NameDA)"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Sturiet e viot gant ar skoazeller-ma� evit stalia� $(^NameDA).$\r$\n$\r$\nGwelloc'h eo serri� pep arload oberiant er reizhiad a-raok mont pelloc'h gant ar skoazeller-ma�. Evel-se e c'heller nevesaat ar restro� reizhiad hep rankout adloc'ha� hoc'h urzhiataer.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Degemer mat er skoazeller distalia� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Sturiet e viot gant ar skoazeller-ma� a-benn distalia� $(^NameDA).$\r$\n$\r$\nEn em asurit n'eo ket la�set $(^NameDA) a-raok mont pelloc'h gant an distalia�.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "La�vaz emglev"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Bezit aketus en ur lenn pep term eus al la�vaz a-raok stalia� $(^NameDA), mar plij."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Mar degemerit pep term eus al la�vaz, klikit war � War-lerc'h �. Ret eo deoc'h degemer al la�vaz evit stalia� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Mar degemerit pep term eus al la�vaz, klikit war al log a-zindan. Ret eo deoc'h degemer al la�vaz a-benn stalia� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Mar degemerit pep term eus al la�vaz, diuzit an dibab kenta� a-zindan. Ret eo deoc'h degemer al la�vaz a-benn stalia� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "La�vaz emglev"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Trugarez da lenn al la�vaz a-raok distalia� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Mar degemerit pep term eus al la�vaz, klikit war � A-du emaon � evit kenderc'hel. Ret eo deoc'h degemer al la�vaz evit distalia� $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Mar degemerit pep term eus al la�vaz, klikit war al log a-zindan. Ret eo deoc'h degemer al la�vaz evit distalia� $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Mar degemerit pep term eus al la�vaz, dizuit an dibab kenta� a-zindan. Ret eo deoc'h degemer al la�vaz evit distalia� $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Pouezit war � Pajenn a-raok � evit lenn ar pajenno� eus al la�vaz da-heul."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Dibab elfenno�"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Dibabit pe elfenn(o�) $(^NameDA) a fell deoc'h stalia�."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Dibabit elfenno�"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Dibabit pe elfenn(o�) $(^NameDA) a fell deoc'h distalia�."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Deskrivadenn"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Lakait ho logodenn a-zioc'h an elfenn evit gwelout he deskrivadenn."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Lakait ho logodenn a-zioc'h an elfenn evit gwelout he deskrivadenn."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Dibabit al lec'hiadur stalia�"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Dibabit ar c'havlec'h ma vo lakaet $(^NameDA) enna�."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Dibabit al lec'hiadur distalia�"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Dibabit ar c'havlec'h e vo dilamet $(^NameDA) diouta�."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "O stalia�"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Gortozit mar plij, ema� $(^NameDA) o veza� staliet."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Echu eo gant ar stalia�"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Kaset eo bet da benn mat ar stalia�."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Stalia� paouezet"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "N'eo ket bet kaset da benn mat ar stalia�."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "O tistalia�"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Gortozit mar plij, ema� $(^NameDA) o veza� distaliet."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Echu eo gant an distalia�"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Gant berzh eo bet kaset da benn an distalia�."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Distalia� paouezet"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "N'eo ket bet kaset da benn mat an distalia�."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Oc'h echui� stalia� $(^NameDA) gant ar skoazeller"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "Staliet eo bet $(^NameDA) war hoc'h urzhiataer.$\r$\n$\r$\nKlikit war � Echui� � evit serri� ar skoazeller-ma�."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Ret eo hoc'h urzhiataer beza� adloc'het evit ma vez kaset da benn stalia� $(^NameDA). Ha fellout a ra deoc'h adloc'ha� diouzhtu ?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Oc'h echui� distalia� $(^NameDA) gant ar skoazeller"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "Distaliet eo bet $(^NameDA) diouzh hoc'h urzhiataer.$\r$\n$\r$\nKlikit war � Echui� � evit serri� ar skoazeller-ma�."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Ret eo hoc'h urzhiataer beza� adloc'het evit ma vez kaset da benn distalia� $(^NameDA). Ha fellout a ra deoc'h adloc'ha� diouzhtu ?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Adloc'ha� diouzhtu"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Fellout a ra din adloc'ha� diwezatoc'h dre zorn"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&La�sa� $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Diskouez ar restr Malennit"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Echui�"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Diskouez kavlec'h al La�ser loc'ha�"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Dibabit ur c'havlec'h La�ser loc'ha� evit berradenno� $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Diuzit ar c'havlec'h La�ser loc'ha� e vo savet enna� berradenno� ar goulevio�. Gallout a rit ingal rei� un anv evit sevel ur c'havlec'h nevez."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Chom hep sevel berradenno�"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Distalia� $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Dilemel $(^NameDA) adalek hoc'h urzhiataer."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Ha sur oc'h e fell deoc'h kuitaat stalia� $(^Name) ?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Ha sur oc'h e fell deoc'h kuitaat distalia� $(^Name) ?"
!endif
