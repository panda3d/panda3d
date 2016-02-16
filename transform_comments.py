
import os
import sys

from os.path import isdir, isfile, join, splitext

EXTENSIONS = ['.i', '.I', '.cxx', '.yxx', '.lxx', '.h', '.cpp', '.mm', '.java', '.hh', '.C.template']
INCLUDE_FILES = ['soft2Egg.c']
EXCLUDE_FILES = []



def warn(fn, ln, msg):
    print("%s:%s: warning: %s" % (fn, ln, msg))


def convert_comment(fname, refline, comment_lines):



    return [1]

def fix_comments(abspath):
    with open(abspath, "r") as handle:
        lines = handle.readlines()

    # Check if a conversion error ocured
    content = ''.join(lines)
    if not "@file" in content or not '@author' in content or not '/**' in content:
        # warn(abspath, 0, "No starting comment found!")
        return


    in_comment = False
    comment_lines = []
    code_lines = []

    for i, line in enumerate(lines):
        line = line.rstrip("\r\n")

        if line.startswith("/" * 10):
            if line.count("/") != len(line): # could use re here but would be slower
                warn(abspath, i, "Invalid comment line: '{}'".format(line))
                return # safety first
            if in_comment:
                in_comment = False
                for comment_line in convert_comment(abspath, i, comment_lines):
                    code_lines.append(comment_line)
                continue
            else:
                in_comment = True
                comment_lines = []

        elif in_comment:
            if not line.startswith("//"):
                warn(abspath, i, "Invalid comment line: {}".format(line))
                in_comment = False
            else:
                comment_lines.append(line)

        if not in_comment:
            code_lines.append(line)


if __name__ == "__main__":

    if len(sys.argv) > 1:
        for arg in sys.argv[1:]:
            fix_comments(arg)
    else:
        for dirpath, _, filenames in os.walk("."):
            for filename in filenames:
                abspath = join(dirpath, filename)
                extension = splitext(filename)[1]
                if filename in INCLUDE_FILES or (extension in EXTENSIONS and filename not in EXCLUDE_FILES):
                    fix_comments(abspath)
