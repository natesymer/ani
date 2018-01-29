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