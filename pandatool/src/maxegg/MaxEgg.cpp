/*
  MaxEgg.cpp 
  Created by Steven "Sauce" Osman, 01/??/03
  odified and maintained by Ken Strickland, (02/25/03)-(Present)
  Carnegie Mellon University, Entetainment Technology Center

  This file implements the classes that are used in the Panda 3D file 
  exporter for 3D Studio Max.
*/

//Includes & Defines
#include "MaxEgg.h"
//Types and structures from windows system-level calls
#include <sys/types.h>
#include <sys/stat.h>
//Controls used in fopen
#include <fcntl.h>
//C Debugging
#include <crtdbg.h>

// Discreet-Generated ID for this app.
#define MaxEggPlugin_CLASS_ID	Class_ID(0x7ac0d6b7, 0x55731ef6)
// Our version number * 100
#define MAX_EGG_VERSION_NUMBER 100 
#define MNEG Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM2
#define MNEG_GEOMETRY_GENERATION Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM3

/* MaxEggPluginClassDesc - A class that describes 3DS Plugin support.
   This basically says "Yes, I can export files. Use me!"
*/
class MaxEggPluginClassDesc:public ClassDesc2 
{
public:
  int IsPublic() { return TRUE; }
  void *Create(BOOL loading = FALSE) { return new MaxEggPlugin(); }
  const TCHAR *ClassName() { return GetString(IDS_CLASS_NAME); }
  SClass_ID SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
  Class_ID ClassID() { return MaxEggPlugin_CLASS_ID; }
  const TCHAR *Category() { return GetString(IDS_CATEGORY); }

  // returns fixed parsable name (scripter-visible name)
  const TCHAR *InternalName() { return _T("MaxEggPlugin"); }
  // returns owning module handle
  HINSTANCE HInstance() { return hInstance; }
};

// Our static instance of the above class
static MaxEggPluginClassDesc MaxEggPluginDesc;

// The function that I believe Max calls, when looking for information as
// to what this plugin does.
ClassDesc2* GetMaxEggPluginDesc() { return &MaxEggPluginDesc; }

/* MaxEggPluginOptionsDlgProc() - This is the callback function for the
   dialog box that appears at the beginning of the conversion process.
 */
BOOL CALLBACK MaxEggPluginOptionsDlgProc( HWND hWnd, UINT message, 
					  WPARAM wParam, LPARAM lParam ) 
{
  //We pass in our plugin through the lParam variable. Let's convert it back.
  MaxEggPlugin *imp = (MaxEggPlugin*)GetWindowLongPtr(hWnd,GWLP_USERDATA); 

  switch(message) 
    {
    // When we start, center the window.
    case WM_INITDIALOG:
      // this line is very necessary to pass the plugin as the lParam
      SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam); 
      CenterWindow(hWnd,GetParent(hWnd));
      return TRUE;
      break;
    // Closing the window is equivalent to canceling the export
    case WM_CLOSE:
      imp->confirmExport = false;
      EndDialog(hWnd, 0);
      return TRUE;
      break;
    // If we get here, this means thatone of our controls was modified.
    case WM_COMMAND:
      //The control in question will be found in the lower word of the wParam 
      // long.
      switch( LOWORD(wParam) ) 
	{
	// The checkbox for toggling whether this model has animations
	case IDC_ANIMATION:
	  // sets the plugin's animation parameter
	  imp->animation = !imp->animation;
	  CheckDlgButton( hWnd, IDC_ANIMATION, 
			  imp->animation ? BST_CHECKED : BST_UNCHECKED );
	  
	  // Enables/disables the animation options depending on how the 
	  // animation checkbox is checked
	  EnableWindow(GetDlgItem(hWnd, IDC_MODEL), imp->animation);
	  EnableWindow(GetDlgItem(hWnd, IDC_CHAN), imp->animation);
	  EnableWindow(GetDlgItem(hWnd, IDC_POSE), imp->animation);
	  EnableWindow(GetDlgItem(hWnd, IDC_STROBE), imp->animation);
	  EnableWindow(GetDlgItem(hWnd, IDC_BOTH), imp->animation);
	  EnableWindow(GetDlgItem(hWnd, IDC_SF), imp->animation);
	  EnableWindow(GetDlgItem(hWnd, IDC_EF), imp->animation);

	  // if this is the first time the animation checkbox has been checked,
	  // then there is no animation type set, so set the animation type to
	  // "model" 
	  if (imp->anim_type == MaxEggPlugin::AT_none) {
	    CheckDlgButton( hWnd, IDC_MODEL, BST_CHECKED );
	    imp->anim_type = MaxEggPlugin::AT_model;
	  }
	  return TRUE;
	  break;

	// The radio buttons for what type of animation will exported
	// The animation type is set by these buttons
	case IDC_MODEL:
	  imp->anim_type = MaxEggPlugin::AT_model;
	  break;
	case IDC_CHAN:
	  imp->anim_type = MaxEggPlugin::AT_chan;
	  break;
	case IDC_POSE:
	  imp->anim_type = MaxEggPlugin::AT_pose;
	  break;
	case IDC_STROBE:
	  imp->anim_type = MaxEggPlugin::AT_strobe;
	  break;
	case IDC_BOTH:
	  imp->anim_type = MaxEggPlugin::AT_both;
	  break;
	  
	//The checkbox that toggles wether to make a .BAM file or not.
	case IDC_MAKE_BAM:
	  imp->makeBam = !imp->makeBam; 
	  CheckDlgButton( hWnd, IDC_MAKE_BAM,
			  imp->makeBam ? BST_CHECKED : BST_UNCHECKED );
	  return TRUE;
	  break;
	// Ckicking the cancel button obviously cancels the export
	case IDC_CANCEL:
	  imp->confirmExport = false;
	  EndDialog(hWnd, 0);
	  return TRUE;
	  break;
	// Clicking the done button is the only way to continue with the export
	case IDC_DONE:
	  imp->confirmExport = true;
	  EndDialog(hWnd, 0);
	  return TRUE;
	  break;
	}
      break;
    }
  return FALSE;
}

/* MaxEggPlugin() - Uninteresting constructor.
 */
MaxEggPlugin::MaxEggPlugin() 
{
  makeBam = false;
  animation = false;
  anim_type = AT_none;
}

/* ~MaxEggPlugin() - Uninteresting destructor.
 */
MaxEggPlugin::~MaxEggPlugin() 
{
}

/* ExtCount() - Returns the number of extensions this exporter produces. 
   That's only one, .EGG files.
 */
int MaxEggPlugin::ExtCount() 
{
  return 1;
}

/* Ext(int) - Returns the nth extension. Since there's only one, it always 
   returns "egg"
*/
const TCHAR *MaxEggPlugin::Ext(int n) 
{
  return _T("egg");
}

/* LongDesc() - A long description of the files we export.  Curiously, this 
   isn't for the nth extension, rather a one-description-fits-all thing.
*/
const TCHAR *MaxEggPlugin::LongDesc() 
{
  return _T("Panda3D .egg file");
}

/**
 * A short description of the files we export.  Curiously, this isn't
 * for the nth extension, rather a one-description-fits-all thing.
 */
const TCHAR *MaxEggPlugin::ShortDesc() 
{			
  return _T("Panda3D");
}

/**
 * Who wrote this.
 */
const TCHAR *MaxEggPlugin::AuthorName() 
{			
  return _T("Steven \"Sauce\" Osman");
}

/**
 * Who owns this.
 */
const TCHAR *MaxEggPlugin::CopyrightMessage() 
{	
  return _T("Copyright (C) 2003 Carnegie Mellon University, Entertainment Technology Center");
}

/**
 * Who cares?
 */
const TCHAR *MaxEggPlugin::OtherMessage1() 
{
  return _T("Modified by Ken Strickland");
}

/**
 * Who knows?
 */
const TCHAR *MaxEggPlugin::OtherMessage2() 
{		
  return _T("Who's got the funk? We do!");
}

/**
 * Returns version * 100.  defined in MAX_EGG_VERSION_NUMBER
 */
unsigned int MaxEggPlugin::Version() 
{				
  return MAX_EGG_VERSION_NUMBER;
}

/**
 * No about dialog box right now.
 */
void MaxEggPlugin::ShowAbout(HWND hWnd) 
{			
}

/**
 * We'll support all options by default.
 */
BOOL MaxEggPlugin::SupportsOptions(int ext, DWORD options) 
{
  // According to the maxsdk help, there is only one option which is 
  // SCENE_EXPORT_SELECTED.  This should return false until the code
  // for converting only selected objects to egg is written
  
  return false;
}

/*!
 * This method creates and triggers the exporter.  Basically it takes the
 * user's options and builds a command-line parameter list from it.
 * It then invokes the converter pretending it was invoked as a standalone
 * program.  BIG WARNING:  The converter stuff often does exit() if the
 * command line arguments are displeasing.
 */
int MaxEggPlugin::DoExport(const TCHAR *ptcOutputFilename,ExpInterface *ei,
			   Interface *pMaxInterface,
			   BOOL suppressPrompts, DWORD options) 
{
  MaxToEgg *pmteConverter = new MaxToEgg();
  char *apcParameters[64];
  char acOutputFilename[MAX_PATH];
  int iParameterCount=0;

  //Initialize our global error logger
  Logger::globalLoggingInstance = new Logger( Logger::PIPE_TO_FILE, 
					      "MaxEggLog.txt" );

  //Set the various logging levels for the subsystems.
  Logger::SetOneErrorMask( ME, Logger::SAT_ALL );
  Logger::SetOneErrorMask( MTE, Logger::SAT_NULL_ERROR | 
			   Logger::SAT_CRITICAL_ERROR | 
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL );
  Logger::SetOneErrorMask( MTEC, Logger::SAT_NULL_ERROR | 
			   Logger::SAT_CRITICAL_ERROR |
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL |
			   Logger::SAT_MEDIUM_LEVEL | Logger::SAT_LOW_LEVEL |
			   Logger::SAT_DEBUG_SPAM_LEVEL );
  Logger::SetOneErrorMask( MNEG, Logger::SAT_NULL_ERROR |
			   Logger::SAT_CRITICAL_ERROR |
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL |
			   Logger::SAT_MEDIUM_LEVEL | Logger::SAT_LOW_LEVEL );
  Logger::SetOneErrorMask( MNEG_GEOMETRY_GENERATION, Logger::SAT_NULL_ERROR |
			   Logger::SAT_CRITICAL_ERROR |
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL |
			   Logger::SAT_MEDIUM_LEVEL | Logger::SAT_LOW_LEVEL );
  Logger::SetOneErrorMask( LLOGGING, Logger::SAT_ALL );
	
  Logger::FunctionEntry( "MaxEggPlugin::DoExport" );

  // Copy the output filename so that it can be modified if necessary
  strncpy(acOutputFilename,ptcOutputFilename,MAX_PATH-1);
  acOutputFilename[MAX_PATH-1]=0;

  // Panda reaaaaaaaaly wants the extension to be in lower case.
  // So if we see a .egg at the end, lower case it.
  if ((strlen(acOutputFilename)>4) &&
      (stricmp(acOutputFilename+strlen(acOutputFilename)-4,".egg")==0)) {
    strlwr(acOutputFilename+strlen(acOutputFilename)-4);
  }
  
  pmteConverter->SetMaxInterface(pMaxInterface);
  
  // Set the command-line arguments
  // ARGV[0] = program name
  apcParameters[iParameterCount++]="MaxEggPlugin";
  
  confirmExport = false;
  if(!suppressPrompts)
    // Displays the dialog box that retrieves the export options
    DialogBoxParam(hInstance, 
		   MAKEINTRESOURCE(IDD_PANEL), 
		   pMaxInterface->GetMAXHWnd(), 
		   MaxEggPluginOptionsDlgProc, (LPARAM)this);
  
  // Stops the export if the user chooses to cancel
  if (!confirmExport)
    return true;
  
  // ARGV[1] = Input file
  // Use a bogus input filename that exists
  apcParameters[iParameterCount++]="nul.max";
  
  // ARGV[2,3] = Output file
  // Pass in the output filename
  // Output file has to be passed in with the -o parameter in order to be able 
  // to overwrite an existing file
  apcParameters[iParameterCount++]="-o";
  apcParameters[iParameterCount++]=acOutputFilename;
  
  // ARGV[4,5] = Animation options (if animation is checked)
  // Check if there is an animation to be saved and what type of animation it
  // will be saved as.  Then set the animation option.
  if (animation) {
    apcParameters[iParameterCount++]="-a";
    switch (anim_type) 
      {
      case AT_model:
	apcParameters[iParameterCount++]="model";
	break;
      case AT_chan:
	apcParameters[iParameterCount++]="chan";
	break;
      case AT_pose:
	apcParameters[iParameterCount++]="pose";
	break;
      case AT_strobe:
	apcParameters[iParameterCount++]="strobe";
	break;
      case AT_both:
	apcParameters[iParameterCount++]="both";
	break;
      default:
	apcParameters[iParameterCount++]="none";
	break;
      }
  }
  apcParameters[iParameterCount]=0;

  // Parse the command line and run the converter
  pmteConverter->parse_command_line(iParameterCount, apcParameters);
  pmteConverter->Run();
  
  bool bSuccessful = pmteConverter->IsSuccessful();
  
  // Display a message box telling that the export is completed
  if (bSuccessful)
    MessageBox(pMaxInterface->GetMAXHWnd(), 
	       "Export to EGG completed successfully.", "Panda3D Converter",
	       MB_OK);
  else
    MessageBox(pMaxInterface->GetMAXHWnd(), "Export unsuccessful.", 
	       "Panda3D Converter", MB_OK);
		Logger::Log(MTEC, Logger::SAT_MEDIUM_LEVEL, "After finished mbox");

  // This was put in try block because originally deleting pmteConverter 
  // would throw an exception.  That no longer happens, but this is still
  // here for good measure
  try {
		Logger::Log(MTEC, Logger::SAT_MEDIUM_LEVEL, "before deleting pmteconverter");
    delete pmteConverter; 
  } catch (...) {
		Logger::Log(MTEC, Logger::SAT_MEDIUM_LEVEL, "before error message window");
    MessageBox(pMaxInterface->GetMAXHWnd(), "I just got an unknown exception.",
	       "Panda3D Converter", MB_OK);
  }
		Logger::Log(MTEC, Logger::SAT_MEDIUM_LEVEL, "before logger function exit");
  Logger::FunctionExit();
  //Free the error logger
  if ( Logger::globalLoggingInstance )
    delete Logger::globalLoggingInstance;
 
  return bSuccessful;
}
