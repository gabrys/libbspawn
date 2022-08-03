#include "libc/integral/normalize.inc"
#include "libc/sysv/errfuns.h"

#include <errno.h>

#include "libc/calls/ntspawn.c"
#include "libc/calls/fixenotdir.c"
#include "libc/str/strrchr16.c"
#include "libc/str/strlen16.c"
#include "libc/str/memrchr16.c"
#include "libc/calls/mkntpath.c"
#include "libc/calls/mkntcmdline.c"
#include "libc/calls/mkntenvblock.c"
#include "libc/calls/ntmagicpaths.c"
#include "libc/str/tprecode8to16.c"
#include "libc/intrin/createfilemapping.c"
#include "libc/intrin/createprocess.c"
#include "libc/intrin/getfileattributes.c"
#include "libc/intrin/winerr.greg.c"

int main(int argc, char **argv) {
	return 0;
}
