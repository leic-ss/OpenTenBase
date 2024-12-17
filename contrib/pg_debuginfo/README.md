# PostgreSQL pg_debuginfo

### 安装方法：
```bash
make
make install
```
然后进入数据库、启用插件即可：
```bash
CREATE EXTENSION pg_debuginfo;
```

### API:

| func | descryption |
| -- | -- |
| rand_int() | random 32 bits integer |
| rand_int(a, b) | random 32 bits integer between [a, b] |
| rand_text() | random text (size in [1, 32]) |

### 如何使用：

表结构为`t(id int, txt text)`，使用`insert into t select rows_int(5000, 1, 100), rows_text(5000, 20, 30);`可以批量插入5000条随机数据，id字段为值在[1, 100]之间的随机整数，txt字段为长度在[20, 30]的随机字符串。
