
![Lost Moko and Steam](https://github.com/floowsnaake-new/Lost-Moko/blob/main/LM.png)

PROTON LAUNCH OPTIONS

Add your own USERNAME
```http
  PROTON_REMOTE_DEBUG_CMD="/home/joe/.steam/steam/steamapps/LM/LM.exe" %command%
```



# What does Lost Moko do?
This is a memory scanner and editor for the game Lost Ark. It lets you:

Attach to the Lost Ark game process.
Scan the game’s memory for specific values (like health, gold, or other in-game numbers).
Edit those values (cheat by changing them).
Save or copy memory addresses for later use.
Start/Stop a bot (though the bot logic is not fully implemented in this code).


# How does it work?
```
┌───────────────────────┐
│   Start Lost Moko     │
└──────────┬────────────┘
           │
           ▼
┌───────────────────────┐
│   Main Window         │
│   (Tabs: Scanner,     │
│    Pathing, Targeting,│
│    Multibox, Profiles,│
│    Settings)          │
└──────────┬────────────┘
           │
           ▼
┌───────────────────────┐
│   User selects tab    │
└──────────┬────────────┘
           │
           ├───────────────────┐
           │                   │
           ▼                   ▼
┌─────────────────┐   ┌─────────────────┐
│ Scanner Tab     │   │ Pathing Tab     │
└──────────┬──────┘   └──────────┬──────┘
           │                      │
           ▼                      ▼
┌─────────────────┐   ┌─────────────────┐
│ Attach to       │   │ Start/Stop Bot   │
│ LOSTARK.exe     │   │ (Stub, not       │
│                 │   │ implemented)     │
└──────────┬──────┘   └─────────────────┘
           │
           ▼
┌─────────────────┐
│ Enter value to  │
│ scan for        │
└──────────┬──────┘
           │
           ▼
┌─────────────────┐
│ Scan Memory     │
│ (2 or 4 bytes)  │
└──────────┬──────┘
           │
           ▼
┌─────────────────┐
│ Display results │
│ in ListView     │
└──────────┬──────┘
           │
           ├───────────────────┐
           │                   │
           ▼                   ▼
┌─────────────────┐   ┌─────────────────┐
│ Select address, │   │ Rescan for      │
│ edit value      │   │ changed value   │
│ (Write to       │   │ (Narrow down    │
│  memory)        │   │  results)       │
└─────────────────┘   └─────────────────┘
           │
           ▼
┌─────────────────┐
│ Copy/Save       │
│ address         │
└─────────────────┘
```

- Memory Scanner:

You type in a value (like "100" for gold or health).
The program searches the game’s memory for that value.
It shows you all the memory addresses where that value is found.


- Memory Editor:

You can select an address and change its value to whatever you want.


- Bot Tab:

There are buttons to start/stop a bot, but the actual bot code is not finished (it just shows a message).




# What is it meant for?

Cheating: Changing in-game values (like health, gold, etc.) to give yourself an advantage.
Automation: The bot tab suggests it could be used to automate gameplay (like grinding or farming), but this part is not fully built yet.
Reverse Engineering: Finding and editing game values for research or modding.


# Is it safe?

Its Agaisnt Terms of Service and can get your account banned.
Risky: The program accesses game memory directly, which can cause crashes or be detected by anti-cheat software.


# Summary Table
Attach to ProcessConnects to the Lost Ark game processScan MemorySearches for a specific value in the game’s memoryEdit ValuesChanges the value at a specific memory addressSave/Copy AddressesLets you save or copy memory addresses for later useBot TabPlaceholder for automating gameplay (not fully implemented)

In short: This is a cheat tool for Lost Ark, allowing you to find and change game values, and potentially automate gameplay. Using it risks your account and is against the game’s rules.

# Tested on:
Pop_OS

x86_64-w64-mingw32-gcc is a cross-compiler used to generate 64-bit Windows executables from non-Windows systems.

compile command used: x86_64-w64-mingw32-gcc -o LM.exe LM.c -lcomctl32 -mwindows
Tested on Wine and Proton 9
Breakdown:

    x86_64-w64-mingw32-gcc:
    The cross-compiler for compiling Windows 64-bit applications on a non-Windows OS (like Linux).

    -o LM.exe:
    Specifies the output filename. The compiled program will be named LM.exe, which is a Windows executable.

    LM.c:
    The source code file you're compiling.

    -lcomctl32:
    Links the Windows common controls library (comctl32.dll). This library provides GUI controls like buttons, list views, etc.

    -mwindows:
    Tells the compiler to create a Windows GUI application (without a console window). It sets the subsystem to Windows, so no command-line window appears when running the program.

Summary:

This command compiles LM.c into a Windows 64-bit executable named LM.exe, linking it with the Windows common controls library, and sets it up as a GUI application without a console window, using the cross-compiler x86_64-w64-mingw32-gcc.

Debugg info, ignore:

joe@pop-os:~/.steam/steam/steamapps/LM$ x86_64-w64-mingw32-gcc -v -g -o LM.exe LM.c -lcomctl32 -mwindows
Using built-in specs.
COLLECT_GCC=x86_64-w64-mingw32-gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-w64-mingw32/10-win32/lto-wrapper
Target: x86_64-w64-mingw32
Configured with: ../../src/configure --build=x86_64-linux-gnu --prefix=/usr --includedir='/usr/include' --mandir='/usr/share/man' --infodir='/usr/share/info' --sysconfdir=/etc --localstatedir=/var --disable-option-checking --disable-silent-rules --libdir='/usr/lib/x86_64-linux-gnu' --libexecdir='/usr/lib/x86_64-linux-gnu' --disable-maintainer-mode --disable-dependency-tracking --prefix=/usr --enable-shared --enable-static --disable-multilib --with-system-zlib --libexecdir=/usr/lib --without-included-gettext --libdir=/usr/lib --enable-libstdcxx-time=yes --with-tune=generic --with-headers --enable-version-specific-runtime-libs --enable-fully-dynamic-string --enable-libgomp --enable-languages=c,c++,fortran,objc,obj-c++,ada --enable-lto --enable-threads=win32 --program-suffix=-win32 --program-prefix=x86_64-w64-mingw32- --target=x86_64-w64-mingw32 --with-as=/usr/bin/x86_64-w64-mingw32-as --with-ld=/usr/bin/x86_64-w64-mingw32-ld --enable-libatomic --enable-libstdcxx-filesystem-ts=yes --enable-dependency-tracking SED=/bin/sed
Thread model: win32
Supported LTO compression algorithms: zlib
gcc version 10-win32 20220113 (GCC) 
COLLECT_GCC_OPTIONS='-v' '-g' '-o' 'LM.exe' '-mwindows' '-mtune=generic' '-march=x86-64'
 /usr/lib/gcc/x86_64-w64-mingw32/10-win32/cc1 -quiet -v -U_REENTRANT LM.c -quiet -dumpbase LM.c -mwindows -mtune=generic -march=x86-64 -auxbase LM -g -version -o /tmp/ccz7lcPH.s
GNU C17 (GCC) version 10-win32 20220113 (x86_64-w64-mingw32)
	compiled by GNU C version 10.3.0, GMP version 6.2.1, MPFR version 4.1.0, MPC version 1.2.1, isl version isl-0.24-GMP

GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
ignoring nonexistent directory "/usr/lib/gcc/x86_64-w64-mingw32/10-win32/../../../../x86_64-w64-mingw32/sys-include"
#include "..." search starts here:
#include <...> search starts here:
 /usr/lib/gcc/x86_64-w64-mingw32/10-win32/include
 /usr/lib/gcc/x86_64-w64-mingw32/10-win32/include-fixed
 /usr/lib/gcc/x86_64-w64-mingw32/10-win32/../../../../x86_64-w64-mingw32/include
End of search list.
GNU C17 (GCC) version 10-win32 20220113 (x86_64-w64-mingw32)
	compiled by GNU C version 10.3.0, GMP version 6.2.1, MPFR version 4.1.0, MPC version 1.2.1, isl version isl-0.24-GMP

GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
Compiler executable checksum: 540c311885c0710cac74cf12fb1ae6c1
COLLECT_GCC_OPTIONS='-v' '-g' '-o' 'LM.exe' '-mwindows' '-mtune=generic' '-march=x86-64'
 /usr/bin/x86_64-w64-mingw32-as -v -o /tmp/cc1u7h1j.o /tmp/ccz7lcPH.s
GNU assembler version 2.38 (x86_64-w64-mingw32) using BFD version (GNU Binutils) 2.38
COMPILER_PATH=/usr/lib/gcc/x86_64-w64-mingw32/10-win32/:/usr/lib/gcc/x86_64-w64-mingw32/10-win32/:/usr/lib/gcc/x86_64-w64-mingw32/:/usr/lib/gcc/x86_64-w64-mingw32/10-win32/:/usr/lib/gcc/x86_64-w64-mingw32/:/usr/lib/gcc/x86_64-w64-mingw32/10-win32/../../../../x86_64-w64-mingw32/bin/
LIBRARY_PATH=/usr/lib/gcc/x86_64-w64-mingw32/10-win32/:/usr/lib/gcc/x86_64-w64-mingw32/10-win32/../../../../x86_64-w64-mingw32/lib/
COLLECT_GCC_OPTIONS='-v' '-g' '-o' 'LM.exe' '-mwindows' '-mtune=generic' '-march=x86-64'
 /usr/lib/gcc/x86_64-w64-mingw32/10-win32/collect2 -plugin /usr/lib/gcc/x86_64-w64-mingw32/10-win32/liblto_plugin.so -plugin-opt=/usr/lib/gcc/x86_64-w64-mingw32/10-win32/lto-wrapper -plugin-opt=-fresolution=/tmp/ccLpV0Nd.res -plugin-opt=-pass-through=-lmingw32 -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_eh -plugin-opt=-pass-through=-lmoldname -plugin-opt=-pass-through=-lmingwex -plugin-opt=-pass-through=-lmsvcrt -plugin-opt=-pass-through=-lgdi32 -plugin-opt=-pass-through=-lcomdlg32 -plugin-opt=-pass-through=-ladvapi32 -plugin-opt=-pass-through=-lshell32 -plugin-opt=-pass-through=-luser32 -plugin-opt=-pass-through=-lkernel32 -plugin-opt=-pass-through=-lmingw32 -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_eh -plugin-opt=-pass-through=-lmoldname -plugin-opt=-pass-through=-lmingwex -plugin-opt=-pass-through=-lmsvcrt -m i386pep --subsystem windows -Bdynamic -o LM.exe /usr/lib/gcc/x86_64-w64-mingw32/10-win32/../../../../x86_64-w64-mingw32/lib/crt2.o /usr/lib/gcc/x86_64-w64-mingw32/10-win32/crtbegin.o -L/usr/lib/gcc/x86_64-w64-mingw32/10-win32 -L/usr/lib/gcc/x86_64-w64-mingw32/10-win32/../../../../x86_64-w64-mingw32/lib /tmp/cc1u7h1j.o -lcomctl32 -lmingw32 -lgcc -lgcc_eh -lmoldname -lmingwex -lmsvcrt -lgdi32 -lcomdlg32 -ladvapi32 -lshell32 -luser32 -lkernel32 -lmingw32 -lgcc -lgcc_eh -lmoldname -lmingwex -lmsvcrt /usr/lib/gcc/x86_64-w64-mingw32/10-win32/crtend.o
COLLECT_GCC_OPTIONS='-v' '-g' '-o' 'LM.exe' '-mwindows' '-mtune=generic' '-march=x86-64'

