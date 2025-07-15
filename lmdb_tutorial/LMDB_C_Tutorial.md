# LMDB C语言完整教程

## 目录
1. [简介](#简介)
2. [环境准备](#环境准备)
3. [基础概念](#基础概念)
4. [创建和打开数据库](#创建和打开数据库)
5. [写入数据](#写入数据)
6. [读取数据](#读取数据)
7. [遍历数据](#遍历数据)
8. [删除数据](#删除数据)
9. [事务管理](#事务管理)
10. [错误处理](#错误处理)
11. [性能优化](#性能优化)
12. [完整示例](#完整示例)
13. [编译和运行](#编译和运行)

## 简介

LMDB（Lightning Memory-Mapped Database）是一个快速、轻量级的键值数据库，使用内存映射技术实现高性能的数据存储和检索。本教程将详细介绍如何在C语言中使用LMDB。

### LMDB的特点
- **高性能**：使用内存映射技术，读取速度极快
- **ACID**：完全支持ACID事务
- **无锁读取**：多个进程可以同时读取数据
- **零拷贝**：读取数据时无需拷贝到用户空间
- **紧凑**：数据库文件大小接近数据本身大小

## 环境准备

### 1. 安装LMDB开发库

#### Ubuntu/Debian系统
```bash
sudo apt-get update
sudo apt-get install liblmdb-dev
```

#### CentOS/RHEL系统
```bash
sudo yum install lmdb-devel
```

#### 手动编译安装
```bash
git clone https://github.com/LMDB/lmdb.git
cd lmdb/libraries/liblmdb
make
sudo make install
```

### 2. 验证安装
```bash
# 查看LMDB版本
mdb_stat -V

# 检查头文件
ls /usr/include/lmdb.h
```

## 基础概念

### 1. 核心数据结构
- **MDB_env**: 数据库环境，代表一个数据库文件
- **MDB_dbi**: 数据库句柄，代表环境中的一个数据库
- **MDB_txn**: 事务，所有操作都必须在事务中进行
- **MDB_cursor**: 游标，用于遍历数据库
- **MDB_val**: 数据值，包含指向数据的指针和数据长度

### 2. 操作流程
1. 创建或打开环境 (`mdb_env_create`, `mdb_env_open`)
2. 开始事务 (`mdb_txn_begin`)
3. 打开数据库 (`mdb_dbi_open`)
4. 执行操作 (`mdb_put`, `mdb_get`, `mdb_del`)
5. 提交事务 (`mdb_txn_commit`)
6. 关闭环境 (`mdb_env_close`)

### 3. 数据结构定义
```c
typedef struct MDB_val {
    size_t mv_size;  /* 数据大小 */
    void *mv_data;   /* 数据指针 */
} MDB_val;
```

## 创建和打开数据库

### 1. 基本步骤
```c
#include <lmdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_database(const char *db_path) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    int rc;
    
    // 1. 创建环境
    rc = mdb_env_create(&env);
    if (rc) {
        fprintf(stderr, "mdb_env_create failed: %s\n", mdb_strerror(rc));
        return rc;
    }
    
    // 2. 设置环境参数
    rc = mdb_env_set_mapsize(env, 1048576 * 1024); // 1GB
    if (rc) {
        fprintf(stderr, "mdb_env_set_mapsize failed: %s\n", mdb_strerror(rc));
        mdb_env_close(env);
        return rc;
    }
    
    // 3. 打开环境
    rc = mdb_env_open(env, db_path, MDB_FIXEDMAP | MDB_NOSYNC, 0664);
    if (rc) {
        fprintf(stderr, "mdb_env_open failed: %s\n", mdb_strerror(rc));
        mdb_env_close(env);
        return rc;
    }
    
    // 4. 开始事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc) {
        fprintf(stderr, "mdb_txn_begin failed: %s\n", mdb_strerror(rc));
        mdb_env_close(env);
        return rc;
    }
    
    // 5. 打开数据库
    rc = mdb_dbi_open(txn, NULL, MDB_CREATE, &dbi);
    if (rc) {
        fprintf(stderr, "mdb_dbi_open failed: %s\n", mdb_strerror(rc));
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 6. 提交事务
    rc = mdb_txn_commit(txn);
    if (rc) {
        fprintf(stderr, "mdb_txn_commit failed: %s\n", mdb_strerror(rc));
        mdb_env_close(env);
        return rc;
    }
    
    // 7. 关闭数据库
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    
    printf("数据库创建成功: %s\n", db_path);
    return 0;
}
```

### 2. 环境标志说明
- **MDB_FIXEDMAP**: 使用固定内存映射地址
- **MDB_NOSYNC**: 不同步到磁盘（提高性能，但可能丢失数据）
- **MDB_RDONLY**: 只读模式
- **MDB_NOMETASYNC**: 不同步元数据

## 写入数据

### 1. 基本写入操作
```c
int put_data(const char *db_path, const char *key, const char *value) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key, mdb_value;
    int rc;
    
    // 打开环境
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 开始写事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 设置键值
    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void *)key;
    mdb_value.mv_size = strlen(value);
    mdb_value.mv_data = (void *)value;
    
    // 写入数据
    rc = mdb_put(txn, dbi, &mdb_key, &mdb_value, 0);
    if (rc) {
        fprintf(stderr, "mdb_put failed: %s\n", mdb_strerror(rc));
        mdb_txn_abort(txn);
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
        return rc;
    }
    
    // 提交事务
    rc = mdb_txn_commit(txn);
    if (rc) {
        fprintf(stderr, "mdb_txn_commit failed: %s\n", mdb_strerror(rc));
    } else {
        printf("写入成功: %s = %s\n", key, value);
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}
```

### 2. 批量写入操作
```c
int batch_put_data(const char *db_path, const char **keys, const char **values, int count) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key, mdb_value;
    int rc, i;
    
    // 打开环境
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 开始写事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 批量写入
    for (i = 0; i < count; i++) {
        mdb_key.mv_size = strlen(keys[i]);
        mdb_key.mv_data = (void *)keys[i];
        mdb_value.mv_size = strlen(values[i]);
        mdb_value.mv_data = (void *)values[i];
        
        rc = mdb_put(txn, dbi, &mdb_key, &mdb_value, 0);
        if (rc) {
            fprintf(stderr, "批量写入失败 [%d]: %s\n", i, mdb_strerror(rc));
            mdb_txn_abort(txn);
            mdb_dbi_close(env, dbi);
            mdb_env_close(env);
            return rc;
        }
    }
    
    // 提交事务
    rc = mdb_txn_commit(txn);
    if (rc) {
        fprintf(stderr, "批量提交失败: %s\n", mdb_strerror(rc));
    } else {
        printf("批量写入成功: %d 条记录\n", count);
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}
```

### 3. 写入标志说明
- **MDB_NOOVERWRITE**: 如果键已存在则失败
- **MDB_NODUPDATA**: 如果键值对已存在则失败
- **MDB_CURRENT**: 更新游标当前位置的数据
- **MDB_APPEND**: 追加数据（要求键按顺序）

## 读取数据

### 1. 基本读取操作
```c
int get_data(const char *db_path, const char *key, char *value, size_t value_size) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key, mdb_value;
    int rc;
    
    // 打开环境
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 开始读事务
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 设置键
    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void *)key;
    
    // 读取数据
    rc = mdb_get(txn, dbi, &mdb_key, &mdb_value);
    if (rc) {
        if (rc == MDB_NOTFOUND) {
            printf("键不存在: %s\n", key);
        } else {
            fprintf(stderr, "mdb_get failed: %s\n", mdb_strerror(rc));
        }
    } else {
        // 复制数据到用户缓冲区
        size_t copy_size = mdb_value.mv_size < value_size - 1 ? mdb_value.mv_size : value_size - 1;
        memcpy(value, mdb_value.mv_data, copy_size);
        value[copy_size] = '\0';
        printf("读取成功: %s = %s\n", key, value);
    }
    
    mdb_txn_abort(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}
```

### 2. 检查键是否存在
```c
int key_exists(const char *db_path, const char *key) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key, mdb_value;
    int rc;
    
    rc = mdb_env_create(&env);
    if (rc) return -1;
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    if (rc) {
        mdb_env_close(env);
        return -1;
    }
    
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    if (rc) {
        mdb_env_close(env);
        return -1;
    }
    
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return -1;
    }
    
    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void *)key;
    
    rc = mdb_get(txn, dbi, &mdb_key, &mdb_value);
    
    mdb_txn_abort(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    
    return (rc == 0) ? 1 : 0;  // 1表示存在，0表示不存在
}
```

## 遍历数据

### 1. 使用游标遍历所有数据
```c
int iterate_all_data(const char *db_path) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_cursor *cursor;
    MDB_val key, value;
    int rc, count = 0;
    
    // 打开环境
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 开始读事务
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 创建游标
    rc = mdb_cursor_open(txn, dbi, &cursor);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
        return rc;
    }
    
    // 遍历所有数据
    printf("遍历所有数据:\n");
    while ((rc = mdb_cursor_get(cursor, &key, &value, MDB_NEXT)) == 0) {
        printf("  [%d] %.*s = %.*s\n", 
               ++count,
               (int)key.mv_size, (char *)key.mv_data,
               (int)value.mv_size, (char *)value.mv_data);
    }
    
    if (rc != MDB_NOTFOUND) {
        fprintf(stderr, "游标遍历失败: %s\n", mdb_strerror(rc));
    } else {
        printf("遍历完成，总共 %d 条记录\n", count);
    }
    
    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return 0;
}
```

### 2. 前缀查询
```c
int query_by_prefix(const char *db_path, const char *prefix) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_cursor *cursor;
    MDB_val key, value;
    int rc, count = 0;
    size_t prefix_len = strlen(prefix);
    
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    rc = mdb_cursor_open(txn, dbi, &cursor);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
        return rc;
    }
    
    // 设置游标到前缀开始位置
    key.mv_size = prefix_len;
    key.mv_data = (void *)prefix;
    
    rc = mdb_cursor_get(cursor, &key, &value, MDB_SET_RANGE);
    if (rc == 0) {
        printf("前缀查询结果 '%s':\n", prefix);
        do {
            // 检查是否仍然匹配前缀
            if (key.mv_size >= prefix_len && 
                memcmp(key.mv_data, prefix, prefix_len) == 0) {
                printf("  [%d] %.*s = %.*s\n", 
                       ++count,
                       (int)key.mv_size, (char *)key.mv_data,
                       (int)value.mv_size, (char *)value.mv_data);
            } else {
                break;
            }
        } while ((rc = mdb_cursor_get(cursor, &key, &value, MDB_NEXT)) == 0);
        
        printf("找到 %d 条匹配记录\n", count);
    } else {
        printf("没有找到匹配前缀 '%s' 的记录\n", prefix);
    }
    
    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return 0;
}
```

### 3. 游标操作类型
- **MDB_FIRST**: 移动到第一条记录
- **MDB_LAST**: 移动到最后一条记录
- **MDB_NEXT**: 移动到下一条记录
- **MDB_PREV**: 移动到上一条记录
- **MDB_SET**: 移动到指定键
- **MDB_SET_RANGE**: 移动到大于等于指定键的第一条记录

## 删除数据

### 1. 删除单条记录
```c
int delete_data(const char *db_path, const char *key) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key;
    int rc;
    
    // 打开环境
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 开始写事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 设置键
    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void *)key;
    
    // 删除数据
    rc = mdb_del(txn, dbi, &mdb_key, NULL);
    if (rc) {
        if (rc == MDB_NOTFOUND) {
            printf("键不存在: %s\n", key);
        } else {
            fprintf(stderr, "mdb_del failed: %s\n", mdb_strerror(rc));
        }
        mdb_txn_abort(txn);
    } else {
        printf("删除成功: %s\n", key);
        rc = mdb_txn_commit(txn);
        if (rc) {
            fprintf(stderr, "删除提交失败: %s\n", mdb_strerror(rc));
        }
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}
```

### 2. 批量删除
```c
int batch_delete_by_prefix(const char *db_path, const char *prefix) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_cursor *cursor;
    MDB_val key, value;
    int rc, count = 0;
    size_t prefix_len = strlen(prefix);
    
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    rc = mdb_cursor_open(txn, dbi, &cursor);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
        return rc;
    }
    
    // 设置游标到前缀开始位置
    key.mv_size = prefix_len;
    key.mv_data = (void *)prefix;
    
    rc = mdb_cursor_get(cursor, &key, &value, MDB_SET_RANGE);
    if (rc == 0) {
        printf("批量删除前缀 '%s' 的记录:\n", prefix);
        do {
            // 检查是否仍然匹配前缀
            if (key.mv_size >= prefix_len && 
                memcmp(key.mv_data, prefix, prefix_len) == 0) {
                printf("  删除: %.*s\n", (int)key.mv_size, (char *)key.mv_data);
                rc = mdb_cursor_del(cursor, 0);
                if (rc) {
                    fprintf(stderr, "删除失败: %s\n", mdb_strerror(rc));
                    break;
                }
                count++;
            } else {
                break;
            }
        } while ((rc = mdb_cursor_get(cursor, &key, &value, MDB_NEXT)) == 0);
        
        printf("批量删除完成，共删除 %d 条记录\n", count);
    } else {
        printf("没有找到匹配前缀 '%s' 的记录\n", prefix);
    }
    
    mdb_cursor_close(cursor);
    
    if (count > 0) {
        rc = mdb_txn_commit(txn);
        if (rc) {
            fprintf(stderr, "批量删除提交失败: %s\n", mdb_strerror(rc));
        }
    } else {
        mdb_txn_abort(txn);
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return 0;
}
```

## 事务管理

### 1. 事务的基本概念
- **读事务**：使用 `MDB_RDONLY` 标志
- **写事务**：不使用 `MDB_RDONLY` 标志
- **嵌套事务**：在父事务中开始子事务
- **只读事务可以并发**：多个读事务可以同时进行
- **写事务是独占的**：同时只能有一个写事务

### 2. 事务示例
```c
int transaction_example(const char *db_path) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val key, value;
    int rc;
    
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 开始事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 在事务中执行多个操作
    key.mv_size = strlen("key1");
    key.mv_data = "key1";
    value.mv_size = strlen("value1");
    value.mv_data = "value1";
    
    rc = mdb_put(txn, dbi, &key, &value, 0);
    if (rc) {
        fprintf(stderr, "第一次写入失败: %s\n", mdb_strerror(rc));
        mdb_txn_abort(txn);
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
        return rc;
    }
    
    key.mv_size = strlen("key2");
    key.mv_data = "key2";
    value.mv_size = strlen("value2");
    value.mv_data = "value2";
    
    rc = mdb_put(txn, dbi, &key, &value, 0);
    if (rc) {
        fprintf(stderr, "第二次写入失败: %s\n", mdb_strerror(rc));
        mdb_txn_abort(txn);
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
        return rc;
    }
    
    // 提交事务
    rc = mdb_txn_commit(txn);
    if (rc) {
        fprintf(stderr, "事务提交失败: %s\n", mdb_strerror(rc));
    } else {
        printf("事务提交成功\n");
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}
```

## 错误处理

### 1. 常见错误码
- **MDB_SUCCESS (0)**: 操作成功
- **MDB_KEYEXIST**: 键已存在
- **MDB_NOTFOUND**: 键不存在
- **MDB_PAGE_NOTFOUND**: 页面不存在
- **MDB_CORRUPTED**: 数据库损坏
- **MDB_PANIC**: 严重错误
- **MDB_VERSION_MISMATCH**: 版本不匹配
- **MDB_INVALID**: 无效参数
- **MDB_MAP_FULL**: 映射空间已满
- **MDB_DBS_FULL**: 数据库已满
- **MDB_READERS_FULL**: 读者已满
- **MDB_TLS_FULL**: TLS键已满
- **MDB_TXN_FULL**: 事务已满
- **MDB_CURSOR_FULL**: 游标已满

### 2. 错误处理函数
```c
void handle_error(int rc, const char *operation) {
    if (rc != MDB_SUCCESS) {
        fprintf(stderr, "操作 '%s' 失败: %s (错误码: %d)\n", 
                operation, mdb_strerror(rc), rc);
    }
}

int safe_operation(const char *db_path) {
    MDB_env *env = NULL;
    MDB_dbi dbi = 0;
    MDB_txn *txn = NULL;
    int rc;
    
    // 创建环境
    rc = mdb_env_create(&env);
    if (rc) {
        handle_error(rc, "mdb_env_create");
        return rc;
    }
    
    // 打开环境
    rc = mdb_env_open(env, db_path, 0, 0664);
    if (rc) {
        handle_error(rc, "mdb_env_open");
        mdb_env_close(env);
        return rc;
    }
    
    // 开始事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc) {
        handle_error(rc, "mdb_txn_begin");
        mdb_env_close(env);
        return rc;
    }
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc) {
        handle_error(rc, "mdb_dbi_open");
        mdb_txn_abort(txn);
        mdb_env_close(env);
        return rc;
    }
    
    // 在这里执行具体操作...
    
    // 清理资源
    mdb_txn_commit(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    
    return MDB_SUCCESS;
}
```

## 性能优化

### 1. 环境设置优化
```c
int optimize_environment(MDB_env *env) {
    int rc;
    
    // 设置较大的映射大小
    rc = mdb_env_set_mapsize(env, 1024UL * 1024UL * 1024UL * 10UL); // 10GB
    if (rc) return rc;
    
    // 设置最大读者数量
    rc = mdb_env_set_maxreaders(env, 128);
    if (rc) return rc;
    
    // 设置最大数据库数量
    rc = mdb_env_set_maxdbs(env, 16);
    if (rc) return rc;
    
    return MDB_SUCCESS;
}
```

### 2. 写入性能优化
```c
int optimized_batch_write(const char *db_path, const char **keys, const char **values, int count) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val key, value;
    int rc, i;
    const int batch_size = 1000; // 批量大小
    
    rc = mdb_env_create(&env);
    if (rc) return rc;
    
    // 优化环境设置
    rc = optimize_environment(env);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 使用性能优化标志
    rc = mdb_env_open(env, db_path, MDB_NOSYNC | MDB_WRITEMAP, 0664);
    if (rc) {
        mdb_env_close(env);
        return rc;
    }
    
    // 分批写入
    for (i = 0; i < count; i += batch_size) {
        int batch_end = (i + batch_size < count) ? i + batch_size : count;
        
        rc = mdb_txn_begin(env, NULL, 0, &txn);
        if (rc) break;
        
        rc = mdb_dbi_open(txn, NULL, 0, &dbi);
        if (rc) {
            mdb_txn_abort(txn);
            break;
        }
        
        // 写入当前批次
        for (int j = i; j < batch_end; j++) {
            key.mv_size = strlen(keys[j]);
            key.mv_data = (void *)keys[j];
            value.mv_size = strlen(values[j]);
            value.mv_data = (void *)values[j];
            
            rc = mdb_put(txn, dbi, &key, &value, MDB_APPEND);
            if (rc) break;
        }
        
        if (rc) {
            mdb_txn_abort(txn);
            break;
        } else {
            rc = mdb_txn_commit(txn);
            if (rc) break;
        }
        
        printf("已写入 %d/%d 条记录\n", batch_end, count);
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}
```

### 3. 性能优化建议
- 使用 `MDB_NOSYNC` 标志提高写入性能（但可能丢失数据）
- 使用 `MDB_WRITEMAP` 标志使用写映射模式
- 批量操作减少事务开销
- 使用 `MDB_APPEND` 标志进行顺序写入
- 合理设置映射大小避免频繁扩展

## 完整示例

参考实际代码示例文件：`/home/wyp/workbench/Data/lmdb_tutorial/lmdb_example.c`

## 编译和运行

### 1. 编译命令
```bash
# 基本编译
gcc -o lmdb_example lmdb_example.c -llmdb

# 包含调试信息
gcc -g -O0 -o lmdb_example lmdb_example.c -llmdb

# 优化编译
gcc -O2 -o lmdb_example lmdb_example.c -llmdb
```

### 2. 运行示例
```bash
# 创建数据库目录
mkdir -p /home/wyp/workbench/Data/lmdb_tutorial/test_db

# 运行程序
./lmdb_example
```

### 3. Makefile 示例
```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -llmdb

SRCDIR = .
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/lmdb_example

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

.PHONY: all clean install
```

## 总结

本教程详细介绍了LMDB在C语言中的使用方法，包括：

1. **基础操作**：创建、打开、关闭数据库
2. **数据操作**：写入、读取、删除数据
3. **高级功能**：游标遍历、前缀查询、事务管理
4. **性能优化**：批量操作、环境优化
5. **错误处理**：完整的错误处理机制

通过学习本教程，你可以在C语言项目中高效地使用LMDB数据库，实现高性能的数据存储和检索功能。

## 参考资料

- [LMDB官方文档](http://www.lmdb.tech/doc/)
- [LMDB GitHub仓库](https://github.com/LMDB/lmdb)
- [LMDB API参考](http://www.lmdb.tech/doc/group__mdb.html)