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

static char *read_file_to_memory(
    const char *                            filename,
    size_t *                                filesize
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

static inline char* read_file_to_memory(
    const char *filename, size_t *filesize
) {
    // Reads a file into a memory buffer. Sets filesize to bytes read.
    // Returns a pointer to the buffer (caller must free), or NULL on error.

    if (filesize) {
        *filesize = 0;
    }

    FILE *f = fopen(filename, "rb");

    if (!f) {
        return nullptr;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (length < 0) {
        fclose(f);
        return nullptr;
    }

    // Allocate memory
    const size_t bufsz = (size_t) (length + 1);
    char *buffer = malloc(bufsz);

    if (!buffer) {
        fclose(f);
        return nullptr;
    }

    // Read content
    size_t read_size = fread(buffer, 1, bufsz, f);
    buffer[read_size] = '\0'; // Null-terminate
    fclose(f);

    if (filesize) {
        *filesize = read_size;
    }

    if (read_size + 1 != bufsz) {
        free(buffer);
        return nullptr;
    }

    return buffer;
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
