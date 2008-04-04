import LandingPageHTML

class LandingPage:
    def __init__(self):
        self.headerTemplate = LandingPageHTML.header
        self.footerTemplate = LandingPageHTML.footer
        
        self.title = LandingPageHTML.title
        self.contactInfo = LandingPageHTML.contactInfo

        self.menu = {}

    def addTab(self, title, uri):
        self.menu[title] = uri

    def getMenu(self, activeTab):
        tabList = self.menu.keys()
        if "Main" in tabList:
            tabList.remove("Main")
        
        tabList.sort()
        if "Main" in self.menu.keys():
            tabList.insert(0, "Main")

        s = ""
        tabNum = 0

        for tab in tabList:
            if tabNum == 0:
                if tab == activeTab:
                    s += "<li id=\"active\" class=\"first\"><a href=\"%s\" id=\"current\">%s</a></li>\n" % \
                         (self.menu[tab], tab)
                else:
                    s += "<li class=\"first\"><a href=\"%s\">%s</a></li>\n" % \
                         (self.menu[tab], tab)
            else:
                if tab == activeTab:
                    s += "<li id=\"active\"><a href=\"%s\" id=\"current\">%s</a></li>\n" % \
                         (self.menu[tab], tab)
                else:
                    s += "<li><a href=\"%s\">%s</a></li>\n" % \
                         (self.menu[tab], tab)
            tabNum += 1

        return s
                

    def getHeader(self, activeTab = "Main"):
        s = self.headerTemplate % {'titlestring' : self.title,
                                   'menustring' : self.getMenu(activeTab)}
        return s
        

    def getFooter(self):
        return self.footerTemplate % {'contact' : self.contactInfo}

    def listHandlerPage(self, uriToHandler):
        output = self.getHeader("Services")

        uriList = uriToHandler.keys()
        uriList.sort()

        output += "<table>\n<caption>Services</caption><thead><tr><th scope=col>URI</th><th scope=col>Handler</th></tr></thead>\n\n"
        output += "<tbody>\n"

        rowNum = 0
        for uri in uriList:
            rowNum += 1
            handlerFunc = str(uriToHandler[uri][0]).split(" ")[2]

            output += "<tr%s><td><a href=%s>%s</a></td><td>%s</td></tr>\n" % \
                      (LandingPageHTML.getRowClassString(rowNum),
                       uri,
                       uri,
                       handlerFunc)
                       #handlerFunc)
            
        output += "</tbody></table>\n"

        output = output + self.getFooter()
        return output

    def main(self):
        output = self.getHeader("Main") + "<P>Welcome!</P>\n" + self.getFooter()
        return output

