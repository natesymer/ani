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

- [ ] Java interop - eliminate some built-in functions like `Print`.
- [ ] Generalize the implementation of arrays and strings
- [ ] More string functionality
  - [ ] Character support
  - [ ] Escape sequences
  - [ ] String manipulation
- [ ] Modularity, `import`, and files
- [ ] Class improvements
  - [ ] Class methods & variables
  - [ ] Public/private distinction
- [ ] Functions as first-class citizens
- [ ] Mix variable declarations and expressions in functions
- [ ] Declare and assign variables in the same statement
- [ ] Overloaded functions
- [ ] Functions as first-class citizens
- [x] super
- [ ] initializers (colled something like `New(MyClass, "Arg 1", 1, 3)`)
- [ ] direct JVM bytecode output, no more Jasmin
- [x] better CLI interface
- [x] fix bison 3.0 compatibility