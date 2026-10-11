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

static void *clip_reserve(
    struct clip_type *                      clip,
    size_t                                  size
);

static struct clip_type *new_clip(
    struct clip_type **                     list
);

static void free_clip(
    struct clip_type *                      clip
);

static void clip_from_list(
    struct clip_type *                      clip,
    struct clip_type **                     list
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

    size_t real_size = size + 1;

    clip->data = malloc(real_size * sizeof(uint8_t));

    if (clip->data) {
        clip->size = size;
        memcpy(clip->data, data, size);
        ((char *) clip->data)[size] = '\0';
    }
    else {
        clip->size = 0;
    }

    return clip->data;
}

static void *clip_reserve(
    struct clip_type *clip, size_t size
) {
    if (clip->data) {
        free(clip->data);
    }

    size_t real_size = size + 1;

    clip->data = calloc(real_size, sizeof(uint8_t));

    if (clip->data) {
        clip->size = size;
    }
    else {
        clip->size = 0;
    }

    return clip->data;
}

static inline struct {
    char *data;
    size_t size;
} file_to_memory(FILE *fp) {
    static constexpr size_t INITIAL_BUFFER_CAPACITY = 512;
    size_t data_size = 0, data_capacity = INITIAL_BUFFER_CAPACITY;
    char *data = nullptr;
    typeof(file_to_memory(nullptr)) blob = {};

    if (fp == nullptr) {
        const char *error_message = "fopen: error opening file\n";
        [[maybe_unused]] ssize_t bytes_written = write(
            2, error_message, strlen(error_message)
        );

        return blob;
    }

    for (;;) {
        size_t bytes_read, bytes_to_read;

        data = realloc(data, data_capacity);

        if (data == nullptr) {
            const char *error_message = "realloc: memory allocation failed\n";
            [[maybe_unused]] ssize_t bytes_written = write(
                2, error_message, strlen(error_message)
            );

            return blob;
        }

        bytes_to_read = data_capacity - data_size;
        bytes_read = fread(data + data_size, 1, bytes_to_read, fp);
        data_size += bytes_read;

        if (bytes_read != bytes_to_read) {
            break;
        }

        data_capacity *= 2;
    }

    blob.data = data;
    blob.size = data_size;

    return blob;
}

static void clip_from_list(struct clip_type *clip, struct clip_type **list) {
    if (*list == clip) {
        *list = clip->next;
        clip->next = nullptr;
        return;
    }

    for (struct clip_type *c = *list; c; c = c->next) {
        if (c->next == clip) {
            c->next = clip->next;
            clip->next = nullptr;
            return;
        }
    }
}

#endif
