# lispish

A simple LISP-like programming language made to learn a few things. The
language was based on a tutorial [1].

For now this is useful only to do basic arithmetic. :-)

Features:

- basic arithmetic works
- closures
- builtin symbols: atom, eq, define, if, lambda, quote, mod, +, -, /,
  *, >
- types: integer, string, symbol, list
- REPL uses linenoise for history and line-editing
- embedded tests

To build the interpreter:

    make repl

To build the tests:

    make test

To build both of them together:

    make all

[1] https://github.com/kvalle/diy-lisp
