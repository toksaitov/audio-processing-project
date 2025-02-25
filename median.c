#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_INTRINSICS
#ifdef __AVX512F__
#include <immintrin.h>

void sort_8_doubles(double *array)
{
    // TODO: write your SIMD AVX-512 sorting code here
}

void sort_16_floats(float *array)
{
    // TODO: write your SIMD AVX-512 sorting code here
}
#else
#error "This code only supports x86-64 AVX-512 CPUs."
#endif /* __AVX512F__ */
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

static uint64_t ns()
{
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

#define PROGRAM_NAME "median"

#define WAV_RIFF_MAGIC 0x46464952
#define WAV_WAVE_MAGIC 0x45564157
#define WAV_FMT_MAGIC  0x20746D66
#define WAV_DATA_MAGIC 0x61746164
#define WAV_PCM_FORMAT 1

typedef struct wav_header
{
    uint32_t riff_magic;
    uint32_t file_size;
    uint32_t wave_magic;
    uint32_t fmt_magic;
    uint32_t fmt_chunk_size;
    uint16_t format_tag;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t bytes_per_second;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_magic;
    uint32_t data_size;
} __attribute__((__packed__)) wav_header_t;

static int compare_floats(const void *a, const void *b)
{
    const float *fa = (const float *) a;
    const float *fb = (const float *) b;
    return (*fa > *fb) - (*fa < *fb);
}

static int compare_doubles(const void *a, const void *b)
{
    const double *da = (const double *) a;
    const double *db = (const double *) b;
    return (*da > *db) - (*da < *db);
}

int main(int argc, char *argv[])
{
    int program_status = EXIT_SUCCESS;

    FILE *input_file_handle  = NULL;
    FILE *output_file_handle = NULL;

    wav_header_t wav_header;
    int8_t *file_content = NULL;

    int32_t *processed_samples = NULL;

    void *window = NULL;
    float  *float_window  = NULL;
    double *double_window = NULL;

    uint32_t endianness_test = 1;
    if ((*(uint8_t *) &endianness_test) != 1) {
        fputs("This program only works on little-endian systems\n", stderr);

        program_status = EXIT_FAILURE;
        goto end;
    }

    if (argc != 4) {
        fprintf(
            stderr,
            "Usage:\n"
                "\t%s <path to the input `.wav` file> <path to the output `.wav` file> <window sample size, 8 or 16>\n",
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

    char *endptr;
    long window_size_long = strtol(argv[3], &endptr, 10);
    if (endptr == argv[3] || *endptr != '\0') {
        fprintf(stderr, "The window size, 8 or 16, must be specified\n");
        program_status = EXIT_FAILURE;
        goto end;
    }
    if (window_size_long != 8 && window_size_long != 16) {
        fprintf(stderr, "The window size may only be 8 or 16\n");
        program_status = EXIT_FAILURE;
        goto end;
    }
    size_t window_size = (size_t) window_size_long;
    size_t window_radius = window_size / 2;

    if (fseek(input_file_handle, 0, SEEK_END) < 0) {
        perror("Failed to find the end of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }
    long file_size_long = ftell(input_file_handle);
    if (file_size_long < 0) {
        perror("Failed to determine the file size of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }
    size_t file_size = (size_t) file_size_long;
    if (fseek(input_file_handle, 0, SEEK_SET) < 0) {
        perror("Failed to find the beginning of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (file_size_long - (long) sizeof(wav_header) <= 0) {
        fputs("The size of the input `.wav` file is incorrect\n", stderr);

        program_status = EXIT_FAILURE;
        goto end;
    }
    file_size -= sizeof(wav_header);
    if (file_size % sizeof(int32_t) != 0) {
        fputs("The size of the input `.wav` file data is not valid for a bit depth of 32\n", stderr);

        program_status = EXIT_FAILURE;
        goto end;
    }

    if (fread(&wav_header, sizeof(wav_header), 1, input_file_handle) != 1) {
        perror("Failed to read the header of the input `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.riff_magic != WAV_RIFF_MAGIC) {
        fprintf(stderr, "The input `.wav` file does not have the `RIFF` magic\n");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.wave_magic != WAV_WAVE_MAGIC) {
        fprintf(stderr, "The input `.wav` file does not have the `WAVE` magic\n");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.fmt_magic != WAV_FMT_MAGIC) {
        fprintf(stderr, "The input `.wav` file does not have the `FMT ` magic\n");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.data_magic != WAV_DATA_MAGIC) {
        fprintf(stderr, "The input `.wav` file does not have the `data` magic %x\n", wav_header.data_magic);
        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.format_tag != WAV_PCM_FORMAT) {
        fprintf(stderr, "The input `.wav` file must be of a PCM format\n");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.channels != 1) {
        fprintf(stderr, "The input `.wav` file must be mono\n");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.bits_per_sample != 32) {
        fprintf(stderr, "The input `.wav` file must have a 32 integer bit depth\n");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (wav_header.data_size != (uint32_t) file_size) {
        fprintf(stderr, "The input `.wav` file sample data size does not match the file size\n");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (fwrite(&wav_header, sizeof(wav_header), 1, output_file_handle) != 1) {
        perror("Failed to write the header to the output `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

    file_content = (int8_t *) malloc(file_size);
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
    int32_t *original_samples = (int32_t *) file_content;
    size_t total_samples = file_size / sizeof(*original_samples);
    if (total_samples < window_size) {
        fprintf(stderr, "The input `.wav` file is too small\n");

        program_status = EXIT_FAILURE;
        goto end;
    }

    processed_samples = (int32_t *) malloc(file_size);
    if (processed_samples == NULL) {
        perror("Not enough memory to store the processed audio samples");

        program_status = EXIT_FAILURE;
        goto end;
    }
    memcpy(processed_samples, original_samples, file_size);

    size_t window_bytes = window_size * (window_size == 8 ? sizeof(double) : sizeof(float));
#if defined(__linux) || defined(__APPLE__)
    window = (float *) aligned_alloc(32, window_bytes);
#elif defined(_WIN32)
    window = (float *) _aligned_malloc(window_bytes, 32);
#else
#error "This code only supports latest versions of GNU/Linux, macOS, and Windows."
#endif
    if (window == NULL) {
        perror("Not enough memory to store the filtering window");

        program_status = EXIT_FAILURE;
        goto end;
    }
    if (window_size == 8) {
        double_window = (double *) window;
    } else {
        float_window  = (float *) window;
    }

    uint64_t start_time = ns();
    {
        for (size_t i = window_radius, end = total_samples - window_radius; i < end; ++i) {
            if (window_size == 8) {
                for (size_t j = 0; j < window_size; ++j) {
                    double_window[j] = (double) original_samples[i - window_radius + j];
                }
            } else {
                for (size_t j = 0; j < window_size; ++j) {
                    float_window[j] = (float) original_samples[i - window_radius + j];
                }
            }

#ifndef USE_INTRINSICS
            if (window_size == 8) {
                qsort(double_window, window_size, sizeof(*double_window), compare_doubles);
            } else {
                qsort(float_window, window_size, sizeof(*float_window), compare_floats);
            }
#else
#ifdef __AVX512F__
            if (window_size == 8) {
                sort_8_doubles(double_window);
            } else {
                sort_16_floats(float_window);
            }
#else
#error "This code only supports x86-64 AVX-512 CPUs."
#endif /* __AVX512F__ */
#endif /* USE_INTRINSICS */

            if (window_size == 8) {
                double median;
                if (window_size % 2 == 0) {
                    median = (double_window[window_radius - 1] + double_window[window_radius]) * 0.5f;
                } else {
                    median = double_window[window_radius];
                }
                processed_samples[i] = (int32_t) median;
            } else {
                float median;
                if (window_size % 2 == 0) {
                    median = (float_window[window_radius - 1] + float_window[window_radius]) * 0.5f;
                } else {
                    median = float_window[window_radius];
                }
                processed_samples[i] = (int32_t) median;
            }
        }
    }
    uint64_t stop_time = ns();
    uint64_t delta_time = stop_time - start_time;
    printf(
        "%" PRIu64 " ns total\n%" PRIu64 " sec, %" PRIu64 " ms, %" PRIu64 " ns\n",
         delta_time,
         delta_time / UINT64_C(1000000000),
        (delta_time % UINT64_C(1000000000)) / UINT64_C(1000000),
        (delta_time % UINT64_C(1000000000)) % UINT64_C(1000000)
    );

    memcpy(file_content, processed_samples, file_size);
    if (fwrite(file_content, file_size, 1, output_file_handle) != 1) {
        perror("Failed to write the data to the output `.wav` file");

        program_status = EXIT_FAILURE;
        goto end;
    }

end:
    if (file_content != NULL) {
        free(file_content);
        file_content = NULL;
    }

    if (processed_samples != NULL) {
        free(processed_samples);
        processed_samples = NULL;
    }

    if (window != NULL) {
#if defined(__linux) || defined(__APPLE__)
        free(window);
#elif defined(_WIN32)
        _aligned_free(window);
#else
#error "This code only supports latest versions of GNU/Linux, macOS, and Windows."
#endif
        window = NULL;
        float_window  = NULL;
        double_window = NULL;
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
