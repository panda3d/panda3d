Panda3D C++ Style Guide
=======================

Rather than try to justify every decision made, this document attempts to be as
at-a-glance as possible.  Examples provided where necessary to illustrate best
practices.

Note that this style guide is, for the most part, not very strict, and there
are exceptions to every rule.  However, this guide does serve to illustrate the
preferred style for the entire codebase.

Naming conventions
------------------

- **Classes**: `CamelCaseFirstUpper`
- **Filenames**: `lowerCamelCase.h`, `lowerCamelCase.cxx`
  - Note, every header file must have a unique filename.  They are all
    installed into the same path.
- **Functions**: `lower_snake_case_without_prefix`
- **Local variables**: `lower_snake_case_without_prefix`
- **Member variables**: `_lower_snake_case_with_initial_score`

Indentation
-----------

Always use two spaces to indent.  Tabs should never appear in a C++ file.

Access modifiers (`public`/`private`/`protected`/`PUBLISHED`) are never
indented, and do not affect the indentation of the surrounding lines.

Spaces
------

Always put one space after a control keyword, before the condition's
parentheses.  Don't pad the inside of parentheses with spaces unless there are
several nested parenthetical expressions and the extra spaces contribute
substantially to readability.

    if (x == y) {

Don't place a space after a function name (declarations, definitions, or calls).

    void function_name();
    ...
    function_name();

Always place a space after each comma in an arguments list.

    function_call(x, y, z);

Always pad binary operators with a space on either side.

    a || b

Never end a line with trailing whitespace.

Function definitions
--------------------

The recipe for a function definition is:

1. JavaDoc comment describing the purpose of the function
2. Function return value and class
3. **At the beginning of its own line,** the function's name, followed by the
   parameter list and anything else necessary for the function's signature.
4. Opening brace on the same line, not its own line.
5. Single empty line before next definition.

For example:

    /**
     * String documenting the function, in JavaDoc style.
     */
    void ClassName::
    function_name(int x, int y) const {
      ...
    }

    /**
     * ...
     */
    ...

If the function is a constructor with an initializer list, the `:` goes on the
same line as the constructor name, and the initializers are listed *one per
line*, indented by two spaces.

    ClassName::
    ClassName() :
      _a("a"),
      _b("b"),
      _c("c")
    {
    }

Empty function bodies in a .cxx/.I file still contain a single line break (i.e.
to put `}` on its own line).  In a .h file, an empty function can be given as
`{}`

Braces
------

Always put an opening `{` on the same line as the statement that calls for the
brace.  There is exactly one exception to this: constructors with initializer
lists.

    SomeClass::
    SomeClass() :
      _one(1),
      _two(2)
    {
      if (_one < _two) {
        ...
      }
    }

Comments
--------

Comments should be properly-formatted English, with proper capitalization and
punctuation rules.  There are two spaces between sentences.

Note that `//` is preferred over `/*` except in some special circumstances.

Line limits and continuations
-----------------------------

Lines should be limited to 80 columns.  This is not a hard limit, but if a line
does go over this length, consider a continuation.

Continuation lines shall be aligned to the same column as appropriate, or
indented two spaces otherwise:

    if ((a &&
         b) ||
	c) {
      ...
    }

When choosing where to break a line, these are the preferred places, from best
to worst:

1. If inside a comment, break the comment to as many lines as appropriate, but
   only after moving the comment to its own line. (In other words, comments
   long enough to need 
2. After a comma in an argument/parameter list.
3. After a binary operator (leaving the binary operator at the end of the line)
4. In a function declaration, after the function's return type but before the
   function's name. (Indented by two spaces.)
5. In a log statement with `<<`, before a `<<`. (Avoid ending a line with `<<`)
6. Around a string literal, at a natural word boundary, with a space left on
   the previous line (e.g. `"This is a "` / `"continued string literal."`)
7. After the `=` in an assignment operator.
8. As a last resort, after the opening `(` in a function call.

If none of these rules can apply cleanly, it is okay to have lines extend past
the 80-column boundary.

Alignment
---------

TBD, should discuss:

1. Groups of declarations/definitions
2. Groups of comments
3. Pointers always being `obj *x;` and never `obj* x;`

Grouping
--------

Try to group logically-similar lines, separating them with a single blank line.

Modern language features
------------------------

Panda3D is a C++11 project.  The use of the following modern language features
is greatly encouraged:

1. `nullptr` over `NULL`
2. `auto` (when used to store a function return value only)
3. Range-based for loops

The following modern language features are discouraged:

1. C++17 features
2. `std::atomic`
3. Lambda functions
