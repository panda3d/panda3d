'''
Various little utilities used by topic-related modules. 

:copyright: Copyright 2006-2009 by Oliver Schoenborn, all rights reserved.
:license: BSD, see LICENSE.txt for details.

'''

from textwrap import TextWrapper, dedent


__all__ = []


UNDERSCORE = '_' # topic name can't start with this
# just want something unlikely to clash with user's topic names
ALL_TOPICS = 'ALL_TOPICS'


class WeakNone:
    '''Pretend to be a weak reference to nothing. Used by ArgsInfos to
    refer to parent when None so no if-else blocks needed. '''
    def __call__(self):
        return None


def smartDedent(paragraph):
    '''
    Dedents a paragraph that is a triple-quoted string. If the first
    line of the paragraph does not contain blanks, the dedent is applied
    to the remainder of the paragraph. This handles the case where a user
    types a documentation string as 
    
    """A long string spanning
    several lines."""
    
    Regular textwrap.dedent() will do nothing to this text because of the 
    first line. Requiring that the user type the docs as """\ with line 
    continuation is not acceptable. 
    '''
    if paragraph.startswith(' '):
        para = dedent(paragraph)
    else:
        lines = paragraph.split('\n')
        exceptFirst = dedent('\n'.join(lines[1:]))
        para = lines[0]+exceptFirst
    return para


class TopicNameInvalid(RuntimeError):
    def __init__(self, name, reason):
        msg = 'Topic name %s invalid: %s' % (name, reason)
        RuntimeError.__init__(self, msg)


import re
_validNameRE = re.compile(r'[a-zA-Z]\w*')


def validateName(topicName):
    '''Raise TopicNameInvalid if nameTuple not valid as topic name.'''
    topicNameTuple = tupleize(topicName)
    if not topicNameTuple:
        reason = 'name tuple must have at least one item!'
        raise TopicNameInvalid(None, reason)

    class topic: pass
    for subname in topicNameTuple:
        if not subname:
            reason = 'can\'t contain empty string or None'
            raise TopicNameInvalid(topicNameTuple, reason)

        if subname.startswith(UNDERSCORE):
            reason = 'must not start with "%s"' % UNDERSCORE
            raise TopicNameInvalid(topicNameTuple, reason)

        if subname == ALL_TOPICS:
            reason = 'string "%s" is reserved for root topic' % ALL_TOPICS
            raise TopicNameInvalid(topicNameTuple, reason)

        if _validNameRE.match(subname) is None:
            reason = 'element #%s ("%s") has invalid characters' % \
                (1+list(topicNameTuple).index(subname), subname)
            raise TopicNameInvalid(topicNameTuple, reason)


def stringize(topicNameTuple):
    '''If topicName is a string, do nothing and return it 
    as is. Otherwise, convert it to one, using dotted notation,
    i.e. ('a','b','c') => 'a.b.c'. Empty name is not allowed 
    (ValueError). The reverse operation is tupleize(topicName).'''
    if isinstance(topicNameTuple, str):
        return topicNameTuple
    
    try:
        name = '.'.join(topicNameTuple)
    except Exception, exc:
        raise TopicNameInvalid(topicNameTuple, str(exc))
    
    return name


def tupleize(topicName):
    '''If topicName is a tuple of strings, do nothing and return it 
    as is. Otherwise, convert it to one, assuming dotted notation 
    used for topicName. I.e. 'a.b.c' => ('a','b','c'). Empty 
    topicName is not allowed (ValueError). The reverse operation 
    is stringize(topicNameTuple).'''
    # assume name is most often str; if more often tuple, 
    # then better use isinstance(name, tuple)
    if isinstance(topicName, str): 
        topicTuple = tuple(topicName.split('.'))
    else:
        topicTuple = tuple(topicName) # assume already tuple of strings
        
    if not topicTuple:
        raise TopicNameInvalid(topicTuple, "Topic name can't be empty!")
                
    return topicTuple


def versionedImport(modName, g, *varNames, **fromNames):
    '''Import a versioned pubsub module, ie one that has API different
    depending on message protocol used. Example: say a module 'foo.py' does

        from topicutils import versionedImport
        versionedImport('moduleName', globals(), 'class1', obj2='obj1')

    This will import the correct version of 'moduleName' and put its 'class1'
    and 'obj1' objects into foo's module namespace such that foo.class1
    and foo.obj2 will be the correct class1 and obj1 for messaging protocol
    used in pubsub.
    '''
    # associate message protocol to file suffixes
    impVers = dict(
        kwargs  = 'p2',
        arg1 = 'p1')
    # create real module name to import
    import policies
    versionStr = impVers[policies.msgDataProtocol]
    fullModName = "%s_%s" % (modName, versionStr)
    # do import
    modObj = __import__(fullModName)
    
    def impSymbol(nameInG, nameInMod = None):
        nameInMod = nameInMod or nameInG # assume same if not specified
        varVal = getattr(modObj, nameInMod, None)
        if varVal is None:
            raise ValueError('No "%s" in %s' % (nameInMod, modObj))
        g[nameInG] = varVal

    # import given symbols (if any) into g dict
    for varName in varNames:
        impSymbol(varName)
    for nameInG, nameInMod in fromNames.iteritems():
        impSymbol(nameInG, nameInMod)

