;NSIS Modern User Interface - Language File
;Compatible with Modern UI 1.68

;Language: English (1033)
;By Joost Verburg

;--------------------------------

!insertmacro MUI_LANGUAGEFILE_BEGIN "English"

  !define MUI_LANGNAME "English" ;Use only ASCII characters (if this is not possible, use the English name)
  
  !define MUI_TEXT_WELCOME_INFO_TITLE "Welcome to the $(^NameDA) Setup Wizard"
  !define MUI_TEXT_WELCOME_INFO_TEXT "This wizard will guide you through the installation of $(^NameDA). With just a few clicks, you'll be on your way to creating great games with $(^NameDA).\r\n\r\nIt is recommended that you close all other applications before starting Setup. This will make it possible to update relevant system files without having to reboot your computer.\r\n\r\n$_CLICK"
  
  !define MUI_TEXT_LICENSE_TITLE "License Agreement"  
  !define MUI_TEXT_LICENSE_SUBTITLE "Please review the license terms before installing $(^NameDA)."
  !define MUI_INNERTEXT_LICENSE_TOP "Press Page Down to see the rest of the agreement."
  !define MUI_INNERTEXT_LICENSE_BOTTOM "If you accept the terms of the agreement, click I Agree to continue. You must accept the agreement to install $(^NameDA)."
  !define MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "If you accept the terms of the agreement, click the check box below. You must accept the agreement to install $(^NameDA). $_CLICK"
  !define MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "If you accept the terms of the agreement, select the first option below. You must accept the agreement to install $(^NameDA). $_CLICK"
  
  !define MUI_TEXT_COMPONENTS_TITLE "Choose Components"
  !define MUI_TEXT_COMPONENTS_SUBTITLE "Choose which features of $(^NameDA) you want to install."
  !define MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Description"
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    !define MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Position your mouse over a component to see its description."
  !else
    !define MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Select a component to see its description."
  !endif
  
  !define MUI_TEXT_DIRECTORY_TITLE "Choose Install Location"
  !define MUI_TEXT_DIRECTORY_SUBTITLE "Please choose the folder in which to install $(^NameDA)."
  
  !define MUI_TEXT_INSTALLING_TITLE "Installing"
  !define MUI_TEXT_INSTALLING_SUBTITLE "Please wait while $(^NameDA) is being installed. This will take just a few minutes."
  
  !define MUI_TEXT_FINISH_TITLE "Installation Complete"
  !define MUI_TEXT_FINISH_SUBTITLE "Setup was completed successfully."
  
  !define MUI_TEXT_ABORT_TITLE "Installation Aborted"
  !define MUI_TEXT_ABORT_SUBTITLE "Setup was not completed successfully."
  
  !define MUI_BUTTONTEXT_FINISH "&Finish"
  !define MUI_TEXT_FINISH_INFO_TITLE "Completing the $(^NameDA) Setup Wizard"
  !define MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) has been installed on your computer.\r\n\r\n"
  !define MUI_TEXT_FINISH_INFO_REBOOT "Your computer must be restarted in order to complete the installation of $(^NameDA). Do you want to reboot now?"
  !define MUI_TEXT_FINISH_REBOOTNOW "Reboot now"
  !define MUI_TEXT_FINISH_REBOOTLATER "I want to manually reboot later"
  !define MUI_TEXT_FINISH_RUN "&Run $(^NameDA)"
  !define MUI_TEXT_FINISH_SHOWREADME "&Show Readme"
  
  !define MUI_TEXT_STARTMENU_TITLE "Choose Start Menu Folder"
  !define MUI_TEXT_STARTMENU_SUBTITLE "Choose a Start Menu folder for the $(^NameDA) shortcuts."
  !define MUI_INNERTEXT_STARTMENU_TOP "Select the Start Menu folder in which you would like to create the program's shortcuts. You can also enter a name to create a new folder."
  !define MUI_INNERTEXT_STARTMENU_CHECKBOX "Do not create shortcuts"
  
  !define MUI_TEXT_ABORTWARNING "Are you sure you want to quit $(^Name) Setup?"
  
  
  !define MUI_UNTEXT_WELCOME_INFO_TITLE "Welcome to the $(^NameDA) Uninstall Wizard"
  !define MUI_UNTEXT_WELCOME_INFO_TEXT "This wizard will guide you through the uninstallation of $(^NameDA).\r\n\r\nBefore starting the uninstallation, make sure $(^NameDA) is not running.\r\n\r\n$_CLICK"
  
  !define MUI_UNTEXT_CONFIRM_TITLE "Uninstall $(^NameDA)"
  !define MUI_UNTEXT_CONFIRM_SUBTITLE "Remove $(^NameDA) from your computer."
  
  !define MUI_UNTEXT_LICENSE_TITLE "License Agreement"  
  !define MUI_UNTEXT_LICENSE_SUBTITLE "Please review the license terms before uninstalling $(^NameDA)."
  !define MUI_UNINNERTEXT_LICENSE_BOTTOM "If you accept the terms of the agreement, click I Agree to continue. You must accept the agreement to uninstall $(^NameDA)."
  !define MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "If you accept the terms of the agreement, click the check box below. You must accept the agreement to uninstall $(^NameDA). $_CLICK"
  !define MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "If you accept the terms of the agreement, select the first option below. You must accept the agreement to uninstall $(^NameDA). $_CLICK"
  
  !define MUI_UNTEXT_COMPONENTS_TITLE "Choose Components"
  !define MUI_UNTEXT_COMPONENTS_SUBTITLE "Choose which features of $(^NameDA) you want to uninstall."
  
  !define MUI_UNTEXT_DIRECTORY_TITLE "Choose Uninstall Location"
  !define MUI_UNTEXT_DIRECTORY_SUBTITLE "Choose the folder from which to uninstall $(^NameDA)."
  
  !define MUI_UNTEXT_UNINSTALLING_TITLE "Uninstalling"
  !define MUI_UNTEXT_UNINSTALLING_SUBTITLE "Please wait while $(^NameDA) is being uninstalled."
    
  !define MUI_UNTEXT_FINISH_TITLE "Uninstallation Complete"
  !define MUI_UNTEXT_FINISH_SUBTITLE "Uninstall was completed successfully."
  
  !define MUI_UNTEXT_ABORT_TITLE "Uninstallation Aborted"
  !define MUI_UNTEXT_ABORT_SUBTITLE "Uninstall was not completed successfully."
  
  !define MUI_UNTEXT_FINISH_INFO_TITLE "Completing the $(^NameDA) Uninstall Wizard"
  !define MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) has been uninstalled from your computer.\r\n\r\nClick Finish to close this wizard."
  !define MUI_UNTEXT_FINISH_INFO_REBOOT "Your computer must be restarted in order to complete the uninstallation of $(^NameDA). Do you want to reboot now?"
  
  !define MUI_UNTEXT_ABORTWARNING "Are you sure you want to quit $(^Name) Uninstall?"
  
!insertmacro MUI_LANGUAGEFILE_END