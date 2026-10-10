// SPDX-License-Identifier: MIT
#ifndef UTILS_H_18_09_2026
#define UTILS_H_18_09_2026
////////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <string.h>
////////////////////////////////////////////////////////////////////////////////


struct clip_type {
    void *              data;
    size_t              size;
    struct clip_type *  next;
};

static ssize_t beetcone_stdout(
    const char *                            str_src,
    size_t                                  str_src_size

    // Writes the given buffer into the program's standard output fully.
    //
    // Returns the number of bytes written or -1 to indicate an error.
);

static const char *clip_c_str(
    const struct clip_type *                clip
);

static void *clip_set_data(
    struct clip_type *                      clip,
    const void *                            data,
    size_t                                  size
);

static struct clip_type *new_clip(
    struct clip_type **                     list
);

static void free_clip(
    struct clip_type *                      clip
);
////////////////////////////////////////////////////////////////////////////////

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

static inline const char *clip_c_str(const struct clip_type *clip) {
    return (const char *) (clip->data);
}

static inline struct clip_type *new_clip(
    struct clip_type **list
) {
    static struct clip_type zero;

    struct clip_type *clip = malloc(sizeof (struct clip_type));

    if (clip == nullptr) {
        return nullptr;
    }

    *clip = zero;

    if (*list == nullptr) {
        *list = clip;
    }
    else {
        clip->next = *list;
        *list = clip;
    }

    return clip;
}

static inline void free_clip(struct clip_type *clip) {
    if (clip->data) {
        free(clip->data);
    }

    free(clip);
}

static void *clip_set_data(
    struct clip_type *clip, const void *data, size_t size
) {
    if (clip->data) {
        free(clip->data);
    }

    clip->data = malloc(size * sizeof(uint8_t));

    if (clip->data) {
        clip->size = size;
        memcpy(clip->data, data, size);
    }
    else {
        clip->size = 0;
    }

    return clip->data;
}

#endif
