/*
 * LMDB C语言完整示例程序
 * 
 * 本程序演示了LMDB的完整使用方法，包括：
 * 1. 创建和打开数据库
 * 2. 写入数据（单条和批量）
 * 3. 读取数据
 * 4. 遍历数据
 * 5. 前缀查询
 * 6. 删除数据
 * 7. 错误处理
 * 
 * 编译方法：
 * gcc -o lmdb_example lmdb_example.c -llmdb
 * 
 * 运行方法：
 * ./lmdb_example
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <lmdb.h>

// 数据库路径（使用绝对路径）
#define DB_PATH "/home/wyp/workbench/Data/lmdb_tutorial/test_db"
#define MAX_VALUE_SIZE 1024
#define MAX_KEY_SIZE 256

// 错误处理宏
#define CHECK_ERROR(rc, operation) \
    do { \
        if (rc != MDB_SUCCESS) { \
            fprintf(stderr, "错误: %s 失败: %s (错误码: %d)\n", \
                    operation, mdb_strerror(rc), rc); \
            return rc; \
        } \
    } while (0)

// 函数声明
int create_database_directory(const char *path);
int create_and_open_database(const char *db_path);
int put_single_data(const char *db_path, const char *key, const char *value);
int put_batch_data(const char *db_path);
int get_data(const char *db_path, const char *key);
int iterate_all_data(const char *db_path);
int query_by_prefix(const char *db_path, const char *prefix);
int delete_data(const char *db_path, const char *key);
int delete_by_prefix(const char *db_path, const char *prefix);
int get_database_stats(const char *db_path);
void print_separator(const char *title);

// 创建数据库目录
int create_database_directory(const char *path) {
    struct stat st = {0};
    
    // 检查目录是否已存在
    if (stat(path, &st) == -1) {
        // 目录不存在，创建它
        if (mkdir(path, 0755) == -1) {
            fprintf(stderr, "无法创建目录 %s: %s\n", path, strerror(errno));
            return -1;
        }
        printf("创建数据库目录: %s\n", path);
    } else {
        printf("数据库目录已存在: %s\n", path);
    }
    
    return 0;
}

// 创建和打开数据库
int create_and_open_database(const char *db_path) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    int rc;
    
    printf("创建和打开数据库: %s\n", db_path);
    
    // 创建环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    // 设置环境参数
    rc = mdb_env_set_mapsize(env, 1048576 * 1024); // 1GB
    CHECK_ERROR(rc, "mdb_env_set_mapsize");
    
    rc = mdb_env_set_maxreaders(env, 126);
    CHECK_ERROR(rc, "mdb_env_set_maxreaders");
    
    rc = mdb_env_set_maxdbs(env, 4);
    CHECK_ERROR(rc, "mdb_env_set_maxdbs");
    
    // 打开环境
    rc = mdb_env_open(env, db_path, MDB_FIXEDMAP | MDB_NOSYNC, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, MDB_CREATE, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 提交事务
    rc = mdb_txn_commit(txn);
    CHECK_ERROR(rc, "mdb_txn_commit");
    
    // 关闭数据库
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    
    printf("数据库创建和打开成功\n");
    return MDB_SUCCESS;
}

// 写入单条数据
int put_single_data(const char *db_path, const char *key, const char *value) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key, mdb_value;
    int rc;
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始写事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 设置键值
    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void *)key;
    mdb_value.mv_size = strlen(value);
    mdb_value.mv_data = (void *)value;
    
    // 写入数据
    rc = mdb_put(txn, dbi, &mdb_key, &mdb_value, 0);
    CHECK_ERROR(rc, "mdb_put");
    
    // 提交事务
    rc = mdb_txn_commit(txn);
    CHECK_ERROR(rc, "mdb_txn_commit");
    
    printf("写入成功: %s = %s\n", key, value);
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return MDB_SUCCESS;
}

// 批量写入数据
int put_batch_data(const char *db_path) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key, mdb_value;
    int rc, i;
    
    // 示例数据
    const char *keys[] = {
        "battery/001/step_001", "battery/001/step_002", "battery/001/step_003",
        "battery/002/step_001", "battery/002/step_002", "battery/002/step_003",
        "sensor/temp_001", "sensor/temp_002", "sensor/pressure_001",
        "config/max_voltage", "config/min_voltage", "config/timeout",
        "log/2024-01-01", "log/2024-01-02", "log/2024-01-03"
    };
    
    const char *values[] = {
        "CC_Charge_Step_1", "CC_Discharge_Step_2", "Rest_Step_3",
        "CV_Charge_Step_1", "CC_Discharge_Step_2", "Rest_Step_3",
        "Temperature: 25.5°C", "Temperature: 26.2°C", "Pressure: 1013.25 hPa",
        "4.2V", "2.8V", "3600s",
        "System started", "Normal operation", "System shutdown"
    };
    
    int data_count = sizeof(keys) / sizeof(keys[0]);
    
    printf("批量写入 %d 条数据...\n", data_count);
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始写事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 批量写入
    for (i = 0; i < data_count; i++) {
        mdb_key.mv_size = strlen(keys[i]);
        mdb_key.mv_data = (void *)keys[i];
        mdb_value.mv_size = strlen(values[i]);
        mdb_value.mv_data = (void *)values[i];
        
        rc = mdb_put(txn, dbi, &mdb_key, &mdb_value, 0);
        if (rc != MDB_SUCCESS) {
            fprintf(stderr, "批量写入失败 [%d]: %s\n", i, mdb_strerror(rc));
            mdb_txn_abort(txn);
            mdb_dbi_close(env, dbi);
            mdb_env_close(env);
            return rc;
        }
        printf("  [%d] %s = %s\n", i + 1, keys[i], values[i]);
    }
    
    // 提交事务
    rc = mdb_txn_commit(txn);
    CHECK_ERROR(rc, "mdb_txn_commit");
    
    printf("批量写入完成: %d 条记录\n", data_count);
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return MDB_SUCCESS;
}

// 读取数据
int get_data(const char *db_path, const char *key) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key, mdb_value;
    int rc;
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始读事务
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 设置键
    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void *)key;
    
    // 读取数据
    rc = mdb_get(txn, dbi, &mdb_key, &mdb_value);
    if (rc == MDB_SUCCESS) {
        printf("读取成功: %s = %.*s\n", key, (int)mdb_value.mv_size, (char *)mdb_value.mv_data);
    } else if (rc == MDB_NOTFOUND) {
        printf("键不存在: %s\n", key);
    } else {
        fprintf(stderr, "读取失败: %s\n", mdb_strerror(rc));
    }
    
    mdb_txn_abort(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}

// 遍历所有数据
int iterate_all_data(const char *db_path) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_cursor *cursor;
    MDB_val key, value;
    int rc, count = 0;
    
    printf("遍历所有数据:\n");
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始读事务
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 创建游标
    rc = mdb_cursor_open(txn, dbi, &cursor);
    CHECK_ERROR(rc, "mdb_cursor_open");
    
    // 遍历所有数据
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
    return MDB_SUCCESS;
}

// 前缀查询
int query_by_prefix(const char *db_path, const char *prefix) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_cursor *cursor;
    MDB_val key, value;
    int rc, count = 0;
    size_t prefix_len = strlen(prefix);
    
    printf("前缀查询 '%s':\n", prefix);
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始读事务
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 创建游标
    rc = mdb_cursor_open(txn, dbi, &cursor);
    CHECK_ERROR(rc, "mdb_cursor_open");
    
    // 设置游标到前缀开始位置
    key.mv_size = prefix_len;
    key.mv_data = (void *)prefix;
    
    rc = mdb_cursor_get(cursor, &key, &value, MDB_SET_RANGE);
    if (rc == 0) {
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
    } else if (rc == MDB_NOTFOUND) {
        printf("没有找到匹配前缀 '%s' 的记录\n", prefix);
    } else {
        fprintf(stderr, "前缀查询失败: %s\n", mdb_strerror(rc));
    }
    
    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return MDB_SUCCESS;
}

// 删除数据
int delete_data(const char *db_path, const char *key) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_val mdb_key;
    int rc;
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始写事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 设置键
    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void *)key;
    
    // 删除数据
    rc = mdb_del(txn, dbi, &mdb_key, NULL);
    if (rc == MDB_SUCCESS) {
        printf("删除成功: %s\n", key);
        rc = mdb_txn_commit(txn);
        CHECK_ERROR(rc, "mdb_txn_commit");
    } else if (rc == MDB_NOTFOUND) {
        printf("键不存在: %s\n", key);
        mdb_txn_abort(txn);
    } else {
        fprintf(stderr, "删除失败: %s\n", mdb_strerror(rc));
        mdb_txn_abort(txn);
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return rc;
}

// 批量删除（根据前缀）
int delete_by_prefix(const char *db_path, const char *prefix) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_cursor *cursor;
    MDB_val key, value;
    int rc, count = 0;
    size_t prefix_len = strlen(prefix);
    
    printf("批量删除前缀 '%s' 的记录:\n", prefix);
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, 0, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 开始写事务
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 创建游标
    rc = mdb_cursor_open(txn, dbi, &cursor);
    CHECK_ERROR(rc, "mdb_cursor_open");
    
    // 设置游标到前缀开始位置
    key.mv_size = prefix_len;
    key.mv_data = (void *)prefix;
    
    rc = mdb_cursor_get(cursor, &key, &value, MDB_SET_RANGE);
    if (rc == 0) {
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
    } else if (rc == MDB_NOTFOUND) {
        printf("没有找到匹配前缀 '%s' 的记录\n", prefix);
    } else {
        fprintf(stderr, "批量删除失败: %s\n", mdb_strerror(rc));
    }
    
    mdb_cursor_close(cursor);
    
    if (count > 0) {
        rc = mdb_txn_commit(txn);
        CHECK_ERROR(rc, "mdb_txn_commit");
    } else {
        mdb_txn_abort(txn);
    }
    
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return MDB_SUCCESS;
}

// 获取数据库统计信息
int get_database_stats(const char *db_path) {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    MDB_stat stat;
    MDB_envinfo info;
    int rc;
    
    printf("数据库统计信息:\n");
    
    // 打开环境
    rc = mdb_env_create(&env);
    CHECK_ERROR(rc, "mdb_env_create");
    
    rc = mdb_env_open(env, db_path, MDB_RDONLY, 0664);
    CHECK_ERROR(rc, "mdb_env_open");
    
    // 获取环境信息
    rc = mdb_env_info(env, &info);
    CHECK_ERROR(rc, "mdb_env_info");
    
    // 开始读事务
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    CHECK_ERROR(rc, "mdb_txn_begin");
    
    // 打开数据库
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    CHECK_ERROR(rc, "mdb_dbi_open");
    
    // 获取统计信息
    rc = mdb_stat(txn, dbi, &stat);
    CHECK_ERROR(rc, "mdb_stat");
    
    // 打印统计信息
    printf("  数据库路径: %s\n", db_path);
    printf("  总记录数: %lu\n", stat.ms_entries);
    printf("  分支页数: %lu\n", stat.ms_branch_pages);
    printf("  叶子页数: %lu\n", stat.ms_leaf_pages);
    printf("  溢出页数: %lu\n", stat.ms_overflow_pages);
    printf("  页面大小: %u bytes\n", stat.ms_psize);
    printf("  数据库深度: %u\n", stat.ms_depth);
    printf("  映射大小: %lu bytes (%.2f MB)\n", info.me_mapsize, info.me_mapsize / (1024.0 * 1024.0));
    printf("  已用页数: %lu\n", info.me_last_pgno);
    printf("  最大读者数: %u\n", info.me_maxreaders);
    printf("  当前读者数: %u\n", info.me_numreaders);
    
    mdb_txn_abort(txn);
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return MDB_SUCCESS;
}

// 打印分隔线
void print_separator(const char *title) {
    printf("\n");
    printf("==================================================\n");
    printf("   %s\n", title);
    printf("==================================================\n");
}

// 主函数
int main() {
    int rc;
    
    printf("LMDB C语言完整示例程序\n");
    printf("========================\n");
    
    // 1. 创建数据库目录
    print_separator("1. 创建数据库目录");
    if (create_database_directory(DB_PATH) != 0) {
        return 1;
    }
    
    // 2. 创建和打开数据库
    print_separator("2. 创建和打开数据库");
    rc = create_and_open_database(DB_PATH);
    if (rc != MDB_SUCCESS) {
        return 1;
    }
    
    // 3. 写入单条数据
    print_separator("3. 写入单条数据");
    rc = put_single_data(DB_PATH, "test_key", "test_value");
    if (rc != MDB_SUCCESS) {
        return 1;
    }
    
    // 4. 批量写入数据
    print_separator("4. 批量写入数据");
    rc = put_batch_data(DB_PATH);
    if (rc != MDB_SUCCESS) {
        return 1;
    }
    
    // 5. 读取数据
    print_separator("5. 读取数据");
    get_data(DB_PATH, "test_key");
    get_data(DB_PATH, "battery/001/step_001");
    get_data(DB_PATH, "config/max_voltage");
    get_data(DB_PATH, "nonexistent_key");
    
    // 6. 遍历所有数据
    print_separator("6. 遍历所有数据");
    rc = iterate_all_data(DB_PATH);
    if (rc != MDB_SUCCESS) {
        return 1;
    }
    
    // 7. 前缀查询
    print_separator("7. 前缀查询");
    query_by_prefix(DB_PATH, "battery");
    query_by_prefix(DB_PATH, "config");
    query_by_prefix(DB_PATH, "sensor");
    query_by_prefix(DB_PATH, "nonexistent");
    
    // 8. 获取数据库统计信息
    print_separator("8. 数据库统计信息");
    rc = get_database_stats(DB_PATH);
    if (rc != MDB_SUCCESS) {
        return 1;
    }
    
    // 9. 删除数据
    print_separator("9. 删除数据");
    delete_data(DB_PATH, "test_key");
    delete_data(DB_PATH, "nonexistent_key");
    
    // 10. 批量删除
    print_separator("10. 批量删除");
    delete_by_prefix(DB_PATH, "log");
    
    // 11. 查看最终状态
    print_separator("11. 最终状态");
    rc = get_database_stats(DB_PATH);
    if (rc != MDB_SUCCESS) {
        return 1;
    }
    
    printf("\n示例程序执行完成!\n");
    printf("数据库文件位置: %s\n", DB_PATH);
    
    return 0;
}