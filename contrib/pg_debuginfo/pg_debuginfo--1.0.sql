/* contrib/pg_debuginfo/pg_debuginfo--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_debuginfo" to load this file. \quit

CREATE FUNCTION pg_debuginfo_enable()
RETURNS text
AS 'MODULE_PATHNAME', 'pg_debuginfo_enable'
LANGUAGE C STRICT;

CREATE FUNCTION pg_debuginfo_enable(cstring)
RETURNS text
AS 'MODULE_PATHNAME', 'pg_debuginfo_enable'
LANGUAGE C STRICT;

CREATE FUNCTION pg_debuginfo_status()
RETURNS text
AS 'MODULE_PATHNAME', 'pg_debuginfo_status'
LANGUAGE C STRICT;

CREATE FUNCTION pg_debuginfo_output()
RETURNS text
AS 'MODULE_PATHNAME', 'pg_debuginfo_output'
LANGUAGE C STRICT;

CREATE FUNCTION pg_debuginfo_disable()
RETURNS text
AS 'MODULE_PATHNAME', 'pg_debuginfo_disable'
LANGUAGE C STRICT;

