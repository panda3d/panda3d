// Syntax Highlighting Test File for Vala
// Comments are like this
/* Multiline comments are like
 * this.
 */

// Hello World in Vala
using GLib;

//! \summary Documentation keyword
public class Sample : Object {

    // Some Variable definitions
    public static const double ASPECT = 8.0/6.0;
    static unichar a_char = 'a';

    public Sample () {
    }

    public void run () {
            stdout.printf ("Hello World\n");
            stdout.printf ("Unclosed string);
            stdout.printf ('a'); // <- Char
    }

    static int main (string[] args) {
            var sample = new Sample ();
            sample.run ();
            return 0;
    }
}
