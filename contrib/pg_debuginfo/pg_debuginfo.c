#include "postgres.h"
#include "fmgr.h"
#include "funcapi.h"
#include "commands/explain.h"
#include "utils/elog.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>

PG_MODULE_MAGIC;

static char local_ip_str[1024];

static inline text *text_internal(const char* str)
{
    int32_t size = strlen(str);

    text *t = (text *)palloc(VARHDRSZ + size);
    SET_VARSIZE(t, VARHDRSZ + size);
    int flag = 0;
    char ch;
    char *p = (char *)VARDATA(t);
    for(int i = 0; i < size; ++i){
        *p = str[i];
        p++;
    }

    return t;
}

static char* local_ip()
{
    uint32_t ip;
    int32_t fd, intrface;
    struct ifreq buf[32];
    struct ifconf ifc;
    ip = -1;
    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) {
            intrface = ifc.ifc_len / sizeof (struct ifreq); 
            while (intrface-- > 0) {
                if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface]))) {
                    ip = inet_addr( inet_ntoa( ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr) );
                    break;
                }
            }
        }
        close(fd);
    }

    unsigned char *bytes = (unsigned char *) &ip;
    snprintf(local_ip_str, sizeof(local_ip_str), "%02d.%02d.%02d.%02d", bytes[0], bytes[1], bytes[2], bytes[3]);
    return local_ip_str;
}

PG_FUNCTION_INFO_V1(pg_debuginfo_enable);
Datum pg_debuginfo_enable(PG_FUNCTION_ARGS)
{
    Assert(fcinfo->nargs == 0 || fcinfo->nargs == 1);

    text* t = NULL;
    int ret = 0;

    if (fcinfo->nargs == 0) {
        ret = pg_debuginfo_logfile_open(NULL);
    } else if (fcinfo->nargs == 1) {
        const char* logfile = PG_GETARG_CSTRING(0);
        ret = pg_debuginfo_logfile_open(logfile);
    }

    if (ret < 0) {
        t = text_internal("enable failed! logfile open failed.");
        PG_RETURN_TEXT_P(t);
    } else if (ret > 0) {
        t = text_internal("enable failed! pg debuginfo is already enabled.");
        PG_RETURN_TEXT_P(t);
    }else {
        t = text_internal("enable success!");
        PG_RETURN_TEXT_P(t);
    }

    t = text_internal("exception!");
    PG_RETURN_TEXT_P(t);
}

PG_FUNCTION_INFO_V1(pg_debuginfo_status);
Datum pg_debuginfo_status(PG_FUNCTION_ARGS)
{
    Assert(fcinfo->nargs == 0);

    text* t = NULL;
    int logfd = pg_debuginfo_logfile_fd();
    if (logfd < 0) {
        t = text_internal("pg debuginfo is disabled!");
        PG_RETURN_TEXT_P(t);
    } else {
        char  resolved_path[4096] = {0};
        realpath(pg_debuginfo_logfile_name(), resolved_path);

        char buff[5120] = {0};
        snprintf(buff, sizeof(buff), "pg debuginfo is enabled! host[%s] file[%s]", local_ip(), resolved_path);

        t = text_internal(buff);
        PG_RETURN_TEXT_P(t);
    }

    t = text_internal("exception!");
    PG_RETURN_TEXT_P(t);
}

PG_FUNCTION_INFO_V1(pg_debuginfo_output);
Datum
pg_debuginfo_output(PG_FUNCTION_ARGS)
{
    Assert(fcinfo->nargs == 0);

    int logfd = pg_debuginfo_logfile_fd();
    if (logfd < 0) {
        text* t = text_internal("debuginfo logfile is null!");
        PG_RETURN_DATUM( PointerGetDatum(t) );
    }

    const char* logfile = pg_debuginfo_logfile_name();
    if (strlen(logfile) == 0) {
        text* t = text_internal("debuginfo logfile is null!");
        PG_RETURN_DATUM( PointerGetDatum(t) );
    }

    FuncCallContext     *funcctx;
    /* stuff done only on the first call of the function */
    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext   oldcontext;

        /* create a function context for cross-call persistence */
        funcctx = SRF_FIRSTCALL_INIT();

        /* switch to memory context appropriate for multiple function calls */
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        FILE *fp_ = fopen(logfile, "r");
        /* total number of tuples to be returned */
        funcctx->user_fctx = (void*)fp_;

        MemoryContextSwitchTo(oldcontext);
    }

    /* stuff done on every call of the function */
    funcctx = SRF_PERCALL_SETUP();
    FILE* fp = (FILE*)funcctx->user_fctx;
    if (!fp) {
        text* t = text_internal("open debuginfo logfile failed!");
        PG_RETURN_DATUM(PointerGetDatum(t));
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if ( (read = getline(&line, &len, fp)) != -1 ) {
         if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        text *t = text_internal(line);

        if (line) free(line);
        SRF_RETURN_NEXT(funcctx, PointerGetDatum(t));
    } else {
        fclose(fp);
        if (line) free(line);
        SRF_RETURN_DONE(funcctx);
    }
}

PG_FUNCTION_INFO_V1(pg_debuginfo_disable);
Datum pg_debuginfo_disable(PG_FUNCTION_ARGS)
{
    Assert(fcinfo->nargs == 0);

    text* t = NULL;
    int ret = pg_debuginfo_logfile_close();
    if (ret < 0) {
        t = text_internal("disable failed! pg debuginfo not enabled.");
        PG_RETURN_TEXT_P(t);
    } else {
        t = text_internal("disable success!");
        PG_RETURN_TEXT_P(t);
    }

    t = text_internal("exception!");
    PG_RETURN_TEXT_P(t);
}
