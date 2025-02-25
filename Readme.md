Accelerating Audio Processing
=============================

![Audio Waveform](https://i.imgur.com/ogS6xXO.png)

The aim of this project is to optimize a [median filter](https://en.wikipedia.org/wiki/Median_filter) used to remove clicking, crackling, and popping noises from a `.wav` audio file by leveraging AVX-512 SIMD instructions on x86-64 CPUs. The provided code contains a median filter implementation that employs the C standard library’s quicksort algorithm to determine the median of small windows within the audio signal. Your task is to modify the relevant sorting functions (specifically, replacing the placeholder marked `// TODO...` in `median.c`) to incorporate AVX-512 intrinsics for parallel sorting. For windows containing eight `double` samples, you must implement a branch-free Bitonic sort variant described in the paper 'Fast Sorting Algorithms Using AVX-512 on Intel Knights Landing' by Berenger Bramas. You can read the paper [here](https://inria.hal.science/hal-01512970v1/document). For windows containing sixteen `float` samples, you may either generalize the Bitonic approach, adapt another SIMD solution found in the literature, or consult modern AI-based systems, as they are permitted for this project. Refer to the resources provided during lectures, as well as the recommended links outlined at the end of this requirement document.

## Tasks

1. Upload or clone the source files to our main course server, `auca.space`. You must measure performance on `auca.space` and not on your computer.
2. Compile the C version of `median.c` without AVX-512 intrinsics using the command: `gcc -O3 -o median median.c`.
3. Execute the program with `./median /srv/input.wav output_8.wav 8` to apply a small filtering window of 8 audio samples to the `/srv/input.wav` file. This window will remove small clicks, pops, and cracks from the audio signal without significantly altering other sounds. You can listen to and analyze the `output_8.wav` file using a program like [Audacity](https://www.audacityteam.org) installed on your computer. Don't forget to download the audio file to your machine first.
4. Execute the program with `./median /srv/input.wav output_16.wav 16` to apply a larger filtering window of 16 audio samples. This window will remove more prominent clicks, pops, and cracks from the audio signal. However, it will also have a greater negative impact on other sounds.
5. Open the `median.c` file and locate the `TODO` comments. Replace these comments with your AVX-512 intrinsics to parallelize the sorting of arrays containing 8 and 16 elements. For the filter window of size 8, refer to the paper 'Fast Sorting Algorithms Using AVX-512 on Intel Knights Landing' by Berenger Bramas for the Bitonic sort variant. For the filter window of size 16, you may either generalize the Bitonic approach, adapt another research-based solution, or seek assistance from modern AI systems, as consulting them is permitted for this project. Ensure that your implementation prioritizes both efficiency and correctness.
6. Compile the AVX-512 version of `median.c` using the `gcc -march=native -O3 -DUSE_INTRINSICS -o median median.c` command.
7. Generate the `output_8.wav` and `output_16.wav` files again using the AVX-512 version of the program. You should observe a significant performance improvement while maintaining similar audio quality. Note that AVX-512 instructions may handle floating-point numbers slightly differently than the C standard library, which could result in minor variations in the generated audio. However, the output should remain very similar to that produced by the C version of the code.

## Rules

* Do NOT profile code anywhere except on our server at `auca.space`.
* Do NOT procrastinate or leave the work until the very last moment. If the server is overloaded close to the deadline, you will not be able to obtain accurate measurements. No extensions will be granted for this reason.
* Use the GCC compiler installed on the server to build your code.
* Do NOT use any additional compiler flags aside from `-O3`, `-march=native`, or `-DUSE_INTRINSICS`.
* Do NOT modify any part of the `median.c` file except within the functions that contain the `// TODO...` comments.
* Limit the number of `.wav` files in your home directory to one or two. Each file occupies approximately one gigabyte of space. Ensure that you remove the files after submission.

## Recommendations

We recommend against using AI systems to optimize your code, but we do NOT prohibit their use. Instead, we suggest using them as tutors to assist with the C programming language or to help strategize your optimization efforts.

## What to Submit

Commit and push your changes to the private GitHub repository provided by your instructor. Do NOT attempt to upload the generated `.wav` files. Before the deadline, submit the URL of your latest commit to Moodle. The URL MUST include the commit hash. After submission, remove all `.wav` files from your home directory.

## Deadline

Check Moodle for information about the deadlines.

## Documentation

    man gcc

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
