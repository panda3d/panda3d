// This script injects the appropriate syntax into the document to
// embed Panda3D, either for the ActiveX or Mozilla-based plugin.

// It is also possible to write browser-independent code by nesting
// <object> tags, but this causes problems when you need to reference
// the particular object that is actually running (which object is
// it?) for scripting purposes.

// This script writes only a single <object> tag, and it can be
// assigned the id you specify, avoiding this ambiguity.

var isIE  = (navigator.appVersion.indexOf("MSIE") != -1) ? true : false;
var isWin = (navigator.appVersion.toLowerCase().indexOf("win") != -1) ? true : false;
var isOpera = (navigator.userAgent.indexOf("Opera") != -1) ? true : false;


function P3D_Generateobj(objAttrs, params, embedAttrs, imageAttrs) 
{ 
  var str = '';

  if (isIE && isWin && !isOpera)
  {
    str += '<object ';
    for (var i in objAttrs)
    {
      str += i + '="' + objAttrs[i] + '" ';
    }
    str += '>';
    for (var i in params)
    {
      str += '<param name="' + i + '" value="' + params[i] + '" /> ';
    }
  }
  else
  {
    str += '<object ';
    for (var i in embedAttrs)
    {
      str += i + '="' + embedAttrs[i] + '" ';
    }
    str += '> ';
  }
  if (imageAttrs["src"]) {
    if (imageAttrs["href"]) {
      str += '<a href="' + imageAttrs["href"] + '">';
    }
    str += '<img ';
    for (var i in imageAttrs) {
      if (i != "href") {
        str += i + '="' + imageAttrs[i] + '" ';
      }
    }
    str += '>';
    if (imageAttrs["href"]) {
      str += '</a>';
    }
  }
  str += '</object>';

  document.write(str);
}

function P3D_RunContent() {
  var ret = 
    P3D_GetArgs
      (arguments, "clsid:924b4927-d3ba-41ea-9f7e-8a89194ab3ac",
       "application/x-panda3d");
  P3D_Generateobj(ret.objAttrs, ret.params, ret.embedAttrs, ret.imageAttrs);
}

function P3D_GetArgs(args, classid, mimeType){
  var ret = new Object();
  ret.embedAttrs = new Object();
  ret.params = new Object();
  ret.objAttrs = new Object();
  ret.imageAttrs = new Object();

  for (var i = 0; i < args.length; i = i + 2){
    var currArg = args[i].toLowerCase();    

    switch (currArg){	
    case "src":
    case "data":
        ret.embedAttrs['data'] = args[i+1];
        ret.params['data'] = args[i+1];
        break;

    case "codebase":
        ret.objAttrs['codebase'] = args[i+1];
        break;

    case "noplugin_img":
        ret.imageAttrs["src"] = args[i+1];
        ret.imageAttrs["border"] = '0';
        break;

    case "noplugin_href":
        ret.imageAttrs["href"] = args[i+1];
        break;

    case "splash_img":
        ret.embedAttrs[args[i]] = ret.params[args[i]] = args[i+1];
        if (!ret.imageAttrs["src"]) {
          ret.imageAttrs["src"] = args[i+1];
        }
        break;

    case "width":
    case "height":
        ret.imageAttrs[args[i]] = ret.embedAttrs[args[i]] = ret.objAttrs[args[i]] = args[i+1];
        break;

    case "id":
    case "align":
    case "vspace": 
    case "hspace":
    case "class":
    case "title":
    case "accesskey":
    case "name":
    case "tabindex":
        ret.embedAttrs[args[i]] = ret.objAttrs[args[i]] = args[i+1];
        break;

    default:
        ret.embedAttrs[args[i]] = ret.params[args[i]] = args[i+1];
    }
  }
  ret.objAttrs["classid"] = classid;
  if (mimeType) ret.embedAttrs["type"] = mimeType;
  return ret;
}
