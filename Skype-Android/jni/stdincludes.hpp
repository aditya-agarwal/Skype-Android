#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#if defined(__APPLE__) || defined(_SYMBIAN)
#define TEMP_FAILURE_RETRY

#elif __linux
#include <linux/sockios.h>
#include <linux/if.h>
#endif

#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#ifdef _SYMBIAN
#include <sys/select.h>
#include <es_sock.h>
#else
#include <syslog.h>
#include <poll.h>
#endif
#include <unistd.h>
#include <fcntl.h>
