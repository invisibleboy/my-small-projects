EXAMPLE ATMEL AVR PROGRAM FOR BOUND-T


1. Introduction

This folder contains a small example program for use with Bound-T.
The program does not do anything useful; it just demonstrates
some forms of code and how Bound-T can or cannot analyze them.

The program consists of a 'C' main function, in the file "main.c",
a number of 'C' functions in the file "routines.c" (supported by
the corresponding header file "routines.h"), and some types
defined in the files "types.h" and "count_t.h".

In this folder, the program has been compiled and linked for the
Atmel AVR target processor, using the the IAR compiler.

The executable file is named "prog.d90" and is in IAR-UBROF form.
This is the file that should be given as input to Bound-T.

The operation and usage of Bound-T are explained in the User Guide,
the Reference Manual, and the Application Note documents. The
following is a brief introduction that shows some ways to use
Bound-T to analyse this example program for its worst-case
execution time and stack usage.

The examples assume that Bound-T for the AVR has been installed
and the Bound-T binaries are included in the PATH variable in your
command window.

The only files from this folder that are actually needed for
these examples are the AVR executable "prog.d90" and the
assertion file "assert.txt". In particular, the C source-code files
are not necessary; Bound-T does not use the source-code files.
The output from Bound-T includes references to source-code files
and line-numbers, but these are all taken from the debugging
information that the compiler generated and placed in the UBROF
file (prog.d90).

Note that the text files and C source-code files may have
line terminators of the Unix type; open them in WordPad rather
than Notepad.


2. First Experiment

As a first experiment you might use Bound-T to analyse the routine
Count25. The following command will do that:

   boundt_avr -atmega32 prog.d90 Count25

In this command:

   - "boundt_avr" is the Bound-T executable for the Atmel AVR target.

   - "-atmega32" tells Bound-T to assume that the AVR to be used is
     model ATmega32. The most important model-specific aspect of the
     AVR is the memory map: how much internal (on-chip) RAM and
     flash are available.

   - "prog.d90" is the AVR program (UBROF file) to be analysed.

   - "Count25" is the linker symbol that the IAR compiler assigns
     to the routine Count25. (This compiler is nice in that it
     seldom "mangles" the names of C functions.)

The result of this commmand should be the following output:

   Loop_Bound:prog.d90:routines.c:Count25:18-20:13
   Wcet:prog.d90:routines.c:Count25:14-22:285

The first output line reports that Bound-T has determined that the
loop in the Count25 routine, on lines 18-20 in the file routines.c,
repeats at most 13 times. To be precise, the "back edge" of the
loop is executed at most 13 times. If you look at the machine code
(using, for example, the Bound-T option "-trace decode"), you can
see that this is a "top-test" loop, so the body of the loop is
also executed 13 times, and the loop head, with the termination
test, is executed 14 times.

The second output line reports that Bound-T has determined an
upper bound of 285 machine cycles on the worst-case execution time
(WCET) of the function Count25. The line also shows that the function
is located in the source-file routines.c on lines 14-22.


2. Full Analysis Fails

To analyse the top-level "main" function in the example program, you
can try the command

   boundt_avr -atmega32 prog.d90 main

However, there are some loops for which Bound-T cannot find iteration
bounds, so the output of this command contains with some warnings
and errors to this effect:

   Warning:prog.d90:main.c:main:36-:Large combined 16-bit literal
      65535 taken as signed = -1
   Warning:prog.d90:main.c:main:32-36:Eternal loop (no exit edges).
   Warning:prog.d90:routines.c:Foo:34-36:Large combined 16-bit literal
      65533 taken as signed = -3
   Warning:prog.d90:routines.c:Foo7:43-:Large combined 16-bit literal
      65526 taken as signed = -10
   Loop_Bound:prog.d90:routines.c:Count25:18-20:13
   Loop_Bound:prog.d90:routines.c:Foo7@45=>Count:27-29:4
   Loop_Bound:prog.d90:routines.c:Solve:57-69:8
   Loop_Bound:prog.d90:routines.c:main@28=>Foo@36=>Count:27-29:5
   Warning:prog.d90:main.c:main:17-36:Non-returning subprogram.
   Wcet:prog.d90:routines.c:Count25:14-22:285
   Wcet_Call:prog.d90:routines.c:Foo7@45=>Count:25-31:86
   Wcet:prog.d90:routines.c:Foo7:41-49:155
   Wcet_Call:prog.d90:routines.c:main@28=>Foo@36=>Count:25-31:105
   Wcet_Call:prog.d90:routines.c:main@28=>Foo:34-38:116
   Error:prog.d90:main.c:main:17-36:Could not be fully bounded.

   main
      Loop unbounded (eternal) at main.c:32-36, offset 00000026
      main@32=>Solve@60=>Ones
         Loop unbounded at routines.c:80-85, offset 00000010
      main@32=>Solve@69=>Count
         Loop unbounded at routines.c:27-29, offset 00000016

In the output quoted above, the first, third and fourth output lines
report on the interpretation of immediate operands and can be ignored.
(The lines are broken into two in this README file, to avoid very
long lines of text.)

The second output line reports that Bound-T has found an eternal
(non-terminating) loop in the main function at lines 32-36 in the
file main.c.

The next four output lines (Loop_Bound lines) report the iteration
bounds that Bound-T determines for four loops. Some of these 
bounds are context-dependent, meaning that they are valid only for
a particular call path. For example, the second Loop_Bound line
reports that the loop in the function Count, at lines 27-29 in
routines.c, has an upper bound of 4 repetitions when Count is
called from the function Foo7 through the call at line 45.

The fourth Loop_Bound line shows that the same loop in Count has
a different bound (5) when Count is called in a different context.

After the Loop_Bound lines, the Warning line reports that the
main function never returns (because it ends with an eternal
loop). The main function is in the file main.c, lines 17-36.

The Warning line is followed by some Wcet lines giving the WCET
bounds computed for those subprograms that have no unbounded
loops. The Wcet_Call lines show the WCET bounds for particular
calls, when the loop-bounds depend on the call. For example, the
first Wcet_Call line reports that the function Count takes at most
86 machine cycles when it is called from Foo7 at line 45. In
contrast, the second such line reports that when Count is called
from the function Foo it has the larger WCET bound of 105 cycles.

The Error line reports that Bound-T could not find a WCET bound
for the main function. The indented, hierarchical listing after
the Error line shows that the problems are the eternal loop, a
loop in the function Ones, which is called from the function
Solve, and the loop in Count when Count is called from Solve.
This is the same loop for which a Loop_Bound was found when Count
was called from Foo7, but that bound does not apply when Count is
called from Solve because Solve does not give a static parameter
value to the parameter of Count that controls the number of
iterations of the loop.

The reasons why Bound-T cannot find bounds on these loops are
explained in comments in the source-code files. For such loops,
the user must support Bound-T by stating "assertions" on the
program's behaviour. We do that in the next section of this
example.


3. Full Analysis Succeeds with Assertions

For this example, the necessary assertions are already written
in the text file assert.txt. That file also contains comments that
explain the assertions.

The option -assert tells Bound-T to use these assertions:

   boundt_avr -atmega32 -assert assert.txt prog.d90 main

This command should give the following output, which concludes
with a "Wcet" line giving the WCET bound for the main function:

   Warning:prog.d90:main.c:main:36-:Large combined 16-bit literal
      65535 taken as signed = -1
   Warning:prog.d90:main.c:main:32-36:Eternal loop (no exit edges).
   Warning:prog.d90:routines.c:Foo:34-36:Large combined 16-bit literal
      65533 taken as signed = -3
   Warning:prog.d90:routines.c:Foo7:43-:Large combined 16-bit literal
      65526 taken as signed = -10
   Loop_Bound:prog.d90:routines.c:Count25:18-20:13
   Loop_Bound:prog.d90:routines.c:Foo7@45=>Count:27-29:4
   Loop_Bound:prog.d90:routines.c:Solve:57-69:8
   Loop_Bound:prog.d90:routines.c:Solve@69=>Count:27-29:8
   Loop_Bound:prog.d90:routines.c:main@28=>Foo@36=>Count:27-29:5
   Warning:prog.d90:main.c:main:17-36:Non-returning subprogram.
   Wcet:prog.d90:routines.c:Count25:14-22:285
   Wcet_Call:prog.d90:routines.c:Foo7@45=>Count:25-31:86
   Wcet:prog.d90:routines.c:Foo7:41-49:155
   Wcet_Call:prog.d90:routines.c:main@28=>Foo@36=>Count:25-31:105
   Wcet_Call:prog.d90:routines.c:main@28=>Foo:34-38:116
   Wcet:prog.d90:routines.c:Ones:76-88:173
   Wcet_Call:prog.d90:routines.c:Solve@69=>Count:25-31:162
   Wcet:prog.d90:routines.c:Solve:52-73:3238
   Wcet:prog.d90:main.c:main:17-36:3844

There are now more Loop_Bound lines and more Wcet and Wcet_Call
lines, as expected.

You may wonder how Bound-T can compute a WCET bound of 3844 cycles
for the main function although the function contains an eternal loop.
The answer is that assert.txt contains an assertion that makes
Bound-T include only one iteration of the eternal loop in the WCET.


4. Graph Files

The files graphs.dot, graphs.ps and graphs.pdf show the call-graph
and control-flow graphs in DOT, PS and PDF forms. Bound-T created
the DOT file in response to the option "-dot" when the full analysis
was run with the command

   boundt_avr -atmega32 -assert assert.txt -dot graphs.dot -draw total prog.d90 main

The PS file was created from the DOT file by the DOT tool (available
from http://www.graphviz.org) and the PDF was created from the PS
file by the GNU ps2pdf tool.


5. Stack Analysis

Bound-T can also compute upper bounds on the stack-space usage in
the program. This can be useful to ensure that sufficient stack
space is allocated to avoid stack overflow.

The IAR compiler in fact uses two stacks on the AVR. The "native"
AVR stack, using the SP register, is used for return addresses but
not for data. The compiler defines a "software" stack to hold
data such as parameters and local variables. The compiler uses
the AVR "Y" register as the stack pointer fort this software stack.
Bound-T analyses the usage of the SP stack and the Y stack separately,
and indeed there is no reason for them to be correlated.

To compute an upper bound on the amount of stack space used in
the example program, starting from the function main, give the
command:

   boundt_avr -atmega32 -stack_path -no_time prog.d90 main

This should give the output

   Warning:prog.d90:main.c:main:36-:Large combined 16-bit literal
      65535 taken as signed = -1
   Warning:prog.d90:main.c:main:32-36:Eternal loop (no exit edges).
   Warning:prog.d90:routines.c:Foo:34-36:Large combined 16-bit literal
      65533 taken as signed = -3
   Warning:prog.d90:routines.c:Foo7:43-:Large combined 16-bit literal
      65526 taken as signed = -10
   Stack:prog.d90:routines.c:Count25:14-22:SP:2
   Stack:prog.d90:routines.c:Count25:14-22:-Y:2
   Stack:prog.d90:routines.c:Count:25-31:SP:0
   Stack:prog.d90:routines.c:Count:25-31:-Y:0
   Stack:prog.d90:routines.c:Ones:76-88:SP:0
   Stack:prog.d90:routines.c:Ones:76-88:-Y:0
   Stack:prog.d90:routines.c:Foo7:41-49:SP:2
   Stack:prog.d90:routines.c:Foo7:41-49:-Y:4
   Stack:prog.d90:routines.c:Foo:34-38:SP:2
   Stack:prog.d90:routines.c:Foo:34-38:-Y:0
   Stack:prog.d90:routines.c:Solve:52-73:SP:2
   Stack:prog.d90:routines.c:Solve:52-73:-Y:6
   Stack:prog.d90:main.c:main:17-36:SP:4
   Stack:prog.d90:main.c:main:17-36:-Y:8
   Warning:prog.d90:main.c:main:17-36:Non-returning subprogram.
   Stack_Path:prog.d90:main.c:main@21=>Count25:21:SP:4:2:2:2
   Stack_Leaf:prog.d90:routines.c:Count25:14-22:SP:2:2::
   Stack_Path:prog.d90:main.c:main@32=>Solve:32:-Y:8:2:2:6
   Stack_Leaf:prog.d90:routines.c:Solve:52-73:-Y:6:6::

The Warning lines repeat the warnings about the immediate operands
and the eternal loop, already discussed above. These have no effect
on the stack analysis.

The output lines starting with "Stack" show the total stack usage
in various functions and for each of the two stacks, SP and Y.
(The Y stack is identified as "-Y" to emphasize that it grows
downwards in memory; a "push" will decrease the stack pointer Y.)

For example, the first Stack line shows that the Count25 function
uses 2 octets of the "native" or SP stack, when we include stack
space used in all functions that Count25 calls. (The call-graph
in graphs.pdf does not show any calls from Count25, but in fact
Count25 calls a "prelude" routine, defined by the IAR compiler,
that consumes 2 octets of SP space for the return addresss. The
prelude routine is given special treatment in Bound-T and is
therefore not considered a normal "call" and not shown as a call
in the call-graph.)

The final Stack line shows that the main function and all its
callees use a total of 8 octets of the software stack (the Y
stack).

The lines that start with "Stack_Path" and "Stack_Leaf" shows the
call-path, starting from the main function, that uses the maximal
amount of stack space. For the SP stack this is the path
main => Count25. For the Y stack this is the different path
main => Solve.

In each Stack_Path line, the four last numbers show respectively
the total stack usage (this subprogram and callees); the maximum
local stack height in this subprogram (not including callees);
the maximum local stack height at any call in this subprogram;
and the total stack usage of the callees.

Note that the space on the SP stack for the return address of a
call is counted in the caller's stack usage. This means that the
return address of the main function (if there is one, which is
not certain) is not included in these results. If the main
routine needs a return address, add 2 octets to the SP stack
usage.


6. For More Information

If you need more information or advice about this example or
about Bound-T in general, Tidorum Ltd will be glad to help.
Please do not hesitate to e-mail us.

Cordially,

Tidorum Ltd
www.tidorum.fi
--
2008-06-12
