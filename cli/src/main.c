// SPDX-License-Identifier: MIT
#include "../../beetcone.h"
#include "utils.h"
////////////////////////////////////////////////////////////////////////////////
#include <getopt.h>
////////////////////////////////////////////////////////////////////////////////

struct argument_type {
    const char *cstr;

    struct {
        bool file:1;
        bool hex:1;
    } bitset;

    struct argument_type *next;
};

static int do_help  (const struct argument_type **, size_t);
static int do_base58(const struct argument_type **, size_t);
static int do_pubkey(const struct argument_type **, size_t);
static int do_addr  (const struct argument_type **, size_t);

static void print_usage(FILE *);
static struct argument_type *new_argument(struct argument_type **list);
static size_t reverse_arguments(struct argument_type **list);
static int cleanup(int exit_code);
static struct clip_type *argument_to_clip(const struct argument_type *);

static const struct command_type {
    const char *name;
    int (*fun) (const struct argument_type **, size_t);
} command_table[] = {
    { .name = "help",   .fun    = do_help   },
    { .name = "base58", .fun    = do_base58 },
    { .name = "pubkey", .fun    = do_pubkey },
    { .name = "addr",   .fun    = do_addr   }
};

static constexpr size_t COMMAND_COUNT = (
    sizeof(command_table)/sizeof(command_table[0])
);

struct {
    char ** argv;
    int     argc;

    struct {
        struct argument_type *  arguments;
        struct clip_type *      clips;
    } heap;
} global;


int main(int main_argc, char *main_argv[]) {
    global.argv = main_argv;
    global.argc = main_argc;

    for (int skip_args = 0;;) {
        int     argc = main_argc - skip_args;
        char ** argv = main_argv + skip_args;

        int flg_hex = 0;

        struct option opt[] = {
            {"hex",         no_argument,       &flg_hex, 'x' },
            {"help",        no_argument,       0,        'h' },
            {"version",     no_argument,       0,        'v' },
            {0,             0,                 0,          0 }
        };

        int opt_i = 0;

        for (;;) {
            int result = getopt_long(argc, argv, "+hvx", opt, &opt_i);

            if (result == -1) {
                break;
            }

            switch (result) {
                case 'h': {
                    print_usage(stdout);
                    return cleanup(EXIT_SUCCESS);
                }
                case 'v': {
                    printf("%s\n", BEETCONE_VERSION);
                    return cleanup(EXIT_SUCCESS);
                }
                case 'x': {
                    flg_hex = 1;
                    break;
                }
                case 0: {
                    if (opt[opt_i].flag != 0) {
                        // If this option sets a flag, do nothing else.
                        break;
                    }
                } [[fallthrough]];
                default: {
                    print_usage(stderr);
                    return cleanup(EXIT_FAILURE);
                }
            }
        }

        if (optind >= argc) {
            if (global.heap.arguments == nullptr) {
                fprintf(stderr, "error: %s\n", "expected command name");
                print_usage(stderr);
                return cleanup(EXIT_FAILURE);
            }
            else if (global.heap.arguments->next == nullptr) {
                struct argument_type *arg = new_argument(
                    &global.heap.arguments
                );

                arg->cstr = "@-";
                arg->bitset.hex = flg_hex;
            }

            break;
        }

        struct argument_type *arg = new_argument(&global.heap.arguments);
        arg->cstr = argv[optind];
        arg->bitset.hex = flg_hex;

        skip_args += optind;
        optind = 1;
    }

    size_t argument_count = reverse_arguments(&global.heap.arguments);

    if (!argument_count) {
        abort();
    }

    struct argument_type *arguments[argument_count] = {};
    size_t argument_index = 0;

    for (auto arg = global.heap.arguments; arg; arg = arg->next) {
        arguments[argument_index++] = arg;
    }

    struct clip_type *command = argument_to_clip(arguments[0]);

    for (size_t i=0; i<COMMAND_COUNT; ++i) {
        if (strncasecmp(command->data, command_table[i].name, command->size)) {
            continue;
        }

        return cleanup(
            command_table[i].fun(
                (const struct argument_type **) arguments + 1,
                argument_count - 1
            )
        );
    }

    fprintf(stderr, "unknown command: %s\n", clip_c_str(command));

    return cleanup(EXIT_FAILURE);
}

static int do_help(const struct argument_type **args, size_t argc) {
    print_usage(stdout);

    return EXIT_SUCCESS;
}

static int do_base58(const struct argument_type **args, size_t argc) {
    if (argc < 1) {
        fprintf(stderr, "base58: %s\n", "not enough arguments provided");
        return EXIT_FAILURE;
    }

    struct clip_type *arg_clip = argument_to_clip(args[0]);
    const char *argument = clip_c_str(arg_clip);

    size_t bufsz = beetcone_base58_estimate_strlen(
        argument, strlen(argument)
    ) + 1;

    char buf[bufsz];

    beetcone_base58_encode(argument, strlen(argument), buf, bufsz);

    printf("%s\n", buf);

    return EXIT_SUCCESS;
}

static int do_pubkey(const struct argument_type **args, size_t argc) {
    if (argc < 1) {
        fprintf(stderr, "pubkey: %s\n", "not enough arguments provided");
        return EXIT_FAILURE;
    }

    struct clip_type *arg_clip = argument_to_clip(args[0]);
    const char *argument = clip_c_str(arg_clip);
    auto key = beetcone_make_key(argument, strlen(argument));

    if (!key.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    auto pubkey = beetcone_key2pubkey(key);

    if (!pubkey.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < sizeof(pubkey.data); ++i) {
        printf("%02x", pubkey.data[i]);
    }

    printf("%s", "\n");

    return EXIT_SUCCESS;
}

static int do_addr(const struct argument_type **args, size_t argc) {
    if (argc < 1) {
        fprintf(stderr, "addr: %s\n", "not enough arguments provided");
        return EXIT_FAILURE;
    }

    struct clip_type *arg_clip = argument_to_clip(args[0]);
    const char *argument = clip_c_str(arg_clip);
    auto key = beetcone_make_key(argument, strlen(argument));

    if (!key.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    auto addr = beetcone_key2addr(key);

    if (!addr.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    printf("%s\n", addr.base58);

    return EXIT_SUCCESS;
}

static void print_usage(FILE *fp) {
    fprintf(
        fp,
        "usage:  %s %s %s %s %s %s %s\n"
        "\n"
        "Where ARGUMENT contains @, treat it as a file path and read it.\n"
        "With no ARGUMENT or where ARGUMENT contains @-, read standard input.\n"
        "\n"
        "Available options:\n"
        "  -x  --hex             the following data is hex encoded\n"
        "  -h  --help            display this help and exit\n"
        "  -v  --version         display version info and exit\n",
        global.argv[0],
        "[OPTION]...", "<COMMAND>", "[", "[OPTION]", "[ARGUMENT]", "]..."
    );

    fprintf(fp, "%s", "\nAvailable commands:\n");

    for (size_t i=0; i<COMMAND_COUNT; ++i) {
        fprintf(fp, "    %s\n", command_table[i].name);
    }
}

static struct argument_type *new_argument(
    struct argument_type **list
) {
    static struct argument_type zero;

    struct argument_type *argument = malloc(sizeof (struct argument_type));

    if (argument == nullptr) {
        return nullptr;
    }

    *argument = zero;

    if (*list == nullptr) {
        *list = argument;
    }
    else {
        argument->next = *list;
        *list = argument;
    }

    return argument;
}

static int cleanup(int exit_code) {
    struct argument_type *a_next = nullptr;

    for (struct argument_type *a = global.heap.arguments; a; a = a_next) {
        a_next = a->next;
        free(a);
    }

    struct clip_type *c_next = nullptr;

    for (struct clip_type *c = global.heap.clips; c; c = c_next) {
        c_next = c->next;
        free_clip(c);
    }

    return exit_code;
}

static size_t reverse_arguments(struct argument_type **list) {
    if (*list == nullptr) {
        return 0;
    }

    if ((*list)->next == nullptr) {
        return 1;
    }

    size_t total = 0;

    struct argument_type *buf = nullptr;

    while (*list) {
        struct argument_type *a = *list;
        *list = a->next;
        a->next = buf;
        buf = a;
        ++total;
    }

    *list = buf;
    return total;
}

static struct clip_type *argument_to_clip(const struct argument_type *arg) {
    struct clip_type *clip = new_clip(&global.heap.clips);

    clip_set_data(clip, arg->cstr, strlen(arg->cstr) + 1);

    return clip;
}
