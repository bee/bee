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
#include <stdarg.h>
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

short chomp(char *line)
{
    char *newline;

    newline = strrchr(line, '\n');
    assert(newline);

    if (!newline)
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
    char *p, *q;

    /* type,mode,access,uid,user,gid,group,size,mtime,nlink,md5,file(//dest) */
    item->data = line;

    p = _extract_pattern(item, &(item->filename), item->data, ":file=", 6, 0);
    if (!p)
       return 0;

    /* get possible symlink destination */
    q = strstr(item->filename, "//");
    if (q) {
        item->destination = q + 2;
        if (!*item->destination) {
            fprintf (stderr, "bee-cache-inventory: syntax error: empty destination for file '%s'\n", item->filename);
            return 0;
        }
        *q = '\0';
    }

    *(p-6) = 0;

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

    fputc(' ', out);
    if(item.md5) {
        fputs(item.md5, out);
    } else if(strcmp(item.type, "symlink") == 0) {
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
        fputs(item.type, out);
    }

    fputc(' ', out);
    fputs(item.filename, out);

    if(meta.append)
        fputs(meta.append, out);

    fputc('\n', out);

    return 1;
}

FILE *fopenf(char *mode, char *format, ...)
{
    FILE *fh;
    int res;
    char *fname;
    va_list list;

    assert(mode);
    assert(format);

    va_start(list, format);
    res = vasprintf(&fname, format, list);
    va_end(list);
    if (res < 0) {
        perror("vasprintf");
        return NULL;
    }

    fh = fopen(fname, mode);

    free(fname);

    return fh;
}

int renamef(char *dest, char *source, ...)
{
    int res;
    char *srcfname;
    va_list list;

    assert(source);

    va_start(list, source);
    res = vasprintf(&srcfname, source, list);
    va_end(list);
    if(!res) {
        perror("vasprintf");
        return 0;
    }

    res = rename(srcfname, dest);
    if(res) {
        perror("rename failed");
        free(srcfname);
        return 0;
    }

    free(srcfname);

    return 1;
}

int inventory_fhfh(FILE *infh, FILE *outfh, struct inventory_meta meta)
{
    char line[LINE_MAX];
    struct item item;

    assert(infh);
    assert(outfh);

    while (fgets(line, LINE_MAX, infh) != NULL) {
        chomp(line);
        init_item(&item);
        if (!do_separation(line, &item)) {
            fprintf(stderr, "bee-cache-inventory: syntax error in '%s'\n", line);
            return 0;
        }

        print_item(outfh, item, meta);

    }

    return 1;
}

int inventory_filefile(char *infname, char *outfname, struct inventory_meta meta)
{
    int res = 1;
    FILE *infh;
    FILE *outfh;
    pid_t pid;

    assert(infname);

    infh = fopenf("r", "%s", infname);
    if (!infh) {
        if (errno == ENOENT || errno == ENOTDIR)
            return 1;
        fprintf(stderr, "failed to open file %s: %m\n", infname);
        return 0;
    }

    if (outfname) {
        pid = getpid();
        outfh = fopenf("w", "%s.%d", outfname, pid);
        if (!outfh) {
            fprintf(stderr, "failed to open file %s: %m\n", outfname);
            fclose(infh);
            return 0;
        }
    } else {
        outfh = stdout;
    }

    res = inventory_fhfh(infh, outfh, meta);

    if (outfname) {
        res = renamef(outfname, "%s.%d", outfname, pid);
        if(!res)
            perror("rename failed");
    }

    fclose(infh);
    fclose(outfh);

    return res;
}

static void _strip_trailing(char *in, char c)
{
    char *p;
    size_t len;

    assert(in);

    len = strlen(in);
    p   = in+len-1;

    while (p > in && *p == '/')
        *(p--) = 0;

}

int inventory_dirfile(char *indname, char *outfname, struct inventory_meta meta)
{
    int res = 1;
    DIR *indh;
    struct dirent *indent;
    char *dirname;
    FILE *infh;
    FILE *outfh;
    pid_t pid;

    assert(indname);

    _strip_trailing(indname, '/');

    indh = opendir(indname);
    if (!indh) {
        fprintf(stderr, "failed to open dir %s: %m\n", indname);
        return 0;
    }

    if (outfname) {
        pid = getpid();
        outfh = fopenf("w", "%s.%d", outfname, pid);
        if (!outfh) {
            fprintf(stderr, "failed to open file %s: %m\n", outfname);
            res = 0;
            goto closedir;
        }
    } else {
        outfh = stdout;
    }

    while ((indent = readdir(indh))) {
        dirname = indent->d_name;

        if (*dirname == '.')
            continue;

        infh = fopenf("r", "%s/%s/CONTENT", indname, dirname);
        if (!infh) {
            if (errno == ENOENT || errno == ENOTDIR)
                continue;
            fprintf(stderr, "failed to open file %s/%s/CONTENT: %m\n", indname, dirname);
            res = 0;
            goto closeoutfh;
        }

        meta.package = dirname;

        res = inventory_fhfh(infh, outfh, meta);
        if (!res) {
            fprintf(stderr, "inventarization from %s/%s/CONTENT to %s failed: %m\n", indname, dirname, outfname);
            fclose(infh);
            goto closeoutfh;
        }

        fclose(infh);
    }

    if (outfname) {
        res = renamef(outfname, "%s.%d", outfname, pid);
        if(!res)
            perror("rename failed");
    }

closeoutfh:
    if (outfname)
        fclose(outfh);

closedir:
    closedir(indh);

    return res;
}

int inventory_dirdir(char *indname, char *outdname, struct inventory_meta meta)
{
    int res;
    int ret = 1;
    DIR *indh;
    struct dirent *indent;
    char *dirname;
    char *infname;
    char *outfname;

    assert(indname);
    assert(outdname);

    _strip_trailing(indname, '/');

    indh = opendir(indname);
    if (!indh) {
        fprintf(stderr, "failed to open dir %s: %m\n", indname);
        return 0;
    }

    while ((indent = readdir(indh))) {
        dirname = indent->d_name;

        if (*dirname == '.')
            continue;

        res = asprintf(&infname, "%s/%s/CONTENT", indname, dirname);
        if (res < 0) {
            perror("asprintf");
            ret = 0;
            break;
        }

        res = asprintf(&outfname, "%s/%s.inv", outdname, dirname);
        if (res < 0) {
            perror("asprintf");
            free(infname);
            ret = 0;
            break;
        }

        meta.package = dirname;

        res = inventory_filefile(infname, outfname, meta);

        free(infname);
        free(outfname);

        if(!res) {
            ret = 0;
            break;
        }
    }

    closedir(indh);
    return ret;
}

int inventory(struct inventory_meta meta)
{
    int res;
    struct stat insb;
    struct stat outsb;

    res = stat(meta.infile, &insb);
    if (res < 0) {
        fprintf(stderr, "bee-cache-inventory: %s: %m\n", meta.infile);
        return 0;
    }

    if (res == 0 && !S_ISDIR(insb.st_mode) && !S_ISREG(insb.st_mode)) {
        fprintf(stderr, "bee-cache-inventory: %s: Invalid filetype.\n", meta.infile);
        return 0;
    }

    if (meta.outfile) {

        res = stat(meta.outfile, &outsb);
        if (res < 0 && errno != ENOENT) {
            fprintf(stderr, "bee-cache-inventory: %s: %m\n", meta.outfile);
            return 0;
        }

        if (res == 0 && !S_ISDIR(outsb.st_mode) && !S_ISREG(outsb.st_mode)) {
            fprintf(stderr, "bee-cache-inventory: %s: Invalid filetype.\n", meta.outfile);
            return 0;
        }

        if (res == 0 && S_ISDIR(outsb.st_mode)) {
             meta.multiplefiles = 1;
        } else if (meta.multiplefiles) {
            if (res == 0) {
                if (!S_ISDIR(outsb.st_mode)) {
                    errno = EEXIST;
                    fprintf(stderr, "bee-cache-inventory: %s: %m\n", meta.outfile);
                    return 0;
                }
            }
            if (res < 0) {
                res = mkdir(meta.outfile, 0777);
                if (res < 0) {
                    fprintf(stderr, "bee-cache-inventory: %s: %m\n", meta.outfile);
                    return 0;
                }
            }
        }
    }

    if (S_ISREG(insb.st_mode)) {
        if (!meta.multiplefiles) {
            return inventory_filefile(meta.infile, meta.outfile, meta);
        }
        errno = ENOTDIR;
        fprintf(stderr, "bee-cache-inventory: %s: %m\n", meta.infile);
        return 0;
    }

    if (S_ISDIR(insb.st_mode)) {
        if (!meta.multiplefiles) {
            return inventory_dirfile(meta.infile, meta.outfile, meta);
        }
        return inventory_dirdir(meta.infile, meta.outfile, meta);
    }

    assert(0);

    return 0;
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

    if(!inventory(meta)) {
        fprintf(stderr, "bee-cache-inventory: %s: Inventarization failed\n", meta.infile);
        return 1;
    }

    if (meta.sync)
        sync();

    return 0;
}
