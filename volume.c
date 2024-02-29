#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_INTRINSICS
#if defined(__AVX512F__) || defined(__AVX2__)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#else
#error "This code only supports x86-64 AVX2, AVX-512, and AArch64 NEON SIMD instructions."
#endif /* defined(__AVX512F__) || defined(__AVX2__) */
#endif /* USE_INTRINSICS */

#ifdef __linux
#define HAVE_POSIX_TIMER
#include <time.h>
#ifdef CLOCK_MONOTONIC
#define CLOCKID CLOCK_MONOTONIC
#else
#define CLOCKID CLOCK_REALTIME
#endif /* CLOCK_MONOTONIC */
#elif __APPLE__
#define HAVE_MACH_TIMER
#include <mach/mach_time.h>
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <malloc.h>
#else
#error "This code only supports latest versions of GNU/Linux, macOS, and Windows."
#endif /* __linux */

static uint64_t ns() {
    static bool initialized = false;

#ifdef __linux
    static struct timespec linux_rate;
    if (!initialized) {
        clock_getres(CLOCKID, &linux_rate);
        initialized = true;
    }
    uint64_t now;
    struct timespec spec;
    clock_gettime(CLOCKID, &spec);
    now = spec.tv_sec * 1.0e9 + spec.tv_nsec;

    return now;
#elif __APPLE__
    static mach_timebase_info_data_t info;
    if (!initialized) {
        mach_timebase_info(&info);
        initialized = true;
    }
    uint64_t now;
    now = mach_absolute_time();
    now *= info.numer;
    now /= info.denom;

    return now;
#elif _WIN32
    static LARGE_INTEGER win_frequency;
    if (!initialized) {
        QueryPerformanceFrequency(&win_frequency);
        initialized = true;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (uint64_t) ((1e9 * (double) now.QuadPart) / (double) win_frequency.QuadPart);
#endif
}

#define PROGRAM_NAME "volume"
#define WAV_HEADER_SIZE 44

static uint8_t WAV_HEADER[WAV_HEADER_SIZE];

int main(int argc, char *argv[])
{
    int program_status = EXIT_SUCCESS;

    FILE *input_file_handle  = NULL;
    FILE *output_file_handle = NULL;

    int8_t *file_content = NULL;

    if (argc != 4) {
        fprintf(
            stderr,
            "Usage:\n"
                "\t%s <path to the input `.wav` file> <path to the output `.wav` file> <volume scale>\n",
            PROGRAM_NAME
        );

        program_status = EXIT_FAILURE;
        goto end;
    }

    const char *input_file_path = (const char *) argv[1];
    input_file_handle = fopen(input_file_path, "rb");
    if (input_file_handle == NULL) {
        perror("Failed to open the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    const char *output_file_path = (const char *) argv[2];
    output_file_handle = fopen(output_file_path, "wb");
    if (output_file_handle == NULL) {
        perror("Failed to open the output `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    float volume_scale = strtof(argv[3], NULL);

    if (fseek(input_file_handle, 0, SEEK_END) < 0) {
        perror("Failed to find the end of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    long file_size = ftell(input_file_handle);
    if (fseek(input_file_handle, 0, SEEK_SET) < 0) {
        perror("Failed to find the beginning of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    if (file_size - WAV_HEADER_SIZE <= 0) {
        fputs("The size of the input `.wav` file is incorrect.", stderr);

        program_status = EXIT_FAILURE;
        goto end;
    }
    file_size -= WAV_HEADER_SIZE;

    if (fread(WAV_HEADER, WAV_HEADER_SIZE, 1, input_file_handle) != 1) {
        perror("Failed to read the header of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    if (fwrite(WAV_HEADER, WAV_HEADER_SIZE, 1, output_file_handle) != 1) {
        perror("Failed to write the header to the output `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

#ifdef __linux
    file_content = (int8_t *) aligned_alloc(32, file_size);
#elif __APPLE__
    file_content = (int8_t *) aligned_alloc(16, file_size);
#elif _WIN32
    file_content = (int8_t *) _aligned_malloc(file_size, 32);
#endif
    if (file_content == NULL) {
        perror("Not enough memory to load the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    if (fread(file_content, file_size, 1, input_file_handle) != 1) {
        perror("Failed to read the data of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    uint64_t start_time = ns();

#ifndef USE_INTRINSICS
    int32_t *samples = (int32_t *) file_content;
    for (size_t i = 0, end = file_size / sizeof(int32_t); i < end; ++i) {
        samples[i] = (int32_t) ((float) samples[i] * volume_scale);
    }
#else
#ifdef __AVX512F__
    // TODO: write your SIMD AVX-512 code here
#elif __AVX2__
    // TODO: write your SIMD AVX/AVX2 code here
#elif __ARM_NEON
    // OPTIONAL TODO: write your SIMD ARM Neon code here
#endif /* __AVX512F__ */
#endif /* USE_INTRINSICS */

    uint64_t stop_time = ns();
    uint64_t delta_time = stop_time - start_time;
    printf(
        "%llu sec, %llu ms, %llu ns\n",
        delta_time / 1000000000ll,
        delta_time % 1000000000ll / 1000000ll,
        delta_time % 1000000000ll % 1000000ll
    );

    if (fwrite(file_content, file_size, 1, output_file_handle) != 1) {
        perror("Failed to write the data of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

end:
    if (file_content != NULL) {
#if defined(__linux) || defined(__APPLE__)
        free(file_content);
#elif defined(_WIN32)
        _aligned_free(file_content);
#endif
        file_content = NULL;
    }

    if (output_file_handle != NULL) {
        fclose(output_file_handle);
        output_file_handle = NULL;
    }

    if (input_file_handle != NULL) {
        fclose(input_file_handle);
        input_file_handle = NULL;
    }

    return program_status;
}
