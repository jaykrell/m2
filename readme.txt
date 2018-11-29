This is a dotnet implementation
with the primary features being:

- Converts IL to C++.
- Maybe also WebAssembly. Else the project name was mxx,
  but m2 is punnier -- mono2. duo?
- Class at a type conversion, for an LTCG/LTO balance.
  - i.e. class at a time, but outputing a source file per type.
  - i.e. not assembly at a time.
- Uses the Mono runtime.
- Written in C++.
- Very portable (like Mono, but an easier time of it
  for lack of JIT, etc.)

- Why not write in C?
 - C is a horrible language.

- Why not generate C?
 - Maybe for exception handling. We'l see.
 - C is a horrible language.

- Why Mono?
  Because I worked there.

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
  Undecided.

- Compatible with CoreCLR?
  Unknown, undecided.
