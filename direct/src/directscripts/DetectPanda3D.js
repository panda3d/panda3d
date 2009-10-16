// Based on Apple sample code at
// http://developer.apple.com/internet/webcontent/examples/detectplugins_source.html


// initialize global variables
var detectableWithVB = false;
var pluginFound = false;

function goURL(daURL) {
    // Assume we have Javascript 1.1 functionality.
    window.location.replace(daURL);
    return;
}

function redirectCheck(pluginFound, redirectURL, redirectIfFound) {
    // check for redirection
    if( redirectURL && ((pluginFound && redirectIfFound) || 
	(!pluginFound && !redirectIfFound)) ) {
	// go away
	goURL(redirectURL);
	return pluginFound;
    } else {
	// stay here and return result of plugin detection
	return pluginFound;
    }	
}

function canDetectPlugins() {
    if( detectableWithVB || (navigator.plugins && navigator.plugins.length > 0) ) {
	return true;
    } else {
	return false;
    }
}

function detectPanda3D(redirectURL, redirectIfFound) {
    pluginFound = detectPlugin('Panda3D'); 
    // if not found, try to detect with VisualBasic
    if(!pluginFound && detectableWithVB) {
	pluginFound = detectActiveXControl('P3DACTIVEX.P3DActiveXCtrl.1');
    }
    // check for redirection
    return redirectCheck(pluginFound, redirectURL, redirectIfFound);
}

function detectPlugin() {
    // allow for multiple checks in a single pass
    var daPlugins = detectPlugin.arguments;
    // consider pluginFound to be false until proven true
    var pluginFound = false;
    // if plugins array is there and not fake
    if (navigator.plugins && navigator.plugins.length > 0) {
	var pluginsArrayLength = navigator.plugins.length;
	// for each plugin...
	for (pluginsArrayCounter=0; pluginsArrayCounter < pluginsArrayLength; pluginsArrayCounter++ ) {
	    // loop through all desired names and check each against the current plugin name
	    var numFound = 0;
	    for(namesCounter=0; namesCounter < daPlugins.length; namesCounter++) {
		// if desired plugin name is found in either plugin name or description
		if( (navigator.plugins[pluginsArrayCounter].name.indexOf(daPlugins[namesCounter]) >= 0) || 
		    (navigator.plugins[pluginsArrayCounter].description.indexOf(daPlugins[namesCounter]) >= 0) ) {
		    // this name was found
		    numFound++;
		}   
	    }
	    // now that we have checked all the required names against this one plugin,
	    // if the number we found matches the total number provided then we were successful
	    if(numFound == daPlugins.length) {
		pluginFound = true;
		// if we've found the plugin, we can stop looking through at the rest of the plugins
		break;
	    }
	}
    }
    return pluginFound;
} // detectPlugin


// Here we write out the VBScript block for MSIE Windows
if ((navigator.userAgent.indexOf('MSIE') != -1) && (navigator.userAgent.indexOf('Win') != -1)) {
    document.writeln('<script language="VBscript">');

    document.writeln('\'do a one-time test for a version of VBScript that can handle this code');
    document.writeln('detectableWithVB = False');
    document.writeln('If ScriptEngineMajorVersion >= 2 then');
    document.writeln('  detectableWithVB = True');
    document.writeln('End If');

    document.writeln('\'this next function will detect most plugins');
    document.writeln('Function detectActiveXControl(activeXControlName)');
    document.writeln('  on error resume next');
    document.writeln('  detectActiveXControl = False');
    document.writeln('  If detectableWithVB Then');
    document.writeln('     detectActiveXControl = IsObject(CreateObject(activeXControlName))');
    document.writeln('  End If');
    document.writeln('End Function');

    document.writeln('</script>');
}
