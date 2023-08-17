#include "include/rvvk.h"

typedef struct {
    const char *name;
    const char *alias;
    int need_arg;
    void (*handle_opt)(const char *arg);
    const char *desc;
} option_t;


static void print_usage();
static void handle_help(const char *arg)
{
    print_usage();
    exit(0);
}

/* Mapping */
unsigned long guest_base = 0x400000000000;
static void handle_guest_base(const char *arg)
{
    guest_base = strtoul(arg, NULL, 0);
}

target_ulong guest_stack_size = 8 * MiB;
static void handle_guest_stack(const char *arg)
{
    guest_stack_size = (target_ulong) strtoul(arg, NULL, 0);
}

int no_printf(const char *fmt, ...)
{
    return 0;
}

log_handler logoutf = no_printf;
static void handle_debug(const char *arg)
{
    logoutf = printf;
}


#define END_OPTION {NULL, NULL, 0, NULL, NULL}
static option_t opt_table[] = {
    { "--help",    "-h", 0, handle_help,        "display this message."   },
    { "--address", "-B", 1, handle_guest_base,  "set guest base address." },
    { "--stack",   "-s", 1, handle_guest_stack, "set guest stack size." },
    { "--debug",   "-d", 0, handle_debug,       "display debug logs." },
    END_OPTION
};

static void print_usage()
{
    option_t *opt;
    fprintf(stdout,
            "Usage:\n"
            "\trvvk [options] prog [args]\n"
            "Options:\n");
    for (opt = opt_table; opt->name != NULL; opt++) {
        fprintf(stdout,
                "\t%s%c %s\t%s\n",
                opt->alias ? opt->alias : "  ",
                opt->alias ? ',' : ' ',
                opt->name, opt->desc);
    }
}

static option_t *find_option(const char *name)
{
    option_t *opt;
    if (!name) {
        return NULL;
    }
    for (opt = opt_table; opt->name != NULL; opt++) {
        if (!strcmp(name, opt->name)) {
            return opt;
        }
        if (opt->alias && !strcmp(name, opt->alias)) {
            return opt;
        }
    }
    return NULL;
}

char *strdupf(const char *fmt, ...)
{
    va_list ap;
    char buf[256];
    char *str;

    va_start(ap, fmt);
    memset(buf, 0, sizeof(buf));
    vsprintf(buf, fmt, ap);
    str = malloc(strlen(buf) + 1);
    memcpy(str, buf, strlen(buf) + 1);

    va_end(ap);
    return str;
}

int parse_options(char **opts, char **prog_path, char ***prog_argv,
        char ***prog_envp, errinfo_t *errp)
{
    const char *opt;
    const char *arg;
    option_t *option;

    for (; *opts != NULL; opts++) {
        arg = NULL;
        opt = *opts;
        if (opt == NULL || *opt != '-') {
            break;
        }
        option = find_option(opt);
        if (!option) {
            get_errinfo(errp, strdupf("Invalid option: %s.", *opts));
            return 1;
        }
        if (option->need_arg) {
            opts++;
            arg = *opts;
        }
        if (option->handle_opt) {
            option->handle_opt(arg);
        }
    }

    /* parse prog and return its args */
    if (*opts == NULL) {
        get_errinfo(errp, "No specified program to execute.");
        return 1;
    } else {
        *prog_path = *opts;
        *prog_argv = opts;
    }

    opts++;
    for (; *opts != NULL; opts++);
    opts++;

    *prog_envp = opts;

    return 0;
}

char *get_real_path(const char *path, char *real_path, errinfo_t *errp)
{
    char *rp1, *rp2;
    const char *p1, *p2;
    char buf[PATH_MAX];
    int n;

    if (path == NULL) {
        get_errinfo(errp, "Can't resolve NULL `path'.");
        return NULL;
    } else {
        p1 = p2 = path;
    }

    if (real_path == NULL) {
        rp1 = rp2 = buf;
    } else {
        rp1 = rp2 = real_path;
    }

    if (*p1 == '/') {
        p1++;
        p2++;
        rp2[0] = '/';
        rp2[1] = 0;
    } else {
        if (getcwd(rp2, PATH_MAX) == NULL) {
            get_syserr(errp);
            return NULL;
        }
        rp2 += strlen(rp2);
    }

    while (*p2) {
        p2 = strchr(p1, '/');
        if (!p2) {
            p2 = strchr(p1, 0);
        }
        switch (p2 - p1) {
        case 0:
            break;
        case 1:
            if (p1[0] == '.') {
                break;
            }
        case 2:
            if (p1[0] == '.' && p1[1] == '.') {
                rp2 = strrchr(rp1, '/');
                if (rp2 != rp1) {
                    rp2[0] = 0;
                } else {
                    rp2[1] = 0;
                }
                break;
            }
        default:
            *rp2 = '/';
            rp2++;
            memcpy(rp2, p1, p2 - p1);
            rp2 += p2 - p1;
            break;
        }
        p1 = p2 + 1;
    };

    if (rp2 == rp1) {
        rp2++;
    }
    *rp2 = 0;

    if (real_path == NULL) {
        n = rp2 - rp1 + 1;
        rp1 = malloc(n);
        if (rp1 == NULL) {
            get_errinfo(errp, "Can't allocate extra memory.");
            return NULL;
        }
        memcpy(rp1, buf, n);
    }

    return rp1;
}

ssize_t readn(int fd, char *buf, size_t count)
{
    ssize_t n;
    while ((n = read(fd, buf, count)) < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            continue;
        } else {
            break;
        }
    }
    return n;
}

#ifdef __STRICT_ANSI__
ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    ssize_t n;

    offset = lseek(fd, offset, SEEK_SET);
    if (offset == (off_t) -1) {
        return -1;
    }

    n = readn(fd, buf, count);

    offset = lseek(fd, 0, SEEK_SET);
    if (offset == (off_t) -1) {
        return -1;
    }

    return n;
}
#endif

