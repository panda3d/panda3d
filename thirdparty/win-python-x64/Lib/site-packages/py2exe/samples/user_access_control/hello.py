import sys
import optparse

def main():
    parser = optparse.OptionParser()
    # Main point here is showing that writing to stderr
    # currently forces py2exe to write a .log file next
    # to the executable, but that might not always work.
    # README.txt relies on that to demonstrate different
    # behaviour for the different manifest values, so
    # if this changes, be sure to also ensure README.txt 
    # stays accurate.
    parser.add_option("-e", "--write-err",
                      action="store",
                      help="a message to write to stderr.")

    parser.add_option("-o", "--write-out",
                      action="store",
                      help="a message to write to stdout.")

    opts, args = parser.parse_args()
    if opts.write_err:
        sys.stderr.write(opts.write_err + "\n")
    if opts.write_out:
        sys.stderr.write(opts.write_out+ "\n")

if __name__=='__main__':
    main()
