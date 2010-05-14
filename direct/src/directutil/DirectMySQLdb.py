from MySQLdb import *

### DCR: from MySQLdb __init__.py
def Connect(*args, **kwargs):
    """Factory function for connections.Connection."""
    ### DCR: use DirectMySQLdbConnection to prevent memory leaks
    from direct.directutil.DirectMySQLdbConnection import DirectMySQLdbConnection
    return DirectMySQLdbConnection(*args, **kwargs)

connect = Connection = Connect
