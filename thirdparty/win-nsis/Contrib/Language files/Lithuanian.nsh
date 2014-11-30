;Language: Lithuanian (1063)
;By Vytautas Krivickas (Vytautas). Updated by Danielius Scepanskis (Daan daniel@takas.lt) 2004.01.09

!insertmacro LANGFILE "Lithuanian" "Lietuviu"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Sveiki atvyk� � $(^NameDA) �diegimo program�."
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "�i programa jums pad�s lengvai �diegti $(^NameDA).$\r$\n$\r$\nRekomenduojama i�jungti visas programas, prie� pradedant �diegim�. Tai leis atnaujinti sistemos failus neperkraunat kompiuterio.$\r$\n$\r$\n"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Sveiki atvyk� � $(^NameDA) pa�alinimo program�."
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "�i programa jums pad�s lengvai i�trinti $(^NameDA).$\r$\n$\r$\nPrie� pradedant pasitikrinkite kad $(^NameDA) yra i�jungta.$\r$\n$\r$\n"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Naudojimo sutartis"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Pra�ome perskaityti sutart� prie� �diegdami $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Jei j�s sutinkate su nurodytomis s�lygomis, spauskite Sutinku. J�s privalote sutikti, jei norite �diegti $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Jei j�s sutinkate su nurodytomis s�lygomis, pad�kite varnel� tam skirtame laukelyje. J�s privalote sutikti, jei norite �diegti $(^NameDA). "
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Jei j�s sutinkate su nurodytomis s�lygomis, pasirinkite pirm� pasirinkim� esant� �emiau. J�s privalote sutikti, jei norite �diegti $(^NameDA). "
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Naudojimo sutartis"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Pra�ome perskaityti sutart� prie� $(^NameDA) pa�alinim�."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Jei j�s sutinkate su nurodytomis s�lygomis, spauskite Sutinku. J�s privalote sutikti, jei norite i�trinti $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "s, pad�kite varnel� tam skirtame laukelyje. J�s privalote sutikti, jei norite i�trinti $(^NameDA). "
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Jei j�s sutinkate su nurodytomis s�lygomis, pasirinkite pirm� pasirinkim� esant� �emiau. J�s privalote sutikti, jei norite i�trinti $(^NameDA)."
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Paspauskite Page Down ir perskaitykite vis� sutart�."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Pasirinkite"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Pasirinkite kokias $(^NameDA) galimybes j�s norite �diegti."
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Pasirinkite"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Pasirinkite kokias $(^NameDA) galimybes j�s norite pa�alinti."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Paai�kinimas"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "U�veskite pel�s �ymekl� ant komponento ir pamatysite jo apra�ym�."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "U�veskite pel�s �ymekl� ant komponento ir pamatysite jo apra�ym�."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Pasirinkite �diegimo viet�"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Pasirinkite katalog� � k�ri �diegsite $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Pasirinkite i�trinimo viet�"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Pasirinkite katalog� i� kurio i�trinsite $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Diegiama"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Pra�ome palaukti, kol $(^NameDA) bus �diegtas."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "�diegimas baigtas"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "�diegimas baigtas sekmingai."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "�diegimas nutrauktas"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "�diegimas nebuvo baigtas sekmingai."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "�alinama"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Pra�ome palaukti, kol $(^NameDA) bus pa�alinta."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Programos pa�alinimas baigtas"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "I�trynimas baigtas sekmingai."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "I�trynimas nutrauktas"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "I�trynimas nebuvo baigtas sekmingai."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Baigiu $(^NameDA) �diegimo proces�"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) buvo �diegtas � j�s� kompiuter�.$\r$\n$\r$\nPaspauskite Baigti."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "J�s� kompiuteris turi b�ti perkrautas, kad b�t� baigtas $(^NameDA) �diegimas. Ar j�s norite perkrauti dabar?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Baigiu $(^NameDA) pa�alinimo program�."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) buvo i�trinta i� j�s� kompiuterio.$\r$\n$\r$\nPaspauskite Baigti."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "J�s� kompiuteris turi b�ti perkrautas, kad b�t� baigtas $(^NameDA) pa�alinimas. Ar j�s norite perkrauti dabar?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Perkrauti dabar"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "A� noriu perkrauti veliau pats"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Leisti $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Parodyti dokumentacij�"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Baigti"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Pasirinkite Start Menu katalog�"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Pasirinkite Start Menu katalog�, kuriame bus sukurtos programos nuorodos."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Pasirinkite Start Menu katalog�, kuriame bus sukurtos programos nuorodos. J�s taip pat galite sukurti nauj� katalog�."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Nekurti nuorod�"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Panaikinti $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "I�trinti $(^NameDA) i� j�s� kompiuterio."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Ar j�s tikrai norite i�jungti $(^Name) �diegimo program�?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Ar j�s tikrai norite i�jungti $(^Name) pa�alinimo program�?"
!endif
