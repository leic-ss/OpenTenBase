# PostgreSQL Plugin pg_debuginfo
在本工作中，我们开发了pg_debuginfo插件，用来支持对function/procedure的Trace功能，插件功能本身，已经实现的相对比较完善，但输出的trace报告中，trace内容和详细信息，还有进一步完善的空间，为了满足开发者需要完善和定制化报告的需求，本插件也支持往trace报告里增加所需trace内容，开发者只需要在Opentenbase代码任何地方，增加如下代码：
```
__PG_DEBUGINFO_OUTPUT__("%s %d", "xxx", xx);
```
然后重新编译安装即可

## 环境准备
### 软硬件环境
| Host | OS | Memory |
| -- | -- | -- |
| 192.168.56.106 | CentOS Linux release 7.9.2009 (Core) | 8G |
| 192.168.56.104 | CentOS Linux release 7.9.2009 (Core) | 8G |

### 编译
1. Opentenbase编译
参考项目文档中Opentenbase编译部分
[Opentenbase编译](https://github.com/leic-ss/OpenTenBase/blob/master/README_ZH.md)

2. pg_debuginfo插件编译
```
cd ${SOURCECODE_PATH}/contrib
make -sj
make install
```

### 部署
1. Opentenbase集群部署，集群规划如下：
| 节点名称 | IP | 数据目录 |
| -- | -- | -- |
| GTM master | 192.168.56.106 | /home/opentenbase/data/gtm |
| GTM slave  | 192.168.56.104 | /home/opentenbase/data/gtm |
| CN1 | 192.168.56.106 | /home/opentenbase/data/coord |
| CN2 | 192.168.56.104 | /home/opentenbase/data/coord |
| DN1 master | 192.168.56.106 | /home/opentenbase/data/dn001 |
| DN1 slave | 192.168.56.104 | /home/opentenbase/data/dn001 |
| DN2 master | 192.168.56.104 | /home/opentenbase/data/dn002 |
| DN2 slave | 192.168.56.106 | /home/opentenbase/data/dn002 |

操作如下：
```
PGXC deploy all
Deploying Postgres-XL components to all the target servers.
Prepare tarball to deploy ...
Deploying to the server 192.168.56.106.
Deploying to the server 192.168.56.104.
Deployment done.
```

```
PGXC init all
Initialize GTM master
The files belonging to this GTM system will be owned by user "opentenbase".
This user must also own the server process.
fixing permissions on existing directory /home/opentenbase/data/gtm ... ok
creating configuration files ... ok
creating xlog dir ... ok
... ...
... ...
```

2. 插件启用
连接Opentenbase
```bash
psql -h 192.168.56.106 -p 30004 -U opentenbase -d postgres
```
进入数据库，执行如下命令，启用插件：
```bash
CREATE EXTENSION pg_debuginfo;
```

## API:

| func | descryption |
| -- | -- |
| pg_debuginfo_enable() | 开启trace功能，保存到默认trace报告文件(/home/opentenbase/data/coord/pgdebuginfo.${pid}.${timestamp}.log |
| pg_debuginfo_enable(file) | 开启trace功能，保存到指定trace报告文件file |
| pg_debuginfo_status() | 查看trace状态，状态信息包含是否开启、trace报告文件所在主机、trace报告文件路径 |
| pg_debuginfo_output() | 查看trace报告，报告信息包含一些常规执行路径信息，另外开发者，也可在本插件基础上，添加定制或更详细的报告内容 |
| pg_debuginfo_disable() | 关闭trace功能，对应trace报告文件不会被删除，有需要可以直接登录主机查看 |


## 如何使用：
### 开启trace功能
执行pg_debuginfo_status()查看trace功能是否打开，如果没有打开，则执行pg_debuginfo_enable()打开trace功能
```
postgres=# select pg_debuginfo_status();
    pg_debuginfo_status
---------------------------
 pg debuginfo is disabled!
(1 row)

postgres=# select pg_debuginfo_enable();
 pg_debuginfo_enable
---------------------
 enable success!
(1 row)
```

### 查看trace功能状态
trace功能打开后，可以执行pg_debuginfo_status()查看trace功能开启状态、trace报告文件所在主机、trace报告文件路径
```
postgres=# select pg_debuginfo_status();
                                                   pg_debuginfo_status
--------------------------------------------------------------------------------------------------------------------------
 pg debuginfo is enabled! host[192.168.56.106] file[/home/opentenbase/data/coord/pgdebuginfo.22981.20241225013809.log]
(1 row)
```

### 查看trace报告
执行SQL命令
```
postgres=# create table foo(id bigint, str text) distribute by shard(id);
CREATE TABLE
postgres=# insert into foo values(1,'opentenbase');
INSERT 0 1
```

执行pg_debuginfo_output()查看trace报告
```
postgres=# select pg_debuginfo_output();
                                                                            pg_debuginfo_output

----------------------------------------------------------------------------------------------------------------------------------------------------------------
 ... ...
 ... ...
 2024-12-25 02:01:53.912157 [22981] --       ******** SQL START ********         [postgres.c:1260, exec_simple_query()]
 2024-12-25 02:01:53.912216 [22981] -- start query sql[create table foo(id bigint, str text) distribute by shard(id);]   [postgres.c:1261, exec_simple_query()]
 2024-12-25 02:01:53.912239 [22981] -- start parse sql[create table foo(id bigint, str text) distribute by shard(id);]   [postgres.c:836, pg_parse_query()]
 2024-12-25 02:01:53.912371 [22981] -- end parse sql[create table foo(id bigint, str text) distribute by shard(id);] type[254] length[1] [postgres.c:861, pg_par
se_query()]
 2024-12-25 02:01:53.912450 [22981] -- process commandTag[CREATE TABLE]  [postgres.c:1423, exec_simple_query()]
 2024-12-25 02:01:53.912462 [22981] -- start rewrite sql[create table foo(id bigint, str text) distribute by shard(id);] [postgres.c:884, pg_analyze_and_rewrite
()]
 2024-12-25 02:01:53.912527 [22981] -- parse analyze: {QUERY :commandType 5 :querySource 0 :canSetTag true :utilityStmt {CREATESTMT
 :relation {RANGEVAR :schemaname <> :relname foo :inh true :relpersistence p
 :alias <> :location 13 :intervalparent false :partitionvalue <> :pubname <>}
 :tableElts ({COLUMNDEF :colname id :typeName {TYPENAME :names ("pg_catalog"
 "int8") :typeOid 0 :setof false :pct_type false :typmods <> :typemod -1
 :arrayBounds <> :location 20} :inhcount 0 :is_local true :is_not_null false
 :is_from_type false :is_from_parent false :storage <> :raw_default <>
 :cooked_default <> :identity <> :collClause <> :collOid 0 :constraints <>
 :fdwoptions <> :location 17} {COLUMNDEF :colname str :typeName {TYPENAME
 :names ("text") :typeOid 0 :setof false :pct_type false :typmods <> :typemod
 -1 :arrayBounds <> :location 32} :inhcount 0 :is_local true :is_not_null false
 :is_from_type false :is_from_parent false :storage <> :raw_default <>
 :cooked_default <> :identity <> :collClause <> :collOid 0 :constraints <>
 :fdwoptions <> :location 28}) :inhRelations <> :partspec <> :partbound <>
 :ofTypename <> :constraints <> :options <> :oncommit 0 :tablespacename <>
 :if_not_exists false} :resultRelation 0 :hasAggs false :hasWindowFuncs false
 :hasTargetSRFs false :hasSubLinks false :hasDistinctOn false :hasRecursive
 false :hasModifyingCTE false :hasForUpdate false :hasRowSecurity false
 :hasCoordFuncs false :cteList <> :rtable <> :jointree <> :targetList <>
 :override 0 :onConflict <> :returningList <> :groupClause <> :groupingSets <>
 :havingQual <> :windowClause <> :distinctClause <> :sortClause <> :limitOffset
 <> :limitCount <> :rowMarks <> :setOperations <> :constraintDeps <>
 :stmt_location 0 :stmt_len 61}
         [analyze.c:164, parse_analyze()]
 --More--
 ... ...
 ... ...
```

在上述的报告文件中，以随机一行报告内容为例，对应的报告格式如下：
```
2024-12-25 02:01:53.912216 [22981] -- start query sql[create table foo(id bigint, str text) distribute by shard(id);]   [postgres.c:1261, exec_simple_query()]
--------------------------  ----      -------------------------------------------------------------------------------    ---------- ----  -------------------
       时间点                PID                                      报告内容                                               文件名   行号          函数名
```