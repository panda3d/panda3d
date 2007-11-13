from direct.task import Task
from direct.http import WebRequest
from direct.directnotify import DirectNotifyGlobal

class webNotifyDebug:
    def __init__(self, portNumber = 8888):

        self.portNumber = portNumber
        self.web = WebRequest.WebRequestDispatcher()
        self.web.listenOnPort(int(self.portNumber))
        # 'debug' will be the name of the page we have to hit
        self.web.registerGETHandler('debug', self.debug)
        self.startCheckingIncomingHTTP()

    def listAllCategories(self, replyTo, optionalMessage = None):
        # Return a web page with a list of all registered notify categories
        # along with an HTML widget to chage their debug state

        completeList = DirectNotifyGlobal.directNotify.getCategories()

        # Define the static head of the response

        if not optionalMessage:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - List All Categories</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n'
        else:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - List All Categories</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><HR>%s<HR><BR><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n' % (optionalMessage)

        # define the static foot

        foot = '</tbody></table></CENTER><BR><A HREF="debug">Main Menu</a></body></html>'

        # Sort our catagory list into alpha order

        completeList.sort()

        # Now generate the body of the page response

        body = ''
        for item in completeList:
            select = '<tr><td>%s</td><td style="text-align: center;">' % (item)
            tempCategory = DirectNotifyGlobal.directNotify.getCategory(item)
            debugStatus = tempCategory.getDebug()
            if debugStatus == 0:
                body = '%s%s<A HREF="debug?command=on&item=%s">Off</a></td></tr>' % (body, select, item)
            else:
                body = '%s%s<A HREF="debug?command=off&item=%s">On</a></td></tr>' % (body, select, item)

        replyTo.respond('%s\n%s\n%s\n' % (head, body, foot))

    def turnCatOn(self, item, replyTo, sString = None):
        # Used to turn a catagory (item), to the on state
        try:
            notifyItem = DirectNotifyGlobal.directNotify.getCategory(item)
            notifyItem.setDebug(1)
            updateMessage = 'Category <b>%s</b>, has been turned on' % (item)
            if not sString:
                self.listAllCategories(replyTo, updateMessage)
            else:
                self.searchForCat(sString, replyTo, updateMessage)
        except AttributeError:
            replyTo.respond('Invalid Category Passed')

    def turnCatOff(self, item, replyTo, sString = None):
        # Used to turn a catagory (item), to the off state
        try:
            notifyItem = DirectNotifyGlobal.directNotify.getCategory(item)
            notifyItem.setDebug(0)
            updateMessage = 'Category <b>%s</b>, has been turned off' % (item)
            if not sString:
                self.listAllCategories(replyTo, updateMessage)
            else:
                self.searchForCat(sString, replyTo, updateMessage)
        except AttributeError:
            replyTo.respond('Invalid Category Passed')

    def searchForCat(self, searchString, replyTo, toggle = None):
        # Used to execute a substring search for a category
        completeList = DirectNotifyGlobal.directNotify.getCategories()
        resultList = []
        while completeList:
            item = completeList.pop()
            if item.find(searchString) != -1:
                resultList.append(item)
        # Now that we have the results, present them
        # First, sort the list
        resultList.sort()
        if not toggle:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - Search Results</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n'
        else:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - Search Results</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><HR>%s<HR><br><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n' % (toggle)
        foot = '</tbody></table></CENTER><BR><A HREF="debug">Main Menu</a></body></html>'
        body = ''
        for item in resultList:
            select = '<tr><td>%s</td><td style="text-align: center;">' % (item)
            tempCategory = DirectNotifyGlobal.directNotify.getCategory(item)
            debugStatus = tempCategory.getDebug()
            if debugStatus == 0:
                body = '%s%s<A HREF="debug?command=on&item=%s&sString=%s">Off</a></td></tr>' % (body, select, item, searchString)
            else:
                body = '%s%s<A HREF="debug?command=off&item=%s&sString=%s">On</a></td></tr>' % (body, select, item, searchString)

        replyTo.respond('%s\n%s\n%s\n' % (head, body, foot))




    def debug(self, replyTo, **kw):
        try:
            command = kw['command']
            if command == 'listAll':
                self.listAllCategories(replyTo)
            elif command == 'on':
                item = kw['item']
                try:
                    sString = kw['sString']
                    self.turnCatOn(item, replyTo, sString)
                except KeyError:
                    self.turnCatOn(item, replyTo)
            elif command == 'off':
                item = kw['item']
                try:
                    sString = kw['sString']
                    self.turnCatOff(item, replyTo, sString)
                except KeyError:
                    self.turnCatOff(item, replyTo)
            elif command == 'search':
                searchString = kw['searchString']
                self.searchForCat(searchString, replyTo)
            else:
                replyTo.respond('Error: Invalid args')
            return
        except KeyError:
            pass
        # Basic Index Page
        
        replyTo.respond('<html><head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify Web Interface</title>\n</head>\n<body>\n<div style="text-align: center;">\n<h1>DirectNotify Web Interface</h1>\n</div>\n<hr style="width: 100%; height: 2px;">\n<form method="get" action="debug" name="searchfom"><INPUT TYPE=HIDDEN NAME="command" VALUE="search">Search for a DirectNotify Category: <input name="searchString"> <input type=submit name="Submit"></button><br>\n</form>\n<br>\n<A HREF="debug?command=listAll">Display all DirectNotify Categories</a>\n</body>\n</html>')


    def startCheckingIncomingHTTP(self):
        taskMgr.remove('pollDirectDebugHTTPTask')
        taskMgr.doMethodLater(0.3,self.pollDirectDebugHTTPTask,'pollDirectDebugHTTPTask')

    def stopCheckingIncomingHTTP(self):
        taskMgr.remove('pollDirectDebugHTTPTask')

    def pollDirectDebugHTTPTask(self,task):
        self.web.poll()
        return Task.again
