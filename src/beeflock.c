/*
** beeflock - lock file and execute commands
**
** Copyright (C) 2009-2011
**       Marius Tolzmann <tolzmann@molgen.mpg.de>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/file.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <sysexits.h>

#include "bee_getopt.h"

#define BEE_EX_OFFSET 96

/*
 * to avoid conflicts with exit codes of executed program:
 * use error codes from sysexits.h with offset
 * use BEE_EXIT(SOFTWARE) for undefined exit codes
 */
#define BEE_EXIT_CODE(code) ((EX_ ## code)-EX__BASE+BEE_EX_OFFSET)
#define BEE_EXIT(code) exit(BEE_EXIT_CODE(code))


void usage(void)
{
   printf(
      "Usage: beeflock [options] lockfile command...\n\n"
      "  -x, --exclusive    acquire an exclusive/write lock (default)\n"
      "  -s, --shared       acquire a shared/read lock\n"
      "\n"
   );
}

#define BEEFLOCK_EXCLUSIVE  LOCK_EX
#define BEEFLOCK_SHARED     LOCK_SH
#define BEEFLOCK_UNLOCK     LOCK_UN
#define BEEFLOCK_NOBLOCK    LOCK_NB

int bee_flock_fd(int fd, int operation, int flags)
{
    int res;

    res = flock(fd, operation);

    return res;
}

#define DEBUG

#ifdef DEBUG
#define BEE_ERROR_DEBUG_FORMAT " (%s:%d:%s)"
#define BEE_ERROR_DEBUG_FORMATARGS ,__FILE__,__LINE__,__FUNCTION__
#else
#define BEE_ERROR_DEBUG_FORMAT
#define BEE_ERROR_DEBUG_FORMATARGS
#endif

#define BEE_ERROR(format, ...) \
    fprintf(stderr, "beeflock" BEE_ERROR_DEBUG_FORMAT ": " format ": %m\n" BEE_ERROR_DEBUG_FORMATARGS, ## __VA_ARGS__);

static int open_and_stat(const char *pathname, int flags, mode_t mode, struct stat *buf)
{
    int fd;
    int res;

    fd = open(pathname, flags, mode);
    if(fd < 0) {
        BEE_ERROR("%s", pathname);
        return -1;
    }

    res = fstat(fd, buf);

    if(res < 0) {
        perror("open_and_stat: fstat");
        res = close(fd);
        if (res < 0)
            perror("open_and_stat: close");
        return -1;
    }

    return fd;
}

int bee_flock_close(int fd)
{
    int res;

    res = close(fd);
    if (res < 0)
        perror("bee_flock_close: close");

    return res;
}

int bee_flock(char *filename, int operation, int flags)
{
    int res;
    int fd, fd2;
    struct stat statbuf_pre, statbuf_post;

    fd = open_and_stat(filename, O_RDONLY|O_CREAT|O_NOCTTY, 0666, &statbuf_pre);
    if (fd < 0)
        return -1;

    while (1) {
        res = bee_flock_fd(fd, operation, 0);
        if (res < 0)
            return -1;

        fd2 = open_and_stat(filename, O_RDONLY|O_CREAT|O_NOCTTY, 0666, &statbuf_post);
        if (fd2 < 0) {
            bee_flock_close(fd);
            return -1;
        }

        if (statbuf_pre.st_ino == statbuf_post.st_ino) {
            res = close(fd2);
            if (res < 0) {
                perror("bee_flock: close");
                bee_flock_close(fd);
                return -1;
            }
            return fd;
        }

        res = bee_flock_close(fd);
        if (res < 0) {
            res = close(fd2);
            if (res < 0)
                perror("bee_flock: close");
            return -1;
        }
        fd = fd2;
        statbuf_pre = statbuf_post;
    }

    return -1;
}


int bee_execute_command(char *command[], int flags)
{
    pid_t fpid, wpid;
    int wstatus;

    fpid = fork();
    if (fpid < 0) {
        perror("fork");
        return -1;
    } else if (fpid == 0) { /* child */
        execvp(command[0], command);
        perror("execvp");
        BEE_EXIT(OSERR);
    } else { /* parent */
        do {
            wpid = waitpid(fpid, &wstatus, 0);
        } while (wpid != fpid);
    }

    return WEXITSTATUS(wstatus);
}


int main(int argc, char *argv[])
{
    struct bee_getopt_ctl optctl;

    struct bee_option opts[] = {
        BEE_OPTION_NO_ARG("help",     'h'),
        BEE_OPTION_NO_ARG("version",  'V'),
        BEE_OPTION_NO_ARG("exclusive",  'x'),
        BEE_OPTION_NO_ARG("shared",  's'),
        BEE_OPTION_END
    };

    int opt;
    int i;

    int operation = BEEFLOCK_EXCLUSIVE;
    char *lockfilename;
    char **command;
    int fd;
    int res;

    bee_getopt_init(&optctl, argc-1, &argv[1], opts);

    optctl.program = "beeflock";
    optctl.flags = BEE_FLAG_SKIPUNKNOWN|BEE_FLAG_STOPONNOOPT|BEE_FLAG_STOPONUNKNOWN;

    while((opt=bee_getopt(&optctl, &i)) != BEE_GETOPT_END) {

        if (opt == BEE_GETOPT_ERROR) {
            BEE_EXIT(USAGE);
        }

        switch(opt) {
            case 'V':
                printf("beeflock Vx.x\n");
                exit(0);
                break;

            case 'h':
                usage();
                exit(0);
                break;

            case 's':
                operation = BEEFLOCK_SHARED;
                break;

            case 'x':
                operation = BEEFLOCK_EXCLUSIVE;
                break;
        }
    }

    argv = &optctl.argv[optctl.optind];
    argc = optctl.argc-optctl.optind;

    lockfilename = argv[0];
    if (!lockfilename) {
       usage();
       BEE_EXIT(USAGE);
    }

    command = &argv[1];
    if (!command[0]) {
       usage();
       BEE_EXIT(USAGE);
    }

    res = setenv("BEE_BEEFLOCK_LOCKEDFILE", lockfilename, 0);
    if (res < 0) {
        BEE_EXIT(OSERR);
    }

    fd = bee_flock(lockfilename, operation, 0);
    if (fd < 0) {
        BEE_EXIT(SOFTWARE);
    }

    res = bee_execute_command(command, 0);
    bee_flock_close(fd);
    if (res < 0) {
        BEE_EXIT(SOFTWARE);
    }
    return (res < 0) ? BEE_EXIT_CODE(SOFTWARE) : res;
}
