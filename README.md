ani
==

Compiler for a language based on decaf (see decaf.pdf).

goals
==

To have a language suitable for teaching people how to code.

running
==

Run `make`, then:

    ./ani -c infile.decaf # output build products to current directory
    ./ani -c infile.decaf /tmp # output build products to /tmp

future
==

[ ] Overloaded functions
[ ] Functions as first-class citizens
[ ] super
[ ] initializers (Probably something like `New(MyClass, "Arg 1", 1, 3)`)
[ ] direct JVM bytecode output, no more Jasmin
[x] better CLI interface
[x] fix bison 3.0 compatibility