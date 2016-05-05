#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <time.h>
#include <sys/timeb.h>
#include "my_popen.h"
#include "log.h"

using namespace std;

static int do_fork = 1;
static int signup_flag = 1;

void signup (int signum) {
    switch (signum) {
    case SIGUSR1:
        break;
    case SIGUSR2:
        break;
    case SIGPIPE:
        printf ("Broken PIPE\n");
    case SIGHUP:
    case SIGTERM:
    case SIGABRT:
    case SIGINT: {
        if (signup_flag == 1) {
            /* Prepare Leave, free malloc*/
            signup_flag = 0;
        } else {
            printf ("Signup_flag is already 0\n");
        }
    }
    break;
    case SIGCHLD: {
        wait ( (int*) 0);
    }
    break;
    default:
        printf ("Do nothing, %d\n", signum);
        break;
    }
}

void init_signals (void) {
    signal (SIGTERM, signup);
    signal (SIGABRT, signup);
    signal (SIGUSR1, signup);
    signal (SIGUSR2, signup);
    signal (SIGPIPE, signup);
    signal (SIGCHLD, signup);
    signal (SIGINT, signup);
}

#define LOG_FILE    "/mnt/extsd/sys.log"

static vector<string> gbuf_pre;
static vector<string> gbuf_cur;

static void work_loop() {
    int i = 0;
    int fd, n;

    /* clear buf */
    gbuf_pre.clear();
    gbuf_cur.clear();
    for (int i = 0; i < 50; i++) {
        gbuf_pre.push_back (" ");
        gbuf_cur.push_back (" ");
    }

    while (signup_flag) {
        fd = open (LOG_FILE, O_CREAT | O_WRONLY | O_APPEND, 0600);
        if (fd < 0) {
            DEBUG_LOG ("work_loop:%s", "open err!");
            perror ("open");
        } else {
            FILE *pp = my_popen ("dmesg |busybox tail -n 50", "r");
            if (pp) {
                /* read to gbuf_cur */
                gbuf_cur.clear();
                char str[512];
                while (!feof (pp)) {
                    if (NULL != fgets (str, sizeof (str), pp)) {
                        char *p = strstr (str, " ");
                        if (p) {
                            gbuf_cur.push_back (p);
                        }
                    }
                }
                my_pclose (pp);
                /* write log file */
                for (int i = 0; i < gbuf_cur.size(); i++) {
                    int find = -1;
                    for (int j = 0; j < gbuf_pre.size(); j++) {
                        if (0 == gbuf_cur[i].compare (gbuf_pre[j])) {
                            find = i;
                        }
                    }
                    if (find == -1) {
                        /* add time */
                        time_t now;
                        struct tm *timenow;
                        char time_now[255];
                        char strtemp[255];
                        time (&now);
                        timenow = localtime (&now);
                        timenow->tm_hour += 8;
                        snprintf (time_now, sizeof (time_now), "%s", asctime (timenow));
                        string str = time_now;
                        size_t found;
                        found = str.find ("\n");
                        if (found != string::npos) {
                            str.erase (str.begin() + found, str.begin() + found + 1);
                        }
                        str.append (gbuf_cur[i]);
                        write (fd, str.c_str(), strlen (str.c_str()));
                    }
                }
                /* copy gbuf_cur to gbuf_pre */
                gbuf_pre.clear();
                for (int i = 0; i < gbuf_cur.size(); i++) {
                    gbuf_pre.push_back (gbuf_cur[i]);
                }
            }
            close (fd);
        }
        sleep (3);
    }

}

void print_usage (const char * prog) {
    system ("clear");
    printf ("Usage: %s [-d]\n", prog);
    puts ("  -d  --set debug mode\n");
}

int parse_opts (int argc, char * argv[]) {
    int ch;

    while ( (ch = getopt (argc, argv, "d")) != EOF) {
        switch (ch) {
        case 'd':
            do_fork = 0;
            break;
        case 'h':
        case '?':
            print_usage (argv[0]);
            return -1;
        default:
            break;
        }
    }

    return 0;
}

int main (int argc, char * argv[]) {

    int ret;
    char* cp;

    /* init log */
    INIT_LOG ("log");
    DEBUG_LOG ("int: %d, str: %s, double: %lf", 1, "sys_monitor", 1.5);

    /* parse opts */
    ret = parse_opts (argc, argv);
    if (ret < 0) {
        exit (0);
    }

    /* init_signals */
    init_signals();

    /* background ourself */
    if (do_fork) {
        /* Make ourselves a daemon. */
        if (daemon (0, 0) < 0) {
            syslog (LOG_CRIT, "daemon - %m");
            perror ("daemon");
            exit (1);
        }
    } else {
        /* Even if we don't daemonize, we still want to disown our parent
        ** process.
        */
        (void) setsid();
    }

    /* While loop util program finish */
    work_loop();

    /* exit */
    printf ("Program is finished!\n");
    exit (0);
}
