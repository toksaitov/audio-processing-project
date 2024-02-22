Accelerating Audio Processing
=============================

![Audio Volume](https://i.imgur.com/ogS6xXO.png)

The aim of this project is to optimize a C program designed for adjusting the volume of a .wav audio file. This will be achieved by leveraging the AVX/AVX2/AVX-512/Neon SIMD instructions of the x86-64/AArch64 CPU ISAs. The program's current version employs a straightforward loop to scale each audio sample by a desired volume level, specified via the command line. Your task is to modify the loop's body (specifically, replacing the placeholder marked "TODO" in `volume.c`) to incorporate AVX/AVX2/AVX-512/Neon intrinsics. This enhancement should improve the loop's performance by a minimum factor of 3, as averaged over five profiling attempts on our course server. Refer to the resources provided during lectures to identify the appropriate AVX, AVX2, AVX-512, or Neon intrinsics for this task.

## Tasks

1. Upload or clone the repository with sources to our course server `auca.space`. You must measure performance on `auca.space` rather than on your personal computer.
2. Use `cd` to navigate into the repository.
3. Inside the repository, create a `build` directory. Do NOT navigate into the `build` directory after creation.
4. Compile `volume.c` using the command `cmake . -B build -DUSE_INTRINSICS=OFF && cmake --build ./build`.
5. Execute the program with `./build/volume /srv/input.wav output.wav 0.2`.
6. Open the `volume.c` file and locate the comments `// TODO: write your SIMD AVX/AVX2/AVX-512 code here`.
7. Replace these comments with your AVX/AVX2/AVX-512 intrinsics to optimize the loop, aiming to improve the timing printed by the program by at least a factor of 3 (averaged over five profiling runs). Recompile the optimized code using `cmake . -B build -DUSE_INTRINSICS=ON -DPREFER_AVX512=OFF && cmake --build ./build`. To switch to AVX-512, compile with `cmake . -B build -DUSE_INTRINSICS=ON -DPREFER_AVX512=ON && cmake --build ./build`.
8. To earn 3 bonus points, complete the optional `TODO` by writing SIMD ARM Neon code. You will need to determine how to compile, run, and test your ARM code.
9. Verify that the volume of the `output.wav` file is correctly adjusted. For this, you can use a program like [Audacity](https://www.audacityteam.org).

## Rules

* Do NOT profile code anywhere except on our server at `auca.space`.
* Avoid procrastination and completing work at the last moment. If the server becomes overloaded close to the deadline, obtaining accurate measurements may be impossible. Extensions will not be granted for this reason.
* Do NOT make changes to any part of `volume.c` except within the loop where the `// TODO...` comments are located.
* Do NOT alter any optimization flags. Some compilers can optimize simple `for` loops effectively. We intentionally disable SIMD instructions when `USE_INTRINSICS=OFF` is selected.
* Do NOT modify the volume adjustment logic in your SIMD code.
* Limit the creation of `.wav` files in your home directory to one. Each file approximately occupies one gigabyte of space. Ensure to remove the file after submission.

## What to Submit

Commit and push your changes to the private repository provided by your instructor. Do NOT upload the generated `.wav` files. Submit the URL of your last commit to Moodle before the deadline.

## Deadline

Check Moodle for information about the deadlines.

## Documentation

    man gcc
    man cmake

## Links

### C, GDB

* [Beej's Guide to C Programming](https://beej.us/guide/bgc)
* [GDB Quick Reference](http://users.ece.utexas.edu/~adnan/gdb-refcard.pdf)

### x86-64 SIMD

* [Intel Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
* [SIMD Basics](https://www.codeproject.com/Articles/874396/Crunching-Numbers-with-AVX-and-AVX)
* [SIMD Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide)
* [Visual SIMD Guide](https://www.officedaytime.com/simd512e/)

## Books

* C Programming: A Modern Approach, 2nd Edition by K. N. King
