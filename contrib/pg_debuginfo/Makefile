# contrib/pg_debuginfo/Makefile

MODULE_big = pg_debuginfo
OBJS = pg_debuginfo.o $(WIN32RES) 
PGFILEDESC = "pg_debuginfo - test data generator"

PG_CPPFLAGS = -I$(libpq_srcdir)
SHLIB_LINK_INTERNAL = $(libpq)

EXTENSION = pg_debuginfo
DATA = pg_debuginfo--1.0.sql

REGRESS = pg_debuginfo # sql、expected 文件夹下的测试，

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
SHLIB_PREREQS = submake-libpq
subdir = contrib/pg_debuginfo
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
