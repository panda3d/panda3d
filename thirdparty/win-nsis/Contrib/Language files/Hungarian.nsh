;Language: Hungarian (1038)
;Translation by Jozsef Tamas Herczeg ( - 1.61-ig),
;               Lajos Molnar (Orfanik) <orfanik@axelero.hu> ( 1.62 - t�l)

!insertmacro LANGFILE "Hungarian" "Magyar"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "�dv�zli a(z) $(^NameDA) Telep�t� Var�zsl�"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "A(z) $(^NameDA) telep�t�se k�vetkezik a sz�m�t�g�pre.$\r$\n$\r$\nJavasoljuk, hogy ind�t�s el�tt z�rja be a fut� alkalmaz�sokat. �gy a telep�t� a rendszer �jraind�t�sa n�lk�l tudja friss�teni a sz�ks�ges rendszerf�jlokat.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "�dv�zli a(z) $(^NameDA) Elt�vol�t� Var�zsl�"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Ez a var�zsl� seg�ti a(z) $(^NameDA) elt�vol�t�s�ban.$\r$\n$\r$\nMiel�tt elkezdi az elt�vil�t�st gy�z�dj�n meg arr�l, hogy a(z) $(^NameDA) nem fut.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Licencszerz�d�s"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "A(z) $(^NameDA) telep�t�se el�tt tekintse �t a szerz�d�s felt�teleit."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Ha elfogadja a szerz�d�s valamennyi felt�tel�t, az Elfogadom gombbal folytathatja. El kell fogadnia a(z) $(^NameDA) telep�t�s�hez."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Amennyiben elfogadja a felt�teleket, jel�lje be a jel�l�n�nyzeten. A(z) $(^NameDA) telep�t�s�hez el kell fogadnia a felt�teleket. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Amennyiben elfogadja a felt�teleket, v�lassza az els� opci�t. A(z) $(^NameDA) telep�t�s�hez el kell fogadnia a felt�teleket. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Licencszerz�d�s"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "A(z) $(^NameDA) elt�vol�t�sa el�tt tekintse �t a szerz�d�s felt�teleit."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Ha elfogadja a szerz�d�s valamennyi felt�tel�t, az Elfogadom gombbal folytathatja. El kell fogadnia a(z) $(^NameDA) elt�vol�t�s�hoz."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Amennyiben elfogadja a felt�teleket, jel�lje be a jel�l�n�nyzeten. A(z) $(^NameDA) elt�vol�t�s�hoz el kell fogadnia a felt�teleket. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Amennyiben elfogadja a felt�teleket, v�lassza az els� opci�t. A(z) $(^NameDA) elt�vol�t�s�hoz el kell fogadnia a felt�teleket. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "A PageDown gombbal olvashatja el a szerz�d�s folytat�s�t."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "�sszetev�k kiv�laszt�sa"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "V�lassza ki, hogy a(z) $(^NameDA) mely funkci�it k�v�nja telep�teni."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "�sszetev�k kiv�laszt�sa"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "V�lassza ki, hogy a(z) $(^NameDA) mely funkci�it k�v�nja elt�vol�tani."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Le�r�s"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Vigye r� az egeret az �sszetev�re, hogy megtekinthesse a le�r�s�t."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Vigye r� az egeret az �sszetev�re, hogy megtekinthesse a le�r�s�t."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Telep�t�si hely kiv�laszt�sa"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "V�lassza ki a(z) $(^NameDA) telep�t�s�nek mapp�j�t."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Telep�t�si hely kiv�laszt�sa"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "V�lassza ki a(z) $(^NameDA) telep�t�s�nek mapp�j�t."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Telep�t�si folyamat"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Kis t�relmet a(z) $(^NameDA) telep�t�s�ig."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Telep�t�s befejez�d�tt"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "A telep�t�s sikeresen befejez�d�tt."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "A telep�t�s megszakadt"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "A telep�t�s sikertelen volt."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Elt�vol�t�si folyamat"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Kis t�relmet a(z) $(^NameDA) elt�vol�t�s�ig."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Az elt�vol�t�s befejez�d�tt"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Az elt�vol�t�s sikeresen befejez�d�tt."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Az elt�vol�t�s megszakadt"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Az elt�vol�t�s sikertelen volt."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "A(z) $(^NameDA) telep�t�se megt�rt�nt."
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "A(z) $(^NameDA) telep�t�se megt�rt�nt.$\r$\n$\r$\nA Befejez�s gomb megnyom�s�val z�rja be a var�zsl�t."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "A(z) $(^NameDA) telep�t�s�nek befejez�s�hez �jra kell ind�tani a rendszert. Most akarja �jraind�tani?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "A(z) $(^NameDA) elt�vol�t�s var�zsl�j�nak befejez�se."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "A(z) $(^NameDA) elt�vol�t�sa sikeresen befejez�d�tt.$\r$\n$\r$\nA Finish-re kattintva bez�rul ez a var�zsl�."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "A sz�m�t�g�pet �jra kell ind�tani, hogy a(z) $(^NameDA) elt�vol�t�sa teljes legyen. Akarja most �jraind�tani a rendszert?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Most ind�tom �jra"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "K�s�bb fogom �jraind�tani"
  ${LangFileString} MUI_TEXT_FINISH_RUN "$(^NameDA) futtat�sa"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "OlvassEl f�jl megjelen�t�se"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Befejez�s"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Start men� mappa kijel�l�se"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Start men� mappa kijel�l�se a program parancsikonjaihoz."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Jel�lj�n ki egy mapp�t a Start men�ben, melybe a program parancsikonjait fogja elhelyezni. Be�rhatja �j mappa nev�t is."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nincs parancsikon elhelyez�s"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "A(z) $(^NameDA) Elt�vol�t�sa."
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "A(z) $(^NameDA) elt�vol�t�sa k�vetkezik a sz�m�t�g�pr�l."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Biztos, hogy ki akar l�pni a(z) $(^Name) Telep�t�b�l?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Biztos, hogy ki akar l�pni a(z) $(^Name) Elt�vol�t�b�l?"
!endif
