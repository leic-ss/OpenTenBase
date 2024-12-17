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

PG_MODULE_MAGIC;

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

PG_FUNCTION_INFO_V1(pg_debug_status);
Datum pg_debug_status(PG_FUNCTION_ARGS)
{
    Assert(fcinfo->nargs == 0);
    int32_t status = ExplainDebug_hook ? 1 : 0;

    PG_RETURN_INT32(status);
}

PG_FUNCTION_INFO_V1(pg_debuginfo_enable);
Datum pg_debuginfo_enable(PG_FUNCTION_ARGS)
{
    Assert(fcinfo->nargs == 0);

    text* t = NULL;
    int ret = pg_debuginfo_file_open(NULL);
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
    int fd = pg_debuginfo_logfile_fd();
    if (fd < 0) {
        t = text_internal("pg debuginfo is disabled!");
        PG_RETURN_TEXT_P(t);
    } else {
        t = text_internal("pg debuginfo is enabled!");
        PG_RETURN_TEXT_P(t);
    }

    t = text_internal("exception!");
    PG_RETURN_TEXT_P(t);
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
