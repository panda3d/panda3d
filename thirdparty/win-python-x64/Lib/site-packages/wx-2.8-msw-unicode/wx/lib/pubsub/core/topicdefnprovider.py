'''
Everything that has to do with topic definition tree import/export. 

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''


import os, re, inspect
from textwrap import TextWrapper, dedent

import policies
from topicargspec import topicArgsFromCallable, ArgSpecGiven
from topictreetraverser import TopicTreeTraverser

IMPORT_MODULE = 'module'
IMPORT_STRING = 'string'
IMPORT_CLASS  = 'class'


# method name assumed to represent Topic Message Data Specification
SPEC_METHOD_NAME = 'msgDataSpec'


class ITopicDefnDeserializer:
    '''
    All functionality to convert a topic tree representation into a
    set of topic definitions that can be used by a topic definition
    provider.
    '''

    class TopicDefn:
        '''Encapsulate date for a topic definition. Returned by
        getNextTopic().'''

        def __init__(self, nameTuple, description, argsDocs, required):
            self.nameTuple = nameTuple
            self.description = description
            self.argsDocs = argsDocs
            self.required = required

        def isComplete(self):
            return (self.description is not None) and (self.argsDocs is not None)

    def getTreeDoc(self):
        '''Get the documentation for the topic tree. This will be
        interpreted differently based on the type of definition provider. '''
        raise NotImplementedError

    def getNextTopic(self):
        '''Override this to provide the next topic definition available
        from the data. The return must be an instance of TopicDefn.'''
        raise NotImplementedError

    def doneIter(self):
        '''This will be called automatically by the definition provider once
        it considers the iteration completed. Override this only if your
        deserializer needs to do something, such as close a file.
        '''
        pass

    def resetIter(self):
        '''May be called by the definition provider if needs to
        restart the iteration. Override this only if something
        special must be done such as resetting a file point to
        beginning etc. '''
        pass


class TopicDefnDeserialClass(ITopicDefnDeserializer):
    '''
    Interpret a class tree as a topic definition tree. The class name is the
    topic name, its doc string is its description. A method called the same 
    as SPEC_METHOD_NAME is inpsected to infer the optional and required
    message arguments that all listeners must accept. The doc string of that
    method is parsed to extract the description for each argument.
    '''

    def __init__(self, pyClassObj=None):
        '''If pyClassObj is given, it is a class that contains nested
        classes defining root topics; the root topics contain nested
        classes defining subtopics; etc. Hence the init calls
        addDefnFromClassObj() on each nested class found in pyClassObj. '''
        self.__rootTopics = []
        self.__iterStarted = False
        self.__nextTopic = iter(self.__rootTopics)
        self.__rootDoc = None

        if pyClassObj is not None:
            self.__rootDoc = pyClassObj.__doc__
            topicClasses = self.__getTopicClasses(pyClassObj)
            for topicName, pyClassObj in topicClasses:
                self.addDefnFromClassObj(pyClassObj)

    def getTreeDoc(self):
        '''Returns the first doc string that was found in the pyClassObj
        given to self. '''
        return self.__rootDoc

    def addDefnFromClassObj(self, pyClassObj):
        '''Use pyClassObj as a topic definition written using "Python classes".
        The class name is the topic name, assumed to be a root topic, and
        descends recursively down into nested classes. '''
        if self.__iterStarted:
            raise RuntimeError('addClassObj must be called before iteration started!')

        parentNameTuple = (pyClassObj.__name__, )
        if pyClassObj.__doc__ is not None:
            self.__rootTopics.append( (parentNameTuple, pyClassObj) )
            if self.__rootDoc is None:
                self.__rootDoc = pyClassObj.__doc__
        self.__findTopics(pyClassObj, parentNameTuple)
        # iterator is now out of sync, so reset it; obviously this would
        # screw up getNextTopic which is why we had to test for self.__iterStarted
        self.__nextTopic = iter(self.__rootTopics)

    def getNextTopic(self):
        '''Get next topic defined by this provider. Returns None when
        no topics are left. May call resetIter() to restart the iteration.'''
        self.__iterStarted = True
        try:
            topicNameTuple, topicClassObj = self.__nextTopic.next()
        except StopIteration:
            return None

        # ok get the info from class
        if hasattr(topicClassObj, SPEC_METHOD_NAME):
            protoListener = getattr(topicClassObj, SPEC_METHOD_NAME)
            argsDocs, required = topicArgsFromCallable(protoListener)
            if protoListener.__doc__:
                self.__setArgsDocsFromProtoDocs(argsDocs, protoListener.__doc__)
        else:
            # assume definition is implicitly that listener has no args
            argsDocs = {}
            required = ()
        desc = None
        if topicClassObj.__doc__:
            desc = dedent(topicClassObj.__doc__)
        return self.TopicDefn(topicNameTuple, desc, argsDocs, required)

    def resetIter(self):
        self.__iterStarted = False
        self.__nextTopic = iter(self.__rootTopics)

    def getDefinedTopics(self):
        return [nt for (nt, defn) in self.__rootTopics]

    def __findTopics(self, pyClassObj, parentNameTuple=()):
        assert not self.__iterStarted
        if parentNameTuple: # sanity check
            assert pyClassObj.__name__ == parentNameTuple[-1]

        topicClasses = self.__getTopicClasses(pyClassObj, parentNameTuple)

        # make sure to update rootTopics BEFORE we recurse, so that toplevel
        # topics come first in the list
        for parentNameTuple2, topicClassObj in topicClasses:
            # we only keep track of topics that are documented, so that
            # multiple providers can co-exist without having to duplicate
            # information
            if topicClassObj.__doc__ is not None:
                self.__rootTopics.append( (parentNameTuple2, topicClassObj) )
            # now can find its subtopics
            self.__findTopics(topicClassObj, parentNameTuple2)

    def __getTopicClasses(self, pyClassObj, parentNameTuple=()):
        '''Returns a list of pairs, (topicNameTuple, memberClassObj)'''
        memberNames = dir(pyClassObj)
        topicClasses = []
        for memberName in memberNames:
            member = getattr(pyClassObj, memberName)
            if inspect.isclass( member ):
                topicNameTuple = parentNameTuple + (memberName,)
                topicClasses.append( (topicNameTuple, member) )
        return topicClasses

    def __setArgsDocsFromProtoDocs(self, argsDocs, protoDocs):
        PAT_ITEM_STR = r'\A-\s*' # hyphen and any number of blanks
        PAT_ARG_NAME = r'(?P<argName>\w*)'
        PAT_DOC_STR  = r'(?P<doc1>.*)'
        PAT_BLANK    = r'\s*'
        PAT_ITEM_SEP = r':'
        argNamePat = re.compile(
            PAT_ITEM_STR + PAT_ARG_NAME + PAT_BLANK + PAT_ITEM_SEP
            + PAT_BLANK + PAT_DOC_STR)
        protoDocs = dedent(protoDocs)
        lines = protoDocs.splitlines()
        argName = None
        namesFound = []
        for line in lines:
            match = argNamePat.match(line)
            if match:
                argName = match.group('argName')
                namesFound.append(argName)
                argsDocs[argName] = [match.group('doc1') ]
            elif argName:
                argsDocs[argName].append(line)

        for name in namesFound:
            argsDocs[name] = '\n'.join( argsDocs[name] )


class TopicDefnDeserialModule(ITopicDefnDeserializer):
    '''
    Deserialize a module containing source code defining a topic tree.
    This loads the module and finds all class definitions in it (at
    module level that is) and uses a TopicDefnDeserialClass to
    deserialize each one into a topic definition.
    '''

    def __init__(self, moduleName, searchPath=None):
        '''Load the given named module, searched for in searchPath or, if not
        specified, in sys.path. The top-level classes will be assumed to be
        topic definitions with a doc string and a message data specification
        method as described in TopicDefnDeserialClass'.
        '''
        import imp
        fp, pathname, description = imp.find_module(moduleName, searchPath)
        try:
            module = imp.load_module(moduleName, fp, pathname, description)
        finally:
            # Since we may exit via an exception, close fp explicitly.
            if fp:
                fp.close()

        self.__classDeserial = TopicDefnDeserialClass(module)
#        self.__moduleDoc = module.__doc__
#        memberNames = dir(module)
#        for memberName in memberNames:
#            member = getattr(module, memberName)
#            if inspect.isclass(member):
#                self.__classDeserial.addDefnFromClassObj(member)

    def getTreeDoc(self):
        return self.__classDeserial.getTreeDoc()
        #return self.__moduleDoc
    
    def getNextTopic(self):
        return self.__classDeserial.getNextTopic()

    def doneIter(self):
        self.__classDeserial.doneIter()

    def resetIter(self):
        self.__classDeserial.resetIter()

    def getDefinedTopics(self):
        return self.__classDeserial.getDefinedTopics()


class TopicDefnDeserialString(ITopicDefnDeserializer):
    '''
    Deserialize a string containing source code defining a topic tree.
    This just saves the string into a temporary file created in os.getcwd(), 
    and the rest is delegated to TopicDefnDeserialModule. The temporary
    file (module) is deleted (as well as its byte-compiled version)
    when the doneIter() method is called.
    '''

    def __init__(self, source):
        def createTmpModule():
            moduleNamePre = 'tmp_export_topics_'
            import os, tempfile
            creationDir = os.getcwd()
            fileID, path = tempfile.mkstemp('.py', moduleNamePre, dir=creationDir)
            stringFile = os.fdopen(fileID, 'w')
            stringFile.write( dedent(source) )
            stringFile.close()
            return path, [creationDir]

        self.__filename, searchPath = createTmpModule()
        moduleName = os.path.splitext( os.path.basename(self.__filename) )[0]
        self.__modDeserial = TopicDefnDeserialModule(moduleName, searchPath)

    def getTreeDoc(self):
        return self.__modDeserial.getTreeDoc()

    def getNextTopic(self):
        return self.__modDeserial.getNextTopic()

    def doneIter(self):
        self.__modDeserial.doneIter()
        # remove the temporary module and its compiled version (*.pyc)
        os.remove(self.__filename)
        os.remove(self.__filename + 'c')

    def resetIter(self):
        self.__modDeserial.resetIter()

    def getDefinedTopics(self):
        return self.__modDeserial.getDefinedTopics()


#########################################################################

class ITopicDefnProvider:
    '''
    All topic definition providers must follow this protocol. They must
    at very least provide a getDefn() method that returns a pair
    (string, ArgSpecGiven), or (None, None). The first member is a
    description for topic, and second one contains the listener callback
    protocol. See note in MasterTopicDefnProvider about what *it*
    returns based on the return of getDefn().
    '''
    
    def getDefn(self, topicNameTuple):
        return 'From incompletely implemented PROVIDER', ArgSpecGiven()

    def topicNames(self):
        '''Return an iterator over topic names available from this provider.
        Note that the topic names should be in tuple rather than dotted-string
        form so as to be compatible with getDefn().'''
        msg = 'Must return a list of topic names available from this provider'
        raise NotImplementedError(msg)

    def __iter__(self):
        '''Same as self.topicNames().'''
        return self.topicNames()


class TopicDefnProvider(ITopicDefnProvider):
    '''
    Default implementation of the ITopicDefnProvider API. This
    implementation accepts several formats for the source data
    and delegates to suitable parser that knows how to convert
    source data into a topic definition.

    You can create your own topic definition provider classes,
    for formats (say, XML) not supported by TopicDefnProvider.
    See also pub.addTopicDefnProvider().
    '''

    typeRegistry = {}

    def __init__(self, source, format=IMPORT_MODULE):
        providerClassObj = self.typeRegistry[format]
        provider = providerClassObj(source)
        self.__topicDefns = {}
        self.__treeDocs = provider.getTreeDoc()
        try:
            topicDefn = provider.getNextTopic()
            while topicDefn is not None:
                self.__topicDefns[topicDefn.nameTuple] = topicDefn
                topicDefn = provider.getNextTopic()
        finally:
            provider.doneIter()

    def getTreeDoc(self):
        return self.__treeDocs

    def getDefn(self, topicNameTuple):
        desc, spec = None, None
        defn = self.__topicDefns.get(topicNameTuple, None)
        if defn is not None:
            assert defn.isComplete()
            desc = defn.description
            spec = ArgSpecGiven(defn.argsDocs, defn.required)
        return desc, spec

    def topicNames(self):
        return self.__topicDefns.iterkeys()


def registerTypeForImport(typeName, providerClassObj):
    TopicDefnProvider.typeRegistry[typeName] = providerClassObj

registerTypeForImport(IMPORT_MODULE, TopicDefnDeserialModule)
registerTypeForImport(IMPORT_STRING, TopicDefnDeserialString)
registerTypeForImport(IMPORT_CLASS,  TopicDefnDeserialClass)


#########################################################################


class MasterTopicDefnProvider:
    '''
    Stores a list of topic definition providers. When queried for a topic
    definition, queries each provider (registered via addProvider()) and
    returns the first complete definition provided, or (None,None).

    The providers must follow the ITopicDefnProvider protocol.
    '''

    def __init__(self, treeConfig):
        self.__providers = []
        self.__treeConfig = treeConfig

    def addProvider(self, provider):
        '''Add given provider IF not already added; returns how many
        providers have been registered, ie if new provider, will be
        1 + last call's return, otherwise (provider had already been added)
        then will be same as last call's return value. '''
        if provider not in self.__providers:
            self.__providers.append(provider)
        return len(self.__providers)

    def clear(self):
        self.__providers = []

    def getNumProviders(self):
        return len(self.__providers)

    def getDefn(self, topicNameTuple):
        '''Returns a pair (string, ArgSpecGiven), or (None,None) if a
        complete definition was not available from any of the registered topic
        definition providers. The first item is a description string for the
        topic, the second is an instance of ArgSpecGiven specifying the
        listener protocol required for listeners of this topic. The
        definition (the returned pair) is complete if the description is
        not None and the second item has isComplete() == True. Hence,
        if the description is None, so is the second item. Alternately,
        if second item, obtained from the provider, has isComplete() == False,
        then return is (None, None).'''
        desc, defn = None, None
        for provider in self.__providers:
            tmpDesc, tmpDefn = provider.getDefn(topicNameTuple)
            if (tmpDesc is not None) and (tmpDefn is not None):
                assert tmpDefn.isComplete()
                desc, defn = tmpDesc, tmpDefn
                break

        return desc, defn

    def isDefined(self, topicNameTuple):
        '''Returns True only if a complete definition exists, ie topic
        has a description and a complete listener protocol specification.'''
        desc, defn = self.getDefn(topicNameTuple)
        if desc is None or defn is None:
            return False
        if defn.isComplete():
            return True
        return False


#########################################################################


def _toDocString(msg):
    if not msg:
        return msg
    if msg.startswith("'''") or msg.startswith('"""'):
        return msg
    return "'''\n%s\n'''" % msg.strip()


class TopicTreeAsSpec:
    '''
    Prints the class representation of topic tree, as Python code
    that can be imported and given to pub.addTopicDefnProvider().
    The printout can be sent to any file object (object that has a
    write() method).

    Example::

        from StringIO import StringIO
        capture = StringIO()
        printer = TopicTreeAsSpec(fileObj=capture)
        printer.traverse(someTopic)

    '''

    INDENT_CH = ' '
    #INDENT_CH = '.'

    def __init__(self, width=70, indentStep=4, treeDoc=None, footer=None, fileObj=None):
        '''Can specify the width of output, the indent step, the header
        and footer to print, and the destination fileObj. If no destination
        file, then stdout is assumed.'''
        self.__traverser = TopicTreeTraverser(self)

        import sys
        self.__destination = fileObj or sys.stdout
        self.__output = []
        self.__header = _toDocString(treeDoc)
        self.__footer = footer
        self.__lastWasAll = False # True when last topic done was the ALL_TOPICS

        self.__width   = width
        self.__wrapper = TextWrapper(width)
        self.__indentStep = indentStep
        self.__indent  = 0

        args = dict(width=width, indentStep=indentStep, treeDoc=treeDoc,
                    footer=footer, fileObj=fileObj)
        def fmItem(argName, argVal):
            if isinstance(argVal, str):
                MIN_OFFSET = 5
                lenAV = width - MIN_OFFSET - len(argName)
                if lenAV > 0:
                    argVal = `argVal[:lenAV] + '...'`
            elif argName == 'fileObj':
                argVal = fileObj.__class__.__name__
            return '# - %s: %s' % (argName, argVal)
        fmtArgs = [fmItem(argName, argVal) for (argName, argVal) in args.iteritems()]
        self.__comment = [
            '# Automatically generated by %s(**kwargs).' % self.__class__.__name__,
            '# The kwargs were:',
        ]
        self.__comment.extend(fmtArgs)
        self.__comment.extend(['']) # two empty line after comment

    def getOutput(self):
        return '\n'.join( self.__output )

    def traverse(self, topicObj):
        self.__traverser.traverse(topicObj)

    def _accept(self, topicObj):
        # accept every topic
        return True

    def _startTraversal(self):
        # output comment
        self.__wrapper.initial_indent = '# '
        self.__wrapper.subsequent_indent = self.__wrapper.initial_indent
        self.__output.extend( self.__comment )

        # output header:
        if self.__header:
            self.__output.extend([''])
            self.__output.append(self.__header)
            self.__output.extend([''])

    def _doneTraversal(self):
        if self.__footer:
            self.__output.append('')
            self.__output.append('')
            self.__output.append(self.__footer)

        if self.__destination is not None:
            self.__destination.write(self.getOutput())

    def _onTopic(self, topicObj):
        '''This gets called for each topic. Print as per specified content.'''
        # don't print root of tree, it is the ALL_TOPICS builtin topic
        if topicObj.isAll():
            self.__lastWasAll = True
            return
        self.__lastWasAll = False

        self.__output.append( '' ) # empty line
        # topic name
        self.__wrapper.width = self.__width
        head = 'class %s:' % topicObj.getTailName()
        self.__formatItem(head)

        # each extra content (assume constructor verified that chars are valid)
        self.__printTopicDescription(topicObj)
        if policies.msgDataProtocol != 'arg1':
            self.__printTopicArgSpec(topicObj)

    def _startChildren(self):
        '''Increase the indent'''
        if not self.__lastWasAll:
            self.__indent += self.__indentStep

    def _endChildren(self):
        '''Decrease the indent'''
        if not self.__lastWasAll:
           self.__indent -= self.__indentStep

    def __printTopicDescription(self, topicObj):
        if topicObj.getDescription():
            extraIndent = self.__indentStep
            self.__formatItem("'''", extraIndent)
            self.__formatItem( topicObj.getDescription(), extraIndent )
            self.__formatItem("'''", extraIndent)

    def __printTopicArgSpec(self, topicObj):
        extraIndent = self.__indentStep

        # generate the listener protocol
        reqdArgs, optArgs = topicObj.getArgs()
        argsStr = []
        if reqdArgs:
            argsStr.append( ", ".join(reqdArgs) )
        if optArgs:
            optStr = ', '.join([('%s=None' % arg) for arg in optArgs])
            argsStr.append(optStr)
        argsStr = ', '.join(argsStr)

        # print it only if there are args; ie if listener() don't print it
        if argsStr:
            # output a blank line and protocol
            self.__formatItem('\n', extraIndent)
            protoListener = 'def %s(%s):' % (SPEC_METHOD_NAME, argsStr)
            self.__formatItem(protoListener, extraIndent)

            # and finally, the args docs
            extraIndent += self.__indentStep
            self.__formatItem("'''", extraIndent)
            # but ignore the arg keys that are in parent args docs:
            parentMsgKeys = ()
            if topicObj.getParent() is not None:
                parentMsgKeys = topicObj.getParent().getArgDescriptions().keys()
            argsDocs = topicObj.getArgDescriptions()
            for key, argDesc in argsDocs.iteritems():
                if key not in parentMsgKeys:
                    msg = "- %s: %s" % (key, argDesc)
                    self.__formatItem(msg, extraIndent)
            self.__formatItem("'''", extraIndent)

    def __formatItem(self, item, extraIndent=0):
        indent = extraIndent + self.__indent
        indentStr = self.INDENT_CH * indent
        lines = item.splitlines()
        for line in lines:
            self.__output.append( '%s%s' % (indentStr, line) )

    def __formatBlock(self, text, extraIndent=0):
        self.__wrapper.initial_indent = self.INDENT_CH * (self.__indent + extraIndent)
        self.__wrapper.subsequent_indent = self.__wrapper.initial_indent
        self.__output.append( self.__wrapper.fill(text) )


defaultTopicTreeSpecHeader = \
"""
Topic tree for application.
Used via pub.importTopicTree(thisModuleName).
"""

defaultTopicTreeSpecFooter = \
"""\
# End of topic tree definition. Note that application may load
# more than one definitions provider.
"""


def exportTreeAsSpec(rootTopic=None, **kwargs):
    '''Prints the topic tree specification starting from rootTopic.
    If not specified, the whole topic tree is printed. The kwargs are the
    same as TopicTreeAsSpec's constructor: width(70), indentStep(4),
    header(None), footer(None), fileObj. If no header or footer are
    given, the default ones are used (see defaultTopicTreeSpecHeader and
    defaultTopicTreeSpecFooter), such that the resulting output can be
    imported in your application. E.g.::

        pyFile = file('appTopicTree.py','w')
        exportTreeAsSpec( pyFile )
        pyFile.close()
        import appTopicTree
    '''
    # only add header/footer if not already given
    kwargs.setdefault('treeDoc', defaultTopicTreeSpecHeader)
    kwargs.setdefault('footer', defaultTopicTreeSpecFooter)
    
    assert rootTopic is not None

    # print it
    printer = TopicTreeAsSpec(**kwargs)
    printer.traverse(rootTopic)


