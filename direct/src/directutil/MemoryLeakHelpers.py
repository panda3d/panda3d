
# Documentation
# http://www.python.org/doc/2.2.3/lib/module-gc.html
# http://www.python.org/~jeremy/weblog/030410.html

# Before you chase down leaks, make sure you Config:
# want-dev 0

# You may also want to run pyo-shell so __debug__ is False and assert code is removed.

import gc
gc.set_debug(gc.DEBUG_LEAK)
gc.collect()
print gc.garbage

# Inside DistributedObjectAI, you can uncomment the __del__ function to
# see when your objects are being deleted (or not)
