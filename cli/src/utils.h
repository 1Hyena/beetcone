// SPDX-License-Identifier: MIT
#ifndef UTILS_H_18_09_2026
#define UTILS_H_18_09_2026
////////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <string.h>
////////////////////////////////////////////////////////////////////////////////

static ssize_t beetcone_stdout(
    const char *                            str_src,
    size_t                                  str_src_size

    // Writes the given buffer into the program's standard output fully.
    //
    // Returns the number of bytes written or -1 to indicate an error.
);

static inline ssize_t beetcone_stdout(
    const char *str_src, size_t str_src_size
) {
    size_t n = 0;

    str_src_size = str_src_size ? str_src_size : strlen(str_src);

    for (; n < str_src_size;) {
        ssize_t written = write(STDOUT_FILENO, str_src + n, str_src_size - n);

        if (written <= 0) {
            break;
        }

        n += (size_t) written;
    }

    return n == str_src_size ? (ssize_t) n : -1;
}

#endif
