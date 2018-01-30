Ani Grammar
=

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
`ReadInteger` `ReadLine`

The following identifiers are reserved as literals:

`true` `false` `null`

An integer: a sequence of numbers from 0 to 9

A double: a sequence of numbers from 0 to 9, followed by a period, followed by a
sequence of characters from 0 to 9.

A string: a double quote, followed by a sequence of characters except newlines
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

All programs require a function called `main`. `main` must take either no arguments,
or an array of strings.

Scoping
===

There are multiple levels of scope, in order of precedence (lowest to highest):

1. Global - variables, functions, and classes
2. Class - variables and functions
3. Function arguments - variables
4. local variables - variables

Variables within a scope must be unique, but can be shadowed in scopes with
higher precendence.

Global variables are enumerated before typechecking begins, so that a class can
extend a class defined lower in the source file.

Declarations are available at the same scope level even before the lexical
point at which they are declared. This enables situations like a class extending
another class that is lexically defined after it. This rule applies uniformly to
all declarations (variables, classes, interfaces, functions) in all scopes.

Types
===

Ani supports four builtin types, `string`, `bool`, `int`, and `double`. It also
supports named types: user-defined classes and interfaces. Ani also supports
arrays. Ani also supports a dummy type `void` solely for functionsthat don't
return anything.

Compatibility
=====

// TODO: write me

Arrays
=====

Arrays are created with the builtin `NewArray(N, type)` where `N` is a positive,
non-zero integer representing the size of the array (number of elements). `type`
is any type other than `void`. Once created, arrays' sizes cannot change.

Arrays' indeces start at 0, and are indexed through the subscript operator, which 
takes a positive integer. Arrays support the `length()` method, which returns
their length. Arrays are passed, assigned, and compared by reference.

Strings
======

Strings are assigned and passed by reference, but compared by value. As of now,
there is no support for string manipulation or any advanced string functionality.

Functions
==

Function declaractions consist of a return type, a name, and a list of argument
types (variable declarations - formals). Functions are declared either globally
or inside a class as methods. Functions cannot be nested in other functions.
Arguments must have distinct names.

Formals are declared in a separate scope from local variables. Any return
statements inside a function's body must return a value matching the return type,
or nothing (`return;`) if the return type is `void`. Recursive functions are legal.

As of the time of writing, function overloading is illegal.

Invoking a function with arguments passes the arguments by value (unless a string
or array is passed, then it's by reference). Arguments are then bound to the
formals, and control is passed to the function. When the function returns, the
returned value is passed back to the callee in the same way as the callee passes
arguments to the function.

When a function is invoked, the arguments must match the formals in number and must
be type compatible with the formals. Arguments are evaluated from left to right. Functions return on a return statement or the end of the function body. Function calls
evaluate to the type of the function's return type.

Variables
==

Variables can be declared anywhere, and how they behave depends upon where they
are declared. Variables may have any type except `void`.

Classes
==

Declaring a class results in a new named type. Class declarations can only exist
in the global scope and must be uniquely named. All declarations inside a class
are declared in a class scope, and must be either variables or functions.

### Terminology

- Fields: any declaration inside a class
- Methods: any function declared inside a class
- Instance variables: any variable declared in a class

All fields' names must be unique to their class and superclasses and can be
declared in any order. You may omit `this` when accessing fields within a method.

Instance variables are private, but may be accessed by methods belonging to their
declaring class or subclasses (Java calls this `protected`). All methods are public.

