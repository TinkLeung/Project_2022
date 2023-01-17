#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>
#include <libgen.h>

#include <linux/fs.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "common.h"
#include "nvme-print.h"
#include "nvme-ioctl.h"
#include "nvme-status.h"
#include "nvme-lightnvm.h"
#include "plugin.h"

//#include "argconfig.h"
//#include "fabrics.h"

static struct stat nvme_stat;
const char *devicename;

static const char nvme_version_string[] = "1.0";

#define CREATE_CMD
#include "nvme-builtin.h"
