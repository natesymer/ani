Ani
==

Imperative JVM language & compiler that resembles C++/Java. Ani is designed
to help teach people how to program. A language specification is available in
`language.md`.

Using Ani
==

Run `make`, then:

    ./ani -c infile.decaf # output build products to current directory
    ./ani -c infile.decaf /tmp # output build products to /tmp

Future
==

Ani is a work in progress. It is extremely limited, but aims to be a complete
language. Below are a number of improvements that will be made in the near
future:

- [ ] Exceptions
- [ ] Ensure builtins have the same calling semantics as user-defined functions
- [ ] Java interop
  - possible syntax: `JavaVirtual(GetStatic("java/lang/System/out", "Ljava/io/PrintStream"), "java/io/PrintStream/println(I)V", -2000);`
  - eliminate built-in functions `Print`, `ReadLine`, `ReadInteger`
- [ ] Generalize the implementation of arrays and strings
- [ ] More string functionality
  - [ ] Character support
  - [ ] Escape sequences
  - [ ] String manipulation
- [ ] Modularity, `import`, and files
- [ ] Class improvements
  - [x] super
  - [ ] initializers (e.g. `New(MyClass, "Arg 1", 2.0, 3)`)
    - [ ] overloaded initializers
  - [ ] Class methods & variables
  - [ ] Public/private distinction
- [ ] Interface array compatibility
  - If a class `Foo` implements an interface `Bar`, you can do `Bar[] x = NewArray(1, Bar);`
- [ ] Function improvements
  - [ ] Mix variable declarations and expressions in functions
  - [ ] Declare and assign variables in the same statement
  - [ ] Overloaded functions & methods
  - [ ] Functions as first-class citizens
- [ ] direct JVM bytecode output, no more Jasmin
- [ ] `else if`
- [x] better CLI interface
- [x] fix bison 3.0 compatibility