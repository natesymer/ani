Ani Language Specification
=

This document is a reference for the Ani programming language & JVM compiler.

Lexemes
==

An identifier: a string of one or more characters, starting with a letter
               and containing letters, numbers, and underscores.

The following identifiers are reserved as keywords:

`void` `int` `double` `bool`
`string` `class` `interface`
`this` `extends` `implements` `for`
`while` `if` `else` `return`
`break` `New` `NewArray` `Print`
`ReadInteger` `ReadLine` `super`

The following identifiers are reserved as literals:

`true` `false` `null`

An `int`: a sequence of numbers from 0 to 9

A `double`: a sequence of numbers from 0 to 9, followed by a period, followed by a
sequence of characters from 0 to 9.

A `string`: a double quote, followed by a sequence of characters except newlines
and double quotes, followed by a double quote

A single line comment: two slashes, followed by any characters, followed
by a newline.

A multiline comment: A slash, followed by an asterisk, followed by a sequence
of characters, followed by an asterisk and a slash.

Grammar
==

Precedence is as expected in arithmetic. IntConstant, DoubleConstant, BoolConstant,
and StringConstant represent the constants described in the `Lexemes` section. A
comma after a + sign means comma separated. Bold means optional. An italicized *e* means
epsilon, the empty string. Capitalized means non-terminal, lowercase means terminal,
except in `Builtin` where `ReadInteger`, `ReadLine`, `New`, `NewArray`, are terminals.

### Grammar

Program       -> Decl+

Decl          -> VariableDecl | FunctionDecl | ClassDecl | InterfaceDecl

VariableDecl  -> Variable ;

Variable      -> Type ident

Type          -> int | double | bool | string | ident | Type [ ]

FunctionDecl  -> Type ident ( Formals ) StmtBlock | void ident ( Formals ) StmtBlock

Formals       -> Variable+, | *e*

ClassDecl     -> class ident **extends ident** **implements ident+,** { Field\* }

Field         -> VariableDecl | FunctionDecl

InterfaceDecl -> interface ident { Prototype\* }

Prototype     -> Type ident ( Formals ) ; | void ident ( Formals ) ;

StmtBlock     -> { VariableDecl\* Stmt\* }

Stmt          -> <Expr>; | IfStmt | WhileStmt | ForStmt |
                 BreakStmt | ReturnStmt | PrintStmt | StmtBlock

IfStmt        -> if ( Expr ) Stmt **else Stmt**

WhileStmt     -> while ( Expr ) Stmt

ForStmt       -> for ( **Expr**; Expr ; **Expr**) Stmt

ReturnStmt    -> return **Expr** ;

BreakStmt     -> break ;

PrintStmt     -> Print ( Expr+, ) ;

Expr          -> LValue = Expr | Constant | LValue | this | super | Call | ( Expr ) |
                 Expr + Expr | Expr - Expr | Expr \* Expr | Expr / Expr |
                 Expr % Expr | - Expr | Expr < Expr | Expr <= Expr |
                 Expr > Expr | Expr >= Expr | Expr == Expr | Expr ! = Expr |
                 Expr && Expr | Expr || Expr | ! Expr | BuiltinExpr

BuiltinExpr  -> ReadInteger ( ) | ReadLine ( ) | New ( ident ) | NewArray ( Expr , Type )

LValue       -> ident | Expr . ident | Expr [ Expr ]

Call         -> ident ( Actuals ) | Expr . ident ( Actuals )

Actuals      -> Expr+, | *e*

Constant     -> IntConstant | DoubleConstant | BoolConstant | StringConstant | null

Semantic Conerns
==

All programs require a function called `main`. `main` must take either no
arguments, or an array of strings.

Scoping
===

There are multiple levels of scope, in order of precedence (lowest to highest):

1. Global - variables, functions, and classes
2. Class - variables and functions
3. Function arguments - variables
4. local variables - variables

Declarations within a scope must be unique, but can be shadowed in scopes with
higher precendence.

Declarations are available at the same scope level even before the lexical
point at which they are declared. This enables situations like a class extending
another class that is lexically defined after it. This rule applies uniformly to
all declarations (variables, classes, interfaces, functions) in all scopes.

Types
===

Ani is a (mostly) strongly typed language.

Ani supports four builtin types, `string`, `bool`, `int`, and `double`. It also
supports named types: user-defined classes and interfaces. Ani also supports
arrays, whose element types can be any of the previously mentioned types in
addition to array types. Ani also supports a type called `void` solely for
functions that don't return anything.

### Equality

If a type `A` is equivalent to a type `B`, an expression of either type can be
substituted for the other in any situation. Two types are equivalent iff they
are the same exact type. This also applies to arrays (type equivalence is then
checked for the element type, which may be an array - recursive structural
equivalence). Named types are equal iff they share the same name (named
equivalence).

### Compatibility

Compatibility is a unidirectional relationship; If type `A` is compatible with type
`B`, then an expression of type `A` can be substituted where an expression of type
`B` is expected. Nothing is implied about the reverse direction. Two equivalent types are type compatible in both directions. Type equivalent types are also type
compatible.

A subclass is compatible with its parent type, but not the reverse. A class is
compatible with any interfaces it implements. The `null` type is compatible with
all named types. Operations such as assignment and parameter passing allow for
not just equivalent types but compatible types.

The recursive structural equality does not have a parallel in type compatibility.

### Arrays

Arrays are created with the builtin `NewArray(N, type)` where `N` is a positive,
non-zero integer representing the size of the array (number of elements). `type`
is any type other than `void`. Once created, arrays' sizes cannot change.

Arrays' indeces start at 0, and are indexed through the subscript operator, which 
takes a positive integer. Arrays support the `length()` method, which returns
their length. Arrays are passed, assigned, and compared by reference.

Arrays are implemented as java array objects and are therefore references.

### Strings

Strings are assigned and passed by reference, but compared by value. As of now,
there is no support for string manipulation or any advanced string functionality.

Strings are implemented as Java String objects and are therefore references.

### Objects

Objects are implemented as references. Objects are dynamically allocated on the
heap using the `New()` builtin function. The type passed to `New()` should be a
class (not an interface). To access fields of the object, use the `.` operator.
Methods are called via `object.method()` where `object` is a named type
(a class or interface) that implements `method`. The field must be accessible
in the scope in which it's accessed (see Classes section).

Object assignment is shallow (assigning an object to a variable assigns the
reference, not a copy). Objects may be assigned only to variables with interface
or class types.

Functions
==

#### *Terminology*

- Formals: The variable declarations that make up part of the function signature
- Acutals: Values passed in a function invocation that correspond to formals

Function declarations consist of a return type, name, and formals. Functions may
be declared globally or inside a class. They may not be nested. Formals must be
uniquely named. Function overloading is illegal.

Formals are declared in a separate scope from local variables. Any return
statements inside a function's body must return a value matching the return type,
or nothing (`return;`) if the return type is `void`. Recursive functions are legal.

### Invocation

When a function is invoked, actuals are passed to the function by first evaluating them left to right, then binding them to the function's formals. Then, control is
given to the function. When either a return statement or the end of the function
is reached, control is given back to the caller. If a value is provided by a
`return` statement, it is passed back to the caller. Function invocations therefore
evaluate to the called function's return type.

#### Passing Semantics

- Ani is pass-by value
- Strings, arrays, and objects are references under-the-hood
  - This results in pass-by-reference semantics for this case

Function invocations must contain the same number of actuals as the number of
formals on the called function. The actuals must be type compatible with the
formals.

Variables
==

Variables can be declared anywhere, and how they behave depends upon where they
are declared. Variables may have any type except `void`.

Classes
==

Declaring a class results in a new named type. Class declarations can only exist
in the global scope and must be uniquely named. All declarations inside a class
are declared in a class scope, and must be either variables or functions.

#### *Terminology*

- Fields: any declaration inside a class
- Methods: any function declared inside a class
- Instance variables: any variable declared in a class

All fields' names must be unique to their class as well as their superclasses
and can be declared in any order. The exception is overriding methods, but the
method signature must match the superclass's definition.

Use `this` to refer to the current object inside methods, and `super` to refer
to the current object upcasted to the superclass's type inside methods. You
may omit `this`, but you may not omit `super`, without changing the semantics
of your program.

Instance variables are private, but may be accessed by methods belonging to
their declaring class or subclasses (Java calls this `protected`). All methods
are public.

### Inheritance

Ani supports single inheritance. If a class `A` inherits class `B`, `A` inherits
all the fields defined in the class `B`, in addition to `A`'s own fields. `B`
must be a class and cannot be an array type. `A` also inherits all interfaces
of `B`.

### Interfaces

An interface declaration consists of a list of function prototypes. Interfaces
may only be declared in the global scope, and classes may implement one or more
interfaces.

When a class states that it implements an interface (it must do so via
`implements`, and each type listed must be an interface), it must provide an
implementation for every function prototype in the interface. Each implemented
method must match the interface's prototype in return type and formal types.

### Automatic Upcasting

Automatic upcasting occurs when an value of a named type is provided when a
different but compatible type is expected. When a value is upcast, it behaves
like a value of the new type. In the case of objects, fields present in the
original type but not the new type are inaccessible through the upcast reference.

Control Structures
==

Ani supports `while`, `for`, and `if`/`else`. An `else` clause always joins with the closest unclosed `if` statement. The test/condition expression in `if`, `while`, and `for` statements must be of type `bool`. `break` statements can only appear
inside `while` or `for` loops. `return` statements can only appear inside functions,
and the value in a return statement must match the return type of the enclosing
function.

Expressions
==

Ani does not allow co-mingling and/or automatic conversion between types within
expressions (like adding an `int` to a `double`, or using an `int` as a `bool`).
The operands for all expressions are evaluated from left to right.

Expressions evaulate as follows:

- Constants evaluate to themselves (`true`, `false`, `null`, `int`s, `double`s, `string` literals)
- `this` is bound to the receiving object within class scope. It is an error outside class scope
- `super` is bound to the receiving object within a class scope *upcast to its superclass*. It is invalid outside a class scope. `super` is used to call the superclass's implementation of a method.

#### Operators

- Binary arithmetic operators (`+`, `-`, `*`, `/`, and `%`) take either two `int`s or `double`s. The result is the same type as the operands.
- Unary minus (`-`) takes an `int` or `double`. The result is the same type as the operand.
- Binary relational operators (`<`, `>`, `<=`, `>=`) take either two `int`s or `doubles`. The result is a `bool`.
- Binary equality operators (`!=` and `==`) take two operands that are type compatible in at least one direction. The result is a `bool`.
- All operands to binary and unary logical operators (`&&`, `||`, and `!`) are `bool`s. The result type is a `bool`. Logical operators do no short-circuit.
- The assignment operator takes an assignable location on the left and an expression on the right. Assignable locations are variables. The right side of an assignment must be type compatible with the left side.

#### Precedence

From highest to lowest:

`[` `.` (array indexing, field selection)

`!` `-` (logical not, unary `-`)

`*` `/` `%` (multiply, divide, mod)

`+` `-` (addition, subtraction)

`<` `<=` `>` `>=` (relational)

`==` `!=` (equality)

`&&` (logical and)

`||` (logical or)

`=` (assignment)

All binary operators (arithmetic and logical) are left-associative. Assigment and
relational operators are non-associative and do not support chaining of themselves
(but may exist in chains with other operators, for example `a < b == c`).
Parentheses may be used to override precedence and/or associativity.

Builtin Functions
==

Ani has builtin functions that allow for simple I/O and object/array instantiation.

- `Print()` takes any number of arguments, and all arguments must be `string`s, `int`s, or `bool`s. They are printed to stdout.
- `New()` takes a single argument, a class name. It returns an object.
- `NewArray()` takes an integer and a non-void type, and return an array.
- `ReadLine()` reads a string entered by the user, up to but not including the newline.
- `ReadInteger()` reads a line of text entered by the user, and converts it into an integer. (returns `0` if the user didn't enter a valid number).
