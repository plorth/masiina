# masiina

Experimental [virtual machine] for [Plorth] programming language.

## Compilation

C++17 capable compiler and [CMake] are required to compile the interpreter.
Dependencies are handled by [cget], which also needs to be installed.

```bash
$ cget install
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Usage

Masiina consists from two binaries: compiler and the runtime that is used to
run [Plorth] programs compiled into bytecode by the compiler.

The compiler is called `masiinac` and it can be used to compile [Plorth]
programs into bytecode. Multiple programs can be compiled into single
compilation unit, but the first one is considered to be _main program_, which
is the one that is initially executed.

For example:

```bash
$ masiinac -o output.bin 1.plorth 2.plorth 3.plorth
```

would compile files `1.plorth`, `2.plorth` and `3.plorth` into a single file
but `1.plorth` would be the [Plorth] program that would be initially executed
while `2.plorth` and `3.plorth` would be considered as utility modules that
could be imported from `1.plorth`. The compiler will then produce file
`output.bin` that can be executed by the Masiina virtual machine.

Once you have compiled one or more [Plorth] programs into an compilation unit
you can then execute it with `masiina` like this:

```bash
$ masiina output.bin arg1 arg2 arg3
```

[virtual machine]: https://en.wikipedia.org/wiki/Virtual_machine#Process_virtual_machines
[plorth]: https://plorth.org
[cmake]: https://cmake.org
[cget]: https://github.com/pfultz2/cget
