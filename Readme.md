Accelerating Audio Processing
=============================

![Audio Volume](https://i.imgur.com/ogS6xXO.png)

The aim of this project is to optimize a C program that allows changing the volume of a .wav audio file. We will do it by utilizing AVX/AVX2 SIMD instructions of the x86-64 CPU ISA. The current implementation of the program has a straightforward loop that multiplies every sample of the audio file by the desired volume specified on the command line. Your goal is to rewrite the body of the loop (replacing the only TODO comment in `volume.c`) to utilize AVX/AVX2 intrinsics to improve the performance of the loop by at least a factor of 3 (on average for five profiling runs on our course server). Use resources mentioned during lectures to help you pick the correct AVX or AVX2 intrinsics.

## Tasks

1. Upload/clone the repo with sources to our course server `auca.space`. You have to measure performance on `auca.space` and not on your computer.
2. `cd` into the repo.
3. Create the `build` directory inside the repo. Do NOT switch to the `build` directory.
4. Compile `volume.c` with `cmake . -B build -DUSE_INTRINSICS=OFF && cmake --build ./build`.
5. Run the program `./build/volume /srv/input.wav output.wav 0.2`.
6. Open the `volume.c` file and find the comments `// TODO: write your SIMD AVX/AVX2 code here`.
7. Replace the comment with AVX/AVX2 intrinsics to optimize the loop to speed up the timing printed by the program by at least a factor of 3 (on average for five profiling runs). Compile the optimized code with `cmake . -B build -DUSE_INTRINSICS=ON && cmake --build ./build`.
8. Ensure the volume of the `output.wav` file is changed correctly.

## Rules

* Do NOT profile code anywhere but on our server at `auca.space`.
* Do NOT procrastinate and leave the work to the very last moment. If the server is overloaded close to the deadline, you will not be able to get good measurements. We will not give any extensions for that reason.
* Do NOT change anything anywhere in `volume.c` except inside of the loop where the `// TODO...` comment is specified.
* Do NOT change any optimization flags. Some compilers are smart enough to optimize such a primitive `for` loop. We disable SIMD instructions on purpose for the `USE_INTRINSICS=OFF` option.
* Do NOT change the volume adjustment logic in your SIMD code.
* Do NOT generate more than one `.wav` file in your home directory. Remember that each file is around one gigabyte in size. Remove the file after submission.

## What to Submit

1. In your private course repository that was given to you by the instructor during the lecture, create the path `project-2/`.
2. Put the `volume.c` file into that directory.
3. Commit and push your repository through Git. Submit the last commit URL to Canvas before the deadline.

## Deadline

Check Canvas for information about the deadlines.

## Documentation

    man gcc
    man cmake

## Links

### C, GDB

* [Beej's Guide to C Programming](https://beej.us/guide/bgc)
* [GDB Quick Reference](http://users.ece.utexas.edu/~adnan/gdb-refcard.pdf)

### x86-64 SIMD

* [SIMD Basics](https://www.codeproject.com/Articles/874396/Crunching-Numbers-with-AVX-and-AVX)
* [SIMD Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide)
* [Visual SIMD Guide](https://www.officedaytime.com/simd512e/)

## Books

* C Programming: A Modern Approach, 2nd Edition by K. N. King
