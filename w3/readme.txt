This is a dotnet implementation
with the primary features being:

- Converts IL to C++? Not C, because, exception handling.
- Convert IL to WebAssembly?
- Interpreter?
- Understandable by me hopefully.
- Class at a type conversion, for an LTCG/LTO balance.
  - i.e. class at a time, but outputing a source file per type.
  - i.e. not assembly at a time.
  - Benefits C++ compilation, at least.
  - Maybe whole program-ish for WebAssembly.
- Uses the Mono runtime?
  Using any runtime is another layer of work.
- Written in C++? C? C++11? C++98?
- Very portable (like Mono, but an easier time of it
  for lack of JIT, etc.)

- Why not write in C?
 - C is a horrible language.

- Why not generate C?
 - Maybe for exception handling. We'l see.
 - C is a horrible language.

- Why Mono?
  Familiarity? Former work.

- What about System.Reflection.Emit?
  Not yet decided.
  Choices:
    - LLVM
    - C++
    - Mono JIT.
    - Mono Interpreter.
    - A new interpreter.
    - Nothing.

- Which GC?
  Undecided. Boehm? Mono/sgen? CoreCLR? New? Modula-3?

- Compatible with CoreCLR?
  Unknown, undecided.
