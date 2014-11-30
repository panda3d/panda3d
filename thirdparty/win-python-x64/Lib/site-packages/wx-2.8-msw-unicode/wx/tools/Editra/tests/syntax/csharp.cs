// C# Syntax Highlighting Test File
// Comments are like this
/** Multiline comments are like this
 * @summary <- Documentation Keyword
 * \brief <- Doxygen style keyword
 */

using System;

#region Foo
class Foo
{
    void Create() {}
}
#endregion Foo

// Hello World
class HelloClass
{
    static void Main()
    {
        System.Console.WriteLine("Hello, world!");
    }

    public static void PrintPlusOne(int x)
    {
        System.Console.WriteLine(x + 1);
    }
}

