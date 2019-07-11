########################################################################
##
## Win32 Usage: makepanda\makechm.bat
## Linux Usage: makepanda/makechm.py
##
## To use this script, you will need to have hhc.exe on your system.
## For verbose output, run with -v or --verbose option.
## To keep the temporary .hhc, .hhk, .hhp, .chw files use -k or --keep.
##
## You can also import this file as a python module. You will then have
## access to three functions: makeCHM, makeManualCHM, makeReferenceCHM.
## This is how you call them:
##   makeCHM(outputfile, dirname, title)
## where outputfile is the filename where the .chm file will be written,
## and dirname is the directory containing the html files to include.
## Title will be the title of the CHM file.
## The functions makeManualCHM and makeReferenceCHM work exactly the
## same, except that they work with a structure resembling that of the
## Panda3D manual and reference, respectively.
## Note: outputfile should not contain spaces.
##
########################################################################

__all__ = ["makeCHM", "makeManualCHM", "makeReferenceCHM"]
import os, re
from sys import exit
import xml.dom.minidom
from xml.dom.minidom import Node

VERBOSE = False
KEEPTEMP = False

if __name__ == "__main__":
    from sys import argv
    VERBOSE  = ("-v" in argv) or ("-vk" in argv) or ("-kv" in argv) or ("--verbose" in argv)
    KEEPTEMP = ("-k" in argv) or ("-kv" in argv) or ("-vk" in argv) or ("--keep"    in argv)

OPTIONBLOCK = """
Binary TOC=Yes
Compatibility=1.1 or later
Compiled file=%s
Contents file=%s.hhc
Default Font=Arial,10,0
Default topic=%s
Display compile progress=VERBOSE
Full-text search=Yes
Index file=%s.hhk
Language=0x409 English (United States)
Title=%s""".replace("VERBOSE", VERBOSE and "Yes" or "No")

HTMLBLOCK = """<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<html>
  <head>
    <meta name="generator" content="Panda3D - makechm.py">
  </head>
  <body>
    <object type="text/site properties">
      <param name="Window Styles" value="0x800025">
      <param name="ImageType" value="Folder">
      <param name="Font" value="Arial,8,0">
    </object>
    <ul>\n"""

REFERENCEITEMS = [
    ("index.html",     "Main Page"),
    ("methods.html",   "Methods"),
    ("functions.html", "Global Functions"),
    ("classes.html",   "Classes"),
]

def urldecode(url):
    regex = re.compile("%([0-9a-hA-H][0-9a-hA-H])", re.M)
    return regex.sub(lambda x: chr(int(x.group(1), 16)), url)

def ireplace(string, target, replacement):
    """Case-insensitive replace."""
    index = string.lower().find(target.lower())
    if index >= 0:
        result = string[:index] + replacement + string[index + len(target):]
        return result
    else:
        return string

def parseAnchor(node):
    """Parses an XML minidom node representing an anchor and returns a tuple
    containing the href and the content of the link."""
    assert node.nodeType == Node.ELEMENT_NODE
    assert node.localName == "a"
    href = ""
    title = ""
    for localName, a in node.attributes.items():
        if localName.lower() == "href":
            href = a
    for e in node.childNodes:
        if e.nodeType == Node.TEXT_NODE:
            title += e.data
    return href, title

def parseManualTree(node):
    """Parses a tree of the manual Main_Page and returns it through a list containing tuples:
    [(title, href, [(title, href, [...]), ...]), ...]"""
    if node.nodeType != Node.ELEMENT_NODE: return []
    result = []
    lastadded = None
    for e in node.childNodes:
        if e.nodeType == Node.ELEMENT_NODE:
            if e.localName == "ol":
                assert lastadded != None
                for i in xrange(len(result)):
                    if result[i][:2] == lastadded:
                        result[i] = lastadded + (parseManualTree(e),)
            elif e.localName == "a":
                href, title = parseAnchor(e)
                lastadded = title, href
                result.append((title, href, None))
    return result

def parseManualTOC(filename):
    """Reads the manual's Main_Page file and returns a list of all the trees found."""
    filename = open(filename)
    text = filename.read()
    filename.close()
    text = text.split("<h2>Table of Contents</h2>")[1].split("</div>")[0]
    text = "<root>" + text.replace("<li>", "") + "</root>"
    text = re.sub(re.compile("<!--([^>]+)>"), "", text)
    result = []
    for e in xml.dom.minidom.parseString(text).childNodes[0].childNodes:
        if e.nodeType == Node.ELEMENT_NODE:
            result.append(parseManualTree(e))
    return result

def treeToHTML(tree, dirname, indent = ""):
    """Converts a tree into HTML code suitable for .hhc or .hhk files. The tree should be like:
    [(title, href, [(title, href, [...]), ...]), ...]"""
    html = ""
    for title, href, sub in tree:
        html += indent + "<li><object type=\"text/sitemap\">\n"
        html += indent + "  <param name=\"Name\" value=\"%s\">\n" % title.replace("CXX", "C++").replace("\"", "&quot;")
        html += indent + "  <param name=\"Local\" value=\"%s\">\n" % urldecode(os.path.join(dirname, href))
        html += indent + "</object>\n"
        if sub != None:
            html += indent + "<ul>\n"
            html += treeToHTML(sub, dirname, indent + "  ")
            html += indent + "</ul>\n"
    return html

def makeCHM(outputfile, dirname, title, special = None):
    """Creates a CHM file based on a directory of HTML files. See the top of this file for more info."""
    assert special == None or special in ["manual", "reference"]
    reference = (special == "reference")
    manual = (special == "manual")
    base = ireplace(outputfile, ".chm", "")
    if os.path.isfile(base + ".chm"): os.remove(base + ".chm")
    # Create the hhp file
    hhp = open(base + ".hhp", "w")
    hhp.write("[OPTIONS]\n")
    hhp.write(OPTIONBLOCK % (base + ".chm", base, urldecode(os.path.join(dirname, "index.html")), base, title))
    hhp.write("\n[FILES]\n")
    # Create the TOC file and Index file
    hhk = open(base + ".hhk", "w")
    hhc = open(base + ".hhc", "w")
    hhk.write(HTMLBLOCK)
    hhc.write(HTMLBLOCK)
    # The manual should be treated as a special case.
    if manual:
        hhc.write("      <li><object type=\"text/sitemap\">\n")
        hhc.write("        <param name=\"Name\" value=\"Main Page\">\n")
        hhc.write("        <param name=\"Local\" value=\"%s\">\n" % urldecode(os.path.join(dirname, "index.html")))
        hhc.write("      </object>\n")
        for item in parseManualTOC(dirname + "/index.html"):
            hhc.write(treeToHTML(item, dirname, "      "))
        for i in os.listdir(dirname):
            hhp.write(os.path.join(dirname, i) + "\n")
            if i != "index.html":
                hhk.write("      <li><object type=\"text/sitemap\">\n")
                hhk.write("        <param name=\"Name\" value=\"%s\">\n" % ireplace(urldecode(i).replace(".1", "").replace("_", " ").replace("CXX", "C++"), ".html", "").replace("\"", "&quot;"))
                hhk.write("        <param name=\"Local\" value=\"%s\">\n" % urldecode(os.path.join(dirname, i)))
                hhk.write("      </object>\n")
    else:
        idt = "      "
        # If we are writing out the reference, write some extra stuff.
        if reference:
            idt = "        "
            for i, desc in REFERENCEITEMS:
                hhk.write("      <li><object type=\"text/sitemap\">\n")
                hhk.write("        <param name=\"Name\" value=\"%s\">\n" % desc.replace("\"", "&quot;"))
                hhk.write("        <param name=\"Local\" value=\"%s\">\n" % urldecode(os.path.join(dirname, i)))
                hhk.write("      </object>\n")
                hhc.write("      <li><object type=\"text/sitemap\">\n")
                hhc.write("        <param name=\"Name\" value=\"%s\">\n" % desc.replace("\"", "&quot;"))
                hhc.write("        <param name=\"Local\" value=\"%s\">\n" % urldecode(os.path.join(dirname, i)))
                hhc.write("      </object>\n")
            hhc.write("      <ul>\n")
        # Loop through the directories and write out relevant data.
        for i in os.listdir(dirname):
            hhp.write(os.path.join(dirname, i) + "\n")
            if i != "index.html" and ((not reference) or (not i  in ["classes.html", "methods.html", "functions.html"])):
                hhk.write("      <li><object type=\"text/sitemap\">\n")
                hhk.write("        <param name=\"Name\" value=\"%s\">\n" % ireplace(urldecode(i).replace(".1", ""), ".html", "").replace("\"", "&quot;"))
                hhk.write("        <param name=\"Local\" value=\"%s\">\n" % urldecode(os.path.join(dirname, i)))
                hhk.write("      </object>\n")
                hhc.write(idt + "<li><object type=\"text/sitemap\">\n")
                hhc.write(idt + "  <param name=\"Name\" value=\"%s\">\n" % ireplace(urldecode(i).replace(".1", ""), ".html", "").replace("\"", "&quot;"))
                hhc.write(idt + "  <param name=\"Local\" value=\"%s\">\n" % urldecode(os.path.join(dirname, i)))
                hhc.write(idt + "</object>\n")
    # Close the files.
    if reference: hhc.write("      </ul>\n")
    hhk.write("    </ul>\n  </body>\n</html>")
    hhc.write("    </ul>\n  </body>\n</html>")
    hhk.close()
    hhc.close()
    hhp.close()
    # Now, execute the command to compile the files.
    if "PROGRAMFILES" in os.environ and os.path.isdir("%s\\HTML Help Workshop" % os.environ["PROGRAMFILES"]):
        cmd = "\"%s\\HTML Help Workshop\\hhc.exe\" %s.hhp" % (os.environ["PROGRAMFILES"], base)
    elif os.path.isdir("C:\\Program Files\\HTML Help Workshop"):
        cmd = "\"C:\\Program Files\\HTML Help Workshop\\hhc.exe\" %s.hhp" % base
    else:
        cmd = "hhc \"%s.hhp\"" % base
    print(cmd)
    os.system(cmd)
    if not KEEPTEMP:
        if os.path.isfile("%s.hhp" % base): os.remove("%s.hhp" % base)
        if os.path.isfile("%s.hhc" % base): os.remove("%s.hhc" % base)
        if os.path.isfile("%s.hhk" % base): os.remove("%s.hhk" % base)
        if os.path.isfile("%s.chw" % base): os.remove("%s.chw" % base)
    if not os.path.isfile(base + ".chm"):
        print("An error has occurred!")
        if __name__ == "__main__":
            exit(1)
        else:
            return False
    if __name__ != "__main__":
        return True

def makeManualCHM(outputfile, dirname, title):
    """Same as makeCHM, but suitable for converting the Panda3D manual."""
    return makeCHM(outputfile, dirname, title, special = "manual")

def makeReferenceCHM(outputfile, dirname, title):
    """Same as makeCHM, but converts a structure resembling that of the Panda3D reference."""
    return makeCHM(outputfile, dirname, title, special = "reference")

if __name__ == "__main__":
    # Extract a version number, if we have one.
    VERSION = None
    try:
        f = file("built/include/pandaVersion.h","r")
        pattern = re.compile('^\\s*[#]\\s*define\\s+PANDA_VERSION_STR\\s+["]([0-9.]+)["]')
        for line in f:
            match = pattern.match(line,0)
            if (match):
                VERSION = match.group(1)
                break
        f.close()
    except:
        # If not, we don't care at all.
        pass

    # Now, make CHM's for both the manual and reference, if we have them.
    for lang in ["python", "cxx"]:
        if not os.path.isdir("manual-" + lang):
            print("No directory named 'manual-%s' found" % lang)
        else:
            print("Making CHM file for manual-%s..." % lang)
            if VERSION == None:
                makeManualCHM("manual-%s.chm" % lang, "manual-" + lang, "Panda3D Manual")
            else:
                makeManualCHM("manual-%s-%s.chm" % (VERSION, lang), "manual-" + lang, "Panda3D %s Manual" % VERSION)

        if not os.path.isdir("reference-" + lang):
            print("No directory named 'reference-%s' found" % lang)
        else:
            print("Making CHM file for reference-%s..." % lang)
            if VERSION == None:
                makeReferenceCHM("reference-%s.chm" % lang, "reference-" + lang, "Panda3D Reference")
            else:
                makeReferenceCHM("reference-%s-%s.chm" % (VERSION, lang), "reference-" + lang, "Panda3D %s Reference" % VERSION)

    print("Done!")

