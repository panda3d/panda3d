import MySQLdb
from MySQLdb.connections import *

class DirectMySQLdbConnection(Connection):
    ### DCR: from MySQLdb connections.py Connection.__init__
    def __init__(self, *args, **kwargs):
        ### DCR: fixed up relative imports
        from MySQLdb.constants import CLIENT, FIELD_TYPE
        from MySQLdb.converters import conversions
        from weakref import proxy, WeakValueDictionary

        import types

        kwargs2 = kwargs.copy()

        conv = kwargs.get('conv', conversions)

        kwargs2['conv'] = dict([ (k, v) for k, v in conv.items()
                                 if type(k) is int ])

        self.cursorclass = kwargs2.pop('cursorclass', self.default_cursor)
        charset = kwargs2.pop('charset', '')

        if charset:
            use_unicode = True
        else:
            use_unicode = False

        use_unicode = kwargs2.pop('use_unicode', use_unicode)
        sql_mode = kwargs2.pop('sql_mode', '')

        client_flag = kwargs.get('client_flag', 0)
        ### DCR: fixed up module reference
        client_version = tuple([ int(n) for n in MySQLdb.connections._mysql.get_client_info().split('.')[:2] ])
        if client_version >= (4, 1):
            client_flag |= CLIENT.MULTI_STATEMENTS
        if client_version >= (5, 0):
            client_flag |= CLIENT.MULTI_RESULTS

        kwargs2['client_flag'] = client_flag

        ### DCR: skip over the Connection __init__
        #super(Connection, self).__init__(*args, **kwargs2)
        MySQLdb._mysql.connection.__init__(self, *args, **kwargs2)

        self.encoders = dict([ (k, v) for k, v in conv.items()
                               if type(k) is not int ])

        self._server_version = tuple([ int(n) for n in self.get_server_info().split('.')[:2] ])

        db = proxy(self)
        ### DCR: these functions create memory leaks with gc.DEBUG_SAVEALL turned on
        """
        def _get_string_literal():
            def string_literal(obj, dummy=None):
                return db.string_literal(obj)
            return string_literal

        def _get_unicode_literal():
            def unicode_literal(u, dummy=None):
                return db.literal(u.encode(unicode_literal.charset))
            return unicode_literal

        def _get_string_decoder():
            def string_decoder(s):
                return s.decode(string_decoder.charset)
            return string_decoder
        """

        ### DCR: use methods rather than inline-defined functions to prevent memory leaks
        string_literal = self._get_string_literal(db)
        self.unicode_literal = unicode_literal = self._get_unicode_literal(db)
        self.string_decoder = string_decoder = self._get_string_decoder()
        if not charset:
            charset = self.character_set_name()
        self.set_character_set(charset)

        if sql_mode:
            self.set_sql_mode(sql_mode)

        if use_unicode:
            self.converter[FIELD_TYPE.STRING].insert(-1, (None, string_decoder))
            self.converter[FIELD_TYPE.VAR_STRING].insert(-1, (None, string_decoder))
            self.converter[FIELD_TYPE.BLOB].insert(-1, (None, string_decoder))

        self.encoders[types.StringType] = string_literal
        self.encoders[types.UnicodeType] = unicode_literal
        self._transactional = self.server_capabilities & CLIENT.TRANSACTIONS
        if self._transactional:
            # PEP-249 requires autocommit to be initially off
            self.autocommit(False)
        self.messages = []

    ### DCR: make inline-defined functions into member methods to avoid garbage
    def _string_literal(self, db, obj, dummy=None):
        return db.string_literal(obj)
    def _get_string_literal(self, db):
        return Functor(self._string_literal, db)

    def _unicode_literal(self, db, u, dummy=None):
        return db.literal(u.encode(unicode_literal.charset))
    def _get_unicode_literal(self, db):
        return Functor(self._unicode_literal, db)

    def _string_decoder(self, s):
        return s.decode(string_decoder.charset)
    def _get_string_decoder(self):
        # make it into a Functor since MySQLdb.connections.Connection wants to set
        # attributes on its string_decoder
        return Functor(self._string_decoder)

    def close(self):
        Connection.close(self)
        # break garbage cycles
        self.unicode_literal = None
        self.string_decoder = None
        self.encoders = None
