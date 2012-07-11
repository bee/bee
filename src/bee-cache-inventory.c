/*
** bee-cache-inventory - store bee's inventory data
**
** Copyright (C) 2012
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
**       Tobias Dreyer <dreyer@molgen.mpg.de>
**       and other bee developers
**
** This file is part of bee.
**
** bee is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bee_getopt.h"

#define BCI_MAJOR    1
#define BCI_MINOR    0
#define BCI_PATCHLVL 0

#define NOENT  0
#define ISFILE 1
#define ISDIR  2

#define BEE_STATIC_INLINE __attribute__((always_inline)) static inline

/* <pkg> <mtime> <uid> <gid> <mode> <size> <md5/md5 of symlink destination/type> <file without link destination> */
struct item {
    char *data;
    char *mtime;
    char *uid;
    char *gid;
    char *mode;
    char *size;
    char *md5;
    char *type;
    char *filename;
    char *destination;
};

struct inventory_meta {
    char *prepend;
    char *append;
    char *package;
    char *outfile;
    char *infile;

    int  multiplefiles;
    int  sync;
};

void print_version(void) {
    printf("bee-cache-inventory v%d.%d.%d - "
           "by Tobias Dreyer <dreyer@molgen.mpg.de> 2012\n",
           BCI_MAJOR, BCI_MINOR, BCI_PATCHLVL);
}

void print_full_usage(void) {

    puts("Usage:");
    puts("    bee-cache-inventory [options] <file|directory>");
    puts("");
    puts("Options:");
    puts("    -h | --help                      print this little help screen");
    puts("    -o | --output <file|directory>   move output to <file>");
    puts("                                     if <directory> is given, it is created in case it does not exist");
    puts("                                     and the output is placed in a file in that directory");
    puts("    -p | --prepend <text>            prepend <text> to each line of output");
    puts("    -a | --append <text>             append <text> to each line of output");
    puts("    -m | --multiplefiles             use -m to split output into multiple files");
}

void usage()
{
    print_version();
    print_full_usage();
}

int init_item(struct item *item)
{
    if(item == NULL)
        return 0;

    memset(item, 0, sizeof(*item));

    return 1;
}

int init_inventory_meta(struct inventory_meta *meta)
{
    if(meta == NULL)
        return 0;

    memset(meta, 0, sizeof(*meta));

    return 1;
}

int chomp(char *line)
{
    char *newline = NULL;

    if(line == NULL)
        return 0;

    newline = strrchr(line, '\n');

    if(newline == NULL)
        return 0;

    *newline = '\0';

    return 1;
}

int substitute(char *data, char from, char to)
{
    char *c = NULL;

    c = data;
    while((c = strchr(c, from)) != NULL) {
        *(c++) = to;
    }

    return 1;
}

BEE_STATIC_INLINE char *_extract_pattern(struct item *item, char **dest, char *hint, char *pattern, char size, int failok)
{
    char *p;

    p = strstr(hint, pattern);
    if (!p)
       p = strstr(item->data, pattern);
    if (!p) {
       if (failok)
           return NULL;
       fprintf(stderr, "syntax error while searching '%s' in '%s'\n", item->data, pattern);
       item->data = NULL;
       return NULL;
    }
    p += size;
    *dest = p;
    return p;
}

#define EXTRACT_PATTERN(item, dest, hint, size, failok) \
        ((hint) = _extract_pattern((item), &((item)->dest), (hint), #dest "=", (size)+1, (failok)))

int do_separation(char *line, struct item *item)
{
    char *p = NULL;

    /* type,mode,access,uid,user,gid,group,size,mtime,nlink,md5,file(//dest) */
    item->data = line;

    p = _extract_pattern(item, &(item->filename), item->data, ":file=", 6, 0);
    if (!p)
       return 0;

    *(p-6) = 0;

    /* get possible symlink destination */
    p = strstr(item->filename, "//");
    if(p != NULL) {
        item->destination = p + 2;
        *p = '\0';
    }

    p = item->data;
    if (!EXTRACT_PATTERN(item, type, p, 4, 0))
        return 0;
    if (!EXTRACT_PATTERN(item, mode, p, 4, 0))
        return 0;
    if (!EXTRACT_PATTERN(item, uid, p, 3, 0))
        return 0;
    if (!EXTRACT_PATTERN(item, gid, p, 3, 0))
        return 0;
    if (!EXTRACT_PATTERN(item, size, p, 4, 0))
        return 0;
    if (!EXTRACT_PATTERN(item, mtime, p, 5, 0))
        return 0;
    EXTRACT_PATTERN(item, md5, p, 3, 1);

    substitute(item->data, ':', '\0');

    return 1;
}

/* <pkg> <mtime> <uid> <gid> <mode> <size> <md5/md5 of symlink destination/type> <file without link destination> */
int print_item(FILE *out, struct item item, struct inventory_meta meta)
{
    char *c;

    if(meta.prepend)
        fputs(meta.prepend, out);

    if(meta.package) {
        fputs(meta.package, out);
        fputc(' ', out);
    }

    fputs(item.mtime, out);

    fputc(' ', out);
    fputs(item.uid, out);

    fputc(' ', out);
    fputs(item.gid, out);

    fputc(' ', out);
    fputs(item.mode, out);

    fputc(' ', out);
    fputs(item.size, out);

    if(item.md5) {
        fputc(' ', out);
        fputs(item.md5, out);
    } else if(strcmp(item.type, "symlink") == 0) {
        fputc(' ', out);
        c = item.destination;
        while(*c != '\0') {
            if(*c == '%')
                fputs("%25", out);
            else if(*c == ' ')
                fputs("%20", out);
            else
                fputc(*c, out);

            c++;
        }
    } else {
        fputc(' ', out);
        fputs(item.type, out);
    }

    fputc(' ', out);
    fputs(item.filename, out);

    if(meta.append)
        fputs(meta.append, out);

    fputc('\n', out);

    return 1;
}

int inventarize_file(char *path, struct inventory_meta meta, FILE *outfile)
{
    FILE        *cf = NULL;
    struct item item;
    char        line[LINE_MAX];
    int         outfd = 0;

    assert(outfile);

    init_item(&item);

    cf = fopen(path, "r");
    if(cf == NULL) {
        fprintf(stderr, "failed to open '%s': %s\n", path, strerror(errno));
        return 0;
    }


    while(fgets(line, LINE_MAX, cf) != NULL) {
        chomp(line);
        if(! do_separation(line, &item)) {
            fprintf(stderr, "failed to separate '%s'\n", line);
            fclose(cf);
            return 0;
        }

        print_item(outfile, item, meta);

        init_item(&item);
    }

    fclose(cf);

    if(meta.sync && outfile != stdout) {
        outfd = fileno(outfile);
        if(outfd < 0) {
            fputs("failed to retrieve file descriptor", stderr);
            return 0;
        }

        if(fsync(outfd) < 0) {
            fprintf(stderr, "failed to sybc fd %d: %m\n", outfd);
            return 0;
        }
    }

    return 1;
}

int inventarize_dir(char *path, struct inventory_meta meta, FILE *outfile)
{
    DIR *dir = NULL;
    FILE *out = NULL;
    struct dirent *dirent = NULL;
    char *dirname = NULL;
    int length = 0, bufsize = 0;
    char *buf = NULL;
    char *packagename = NULL;
    char *filename    = NULL;

    dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "failed to open '%s': %m", path);
        return 0;
    }

    while ((dirent = readdir(dir))) {
        dirname = dirent->d_name;

        if (*dirname == '.')
            continue;

        length = strlen(path) + 1 + strlen(dirname) + 1 + strlen("CONTENT") + 1;
        if(length > bufsize) {
            free(buf);

            buf = calloc(length, sizeof(char));
            if (!buf) {
                fprintf(stderr, "failed to allocate memory: %m");
                closedir(dir);
                return 0;
            }

            bufsize = length;
        }

        snprintf(buf, bufsize, "%s/%s/CONTENT", path, dirname);

        meta.package = dirname;

        out = outfile;
        if (!out && meta.outfile) {
            packagename = meta.package;
            if (!meta.package) {
                packagename = "content";
            }

            filename = meta.outfile;
            if (meta.multiplefiles && packagename) {
                if (asprintf(&filename, "%s/%s.inv", meta.outfile, packagename) < 0) {
                    fprintf(stderr, "failed to create filename %s: %m\n", filename);
                    closedir(dir);
                    return 0;
                }
            }

            out = fopen(filename, "w");
            if (!out) {
                fprintf(stderr, "failed to open '%s' for appending: %m\n", filename);
                free(filename);
                closedir(dir);
                return 0;
            }
        } else if (!out) {
            closedir(dir);
            return 0;
        }

        if (!inventarize_file(buf, meta, out)) {
            fprintf(stderr, "inventarization of '%s' failed\n", buf);
            free(buf);
            free(filename);
            closedir(dir);
            if (!outfile)
                fclose(out);
            return 0;
        }
    }

    free(buf);
    closedir(dir);
    if (!outfile)
        fclose(out);

    return 1;
}

int inventarize(struct inventory_meta meta)
{
    int result = 0;
    FILE *outfile = NULL;
    char *filename = NULL;

    struct stat insb, outsb;

    if(stat(meta.infile, &insb)) {
        fprintf(stderr, "failed to stat '%s': %m", meta.infile);
        return 0;
    }

    if(!meta.outfile) {
        outfile = stdout;
    } else {
        result = stat(meta.outfile, &outsb);

        if(result < 0) {

            if(errno != ENOENT) {
                fprintf(stderr, "failed to stat '%s': %s\n", meta.outfile, strerror(errno));
                return 0;
            }

            if(meta.multiplefiles) {

                if(mkdir(meta.outfile, 0777) != 0) {
                    fprintf(stderr, "failed to create directory '%s': %s\n", meta.outfile, strerror(errno));
                    return 0;
                }

                outfile = NULL;

            } else {

                result = asprintf(&filename, "%s.%d", meta.outfile, getpid());
                if(result < 0) {
                    fprintf(stderr, "failed to create filename for %s: %m\n", meta.outfile);
                    return 0;
                }
                if((outfile = fopen(meta.outfile, "w")) == NULL) {
                    fprintf(stderr, "failed to open %s: %m\n", meta.outfile);
                    free(filename);
                    return 0;
                }
                free(filename);

            }

        } else if(S_ISREG(outsb.st_mode)) {

            if((outfile = fopen(meta.outfile, "w")) == NULL) {
                fprintf(stderr, "failed to open %s: %m\n", meta.outfile);
                return 0;
            }

        } else if(S_ISDIR(outsb.st_mode)) {

            if(S_ISREG(insb.st_mode)) {
                fputs("cannot convert from file to directory\n", stderr);
                return 0;
            } else if(S_ISDIR(insb.st_mode)) {
                outfile = NULL;
                meta.multiplefiles = 1;
            } else {
                fprintf(stderr, "cannot read %s: it's neither file nor directory\n", meta.infile);
                return 0;
            }
        }

    }

    if(S_ISREG(insb.st_mode)) {
        result = inventarize_file(meta.infile, meta, outfile);
    } else if(S_ISDIR(insb.st_mode)) {
        result = inventarize_dir(meta.infile, meta, outfile);
    }

    if(outfile && outfile != stdout)
        fclose(outfile);

    return result;
}

/*
 * RETURN:
 *     0 .. successful
 *     1 .. general error
 */
int main(int argc, char *argv[])
{
    int opt = 0,
        optindex = 0;
    struct bee_getopt_ctl optctl;
    struct bee_option options[] = {
        BEE_OPTION_NO_ARG("help", 'h'),
        BEE_OPTION_REQUIRED_ARG("prepend", 'p'),
        BEE_OPTION_REQUIRED_ARG("append", 'a'),
        BEE_OPTION_REQUIRED_ARG("output", 'o'),
        BEE_OPTION_NO_ARG("multiple-files", 'm'),
        BEE_OPTION_NO_ARG("sync", 's'),
        BEE_OPTION_END
    };
    struct inventory_meta meta;

    if(argc < 2) {
        usage();
        return 1;
    }

    init_inventory_meta(&meta);

    bee_getopt_init(&optctl, argc-1, &argv[1], options);

    optctl.program = "bee-cache-inventory";

    while((opt=bee_getopt(&optctl, &optindex)) != BEE_GETOPT_END) {
        if (opt == BEE_GETOPT_ERROR) {
            return 1;
        }

        switch(opt) {
            case 'h':
                usage();
                return 1;
                break;

            case 'p':
                if(*optctl.optarg)
                    meta.prepend = optctl.optarg;
                break;

            case 'a':
                if(*optctl.optarg)
                    meta.append = optctl.optarg;
                break;

            case 'o':
                if(*optctl.optarg)
                    meta.outfile = optctl.optarg;

                break;

            case 'm':
                meta.multiplefiles = 1;
                break;

            case 's':
                meta.sync = 1;
                break;
        }
    }

    if(meta.multiplefiles && meta.outfile == NULL) {
        fputs("cannot accept option -m without option -o <dir>\n", stderr);
        usage();
        return 1;
    }

    argv = &optctl.argv[optctl.optind];
    argc = optctl.argc-optctl.optind;

    if(argc != 1) {
        usage();
        return 1;
    }

    meta.infile = argv[0];

    if(!inventarize(meta)) {
        fprintf(stderr, "inventarization failed for '%s'\n", meta.infile);
        return 1;
    }

    return 0;
}
