from __future__ import annotations

import os
import time
from typing import Iterable


class RotatingLog:
    """
    An `open()` replacement that will automatically open and write
    to a new file if the prior file is too large or after a time interval.
    """

    def __init__(
        self,
        path: str = "./log_file",
        hourInterval: int | None = 24,
        megabyteLimit: int | None = 1024,
    ) -> None:
        """
        Args:
            path: a full or partial path with file name.
            hourInterval: the number of hours at which to rotate the file.
            megabyteLimit: the number of megabytes of file size the log may
                grow to, after which the log is rotated.  Note: The log file
                may get a bit larger than limit do to writing out whole lines
                (last line may exceed megabyteLimit or "megabyteGuidline").
        """
        self.path = path
        self.timeInterval = None
        self.timeLimit = None
        self.sizeLimit = None
        if hourInterval is not None:
            self.timeInterval = hourInterval*60*60
            self.timeLimit = time.time()+self.timeInterval
        if megabyteLimit is not None:
            self.sizeLimit = megabyteLimit*1024*1024

    def __del__(self) -> None:
        self.close()

    def close(self) -> None:
        if hasattr(self, "file"):
            self.file.flush()
            self.file.close()
            self.closed = self.file.closed
            del self.file
        else:
            self.closed = True

    def shouldRotate(self) -> bool:
        """
        Returns a bool about whether a new log file should
        be created and written to (while at the same time
        stopping output to the old log file and closing it).
        """
        if not hasattr(self, "file"):
            return True
        if self.timeLimit is not None and time.time() > self.timeLimit:
            return True
        if self.sizeLimit is not None and self.file.tell() > self.sizeLimit:
            return True
        return False

    def filePath(self) -> str:
        dateString = time.strftime("%Y_%m_%d_%H", time.localtime())
        for i in range(26):
            limit = self.sizeLimit
            path = "%s_%s_%s.log" % (self.path, dateString, chr(i+97))
            if limit is None or not os.path.exists(path) or os.stat(path)[6] < limit:
                return path
        # Hmm, 26 files are full?  throw the rest in z:
        # Maybe we should clear the self.sizeLimit here... maybe.
        return path

    def rotate(self) -> None:
        """
        Rotate the log now.  You normally shouldn't need to call this.
        See write().
        """
        path=self.filePath()
        file=open(path, "a")
        if file:
            self.close()
            # This should be redundant with "a" open() mode,
            # but on some platforms tell() will return 0
            # until the first write:
            file.seek(0, 2)
            self.file=file

            # Some of these data members may be expected by some of our clients:
            self.closed = self.file.closed
            self.mode = self.file.mode
            self.name = self.file.name
            #self.encoding = self.file.encoding # Python 2.3
            #self.newlines = self.file.newlines # Python 2.3, maybe

            if self.timeLimit is not None and time.time() > self.timeLimit:
                assert self.timeInterval is not None
                self.timeLimit=time.time()+self.timeInterval
        else:
            # We'll keep writing to the old file, if available.
            print("RotatingLog error: Unable to open new log file \"%s\"." % (path,))

    def write(self, data: str) -> int | None:
        """
        Write the data to either the current log or a new one,
        depending on the return of shouldRotate() and whether
        the new file can be opened.
        """
        if self.shouldRotate():
            self.rotate()
        if hasattr(self, "file"):
            r = self.file.write(data)
            self.file.flush()
            return r
        return None

    def flush(self) -> None:
        return self.file.flush()

    def fileno(self) -> int:
        return self.file.fileno()

    def isatty(self) -> bool:
        return self.file.isatty()

    def __next__(self):
        return next(self.file)
    next = __next__

    def read(self, size):
        return self.file.read(size)

    def readline(self, size):
        return self.file.readline(size)

    def readlines(self, sizehint):
        return self.file.readlines(sizehint)

    def xreadlines(self):
        return self.file.xreadlines()

    def seek(self, offset: int, whence: int = 0) -> int:
        return self.file.seek(offset, whence)

    def tell(self) -> int:
        return self.file.tell()

    def truncate(self, size: int | None) -> int:
        return self.file.truncate(size)

    def writelines(self, sequence: Iterable[str]) -> None:
        return self.file.writelines(sequence)
