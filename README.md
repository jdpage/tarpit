Tarpit is a rather naive brainf\*\*\* compiler for Linux. It produces a 32-bit
x86 ELF-formatted executable given a valid brainf\*\*\* program.

Usage: tarpit <source> [<output>]

### Compiling

You should be able to compile Tarpit itself just by running make. If you want
to compile the programs in the examples directory also, do "make examples".

Both of the example programs were taken from the [Wikipedia article](http://en.wikipedia.org/w/index.php?title=Brainfuck&oldid=470402810).

### Internals

The pointer to the current cell is stored in the EAX register. For the read and
write operations, it is moved to ECX to be passed to the read(2) and write(2)
syscalls, then moved back to EAX, so it never has to be stored in memory.

All cell addition and subtraction is done in-memory, without moving it to
a register first.

For \<, \>, +, and - tokens, multiple consecutive instances of the same token
are condensed down to a single ADD or SUB instruction. For cells, the number
is treated as modulo 256 (i.e. it overflows). This means that 257 sequential +
tokens and a single + token are considered equivalent and generate the same
instruction.

Sequential reads and writes are not optimized in any way, so two consecutive
writes would still tear down the syscall and set it up again in between.

, is implemented as a call to read(2) on stdin (fd 0) using 1 character.

The looping instructions [ and ] are each implemented as a combination of a CMP
and either a JE or JNE instruction. Both of them are implemented as conditional
jumps. The target of the jump is the instruction after the corresponding JE or
JNE instruction.

### License

Copyright (c) 2012 Jonathan David Page

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

