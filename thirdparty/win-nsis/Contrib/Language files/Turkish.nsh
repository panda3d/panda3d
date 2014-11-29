;Language: Turkish (1055)
;By �agatay Dilsiz(Chagy)
;Updated by Fatih BOY (fatih_boy@yahoo.com)

!insertmacro LANGFILE "Turkish" "T�rk�e"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "$(^NameDA) Kurulum sihirbaz�na ho� geldiniz"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Bu sihirbaz size $(^NameDA) kurulumu boyunca rehberlik edecektir.$\r$\n$\r$\nKurulumu ba�latmadan �nce �al��an di�er programlari kapatman�z� �neririz. B�ylece bilgisayar�n�z� yeniden ba�latmadan baz� sistem dosyalar� sorunsuz kurulabilir.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "$(^NameDA) Program�n� Kald�rma Sihirbaz�na Ho� Geldiniz"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Bu sihirbaz size $(^NameDA) program�n�n kad�r�l�m� boyunca rehberlik edecektir.$\r$\n$\r$\nKald�r�m i�lemeni ba�latmadan �nce �al��an di�er programlari kapatman�z� �neririz. B�ylece bilgisayar�n�z� yeniden ba�latmadan baz� sistem dosyalar� sorunsuz kald�r�labilir.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Lisans S�zle�mesi"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "L�tfen $(^NameDA) program�n� kurmadan �nce s�zle�meyi okuyunuz."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "S�zle�me ko�ullar�n� kabul ediyorsan�z, 'Kabul Ediyorum'a bas�n�z. $(^NameDA) program�n� kurmak i�in s�zle�me ko�ullar�n� kabul etmelisiniz."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "S�zle�me ko�ullar�n� kabul ediyorsan�z, a�a��daki onay kutusunu doldurunuz. $(^NameDA) program�n� kurmak i�in s�zle�me ko�ullar�n� kabul etmelisiniz. $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "S�zle�me ko�ullar�n� kabul ediyorsan�z, asagidaki onay d��mesini se�iniz. $(^NameDA) program�n� kurmak i�in s�zle�me ko�ullar�n� kabul etmelisiniz. $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Lisans S�zle�mesi"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "L�tfen $(^NameDA) program�n� sisteminizden kald�rmadan �nce s�zle�meyi okuyunuz."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "S�zle�me ko�ullar�n� kabul ediyorsan�z, 'Kabul Ediyorum'a bas�n�z. $(^NameDA) program�n� sisteminizden kald�rmak i�in s�zle�me ko�ullar�n� kabul etmelisiniz."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "S�zle�me ko�ullar�n� kabul ediyorsan�z, a�a��daki onay kutusunu doldurunuz. $(^NameDA) program�n� sisteminizden kald�rmak i�in s�zle�me ko�ullar�n� kabul etmelisiniz. $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "S�zle�me ko�ullar�n� kabul ediyorsan�z, asagidaki onay d��mesini se�iniz. $(^NameDA) program�n� sisteminizden kald�rmak i�in s�zle�me ko�ullar�n� kabul etmelisiniz. $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "S�zle�menin geri kalan�n� okumak i�in 'page down' tu�una basabilirsiniz."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Bile�en se�imi"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "L�tfen $(^NameDA) i�in kurmak istediginiz bile�enleri se�iniz."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Bile�en �e�imi"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "L�tfen kald�rmak istedi�iniz $(^NameDA) program bile�enini se�iniz."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "A��klama"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Bile�enlerin a��klamalar�n� g�rmek i�in imleci bile�en �zerine g�t�r�n."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Bile�enlerin a��klamalar�n� g�rmek i�in imleci bile�en �zerine g�t�r�n."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Hedef dizini se�imi"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "$(^NameDA) program�n� kurmak istedi�iniz dizini �e�iniz."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Kald�r�l�cak Dizin Se�imi"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "$(^NameDA) program�n� kald�rmak istediginiz dizini se�iniz."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Kuruluyor"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "L�tfen $(^NameDA) kurulurken bekleyiniz."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Kurulum Tamamland�"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Kurulum ba�ar�yla tamamland�."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Kurulum �ptal Edildi"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Kurulum tam olarak tamamlanmad�."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Kald�r�l�yor"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Litfen $(^NameDA) program� sisteminizden kald�r�l�rken bekleyiniz."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Kald�rma ��lemi Tamamland�r"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Kald�rma i�lemi ba�ar�yla tamamland�."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Kald�rma ��lemi �ptal Edildi"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Kald�rma ��lemi tamamlanamad�."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "$(^NameDA) Kurulum sihirbaz� tamamlan�yor."
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA)  bilgisayariniza y�klendi.$\r$\n$\r$\nL�tfen 'Bitir'e basarak kurulumu sonland�r�n."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "$(^NameDA) kurulumunun tamamlanmas� i�in bilgisayar�n�z� yeniden ba�latman�z gerekiyor.Bilgisayar�n�z� yeniden ba�latmak istiyor musunuz?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "$(^NameDA) Program� Kald�rma Sihirbaz� Tamamlan�yor"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) program� sisteminizden kald�r�ld�.$\r$\n$\r$\nSihirbaz� kapatmak i�in 'bitir'e bas�n�z."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "$(^NameDA) program�n� kald�rma i�leminin tamamlanmas� i�in bilgisayar�n�z�n yeniden ba�lat�lmas� gerekiyor. Bilgisayar�n�z�n �imdi yeniden ba�lat�lmas�n� ister misiniz?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Yeniden ba�lat"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Bilgisayar�m� daha sonra ba�lataca��m."
  ${LangFileString} MUI_TEXT_FINISH_RUN "$(^NameDA) program�n� �al��t�r"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "beni oku/readme dosyas�n� &g�ster"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Bitir"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Ba�lat Men�s� Klas�r Se�imi"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "$(^NameDA) k�sayollar�n�n konulacag� ba�lat men�s� klas�r�n� se�iniz."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Program�n k�sayollar�n�n konulaca�� ba�lat men�s� klas�r�n� se�iniz. Farkl� bir isim girerek yeni bir klas�r yaratabilirsiniz."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "K�sayollar� olu�turmadan devam et"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "$(^NameDA) Program�n� Kald�r"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "$(^NameDA) program�n� sisteminizden kald�rma."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "$(^Name) kurulumundan ��kmak istedi�inize emin misiniz?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "$(^Name) Programi Kald�rma i�leminden ��kmak istedi�inize emin misiniz?"
!endif
