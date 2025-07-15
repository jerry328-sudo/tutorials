# LMDB C语言教程和示例

本目录包含完整的 LMDB C语言教程和实际代码示例。

## 目录结构

```
/home/wyp/workbench/Data/lmdb_tutorial/
├── LMDB_C_Tutorial.md          # 详细的C语言教程（Markdown文档）
├── lmdb_example.c              # 完整的C语言示例代码
├── Makefile                    # 编译和管理工具
├── README.md                   # 本文件
├── lmdb_tutorial_new.ipynb     # Python版本教程
└── test_db/                    # 程序运行后生成的数据库文件
```

## 快速开始

### 1. 检查依赖

```bash
# 检查 LMDB 是否已安装
make check-deps

# 如果未安装，在 Ubuntu/Debian 上安装：
sudo apt-get install liblmdb-dev

# 在 CentOS/RHEL 上安装：
sudo yum install lmdb-devel
```

### 2. 编译程序

```bash
# 基本编译
make

# 或者查看所有选项
make help
```

### 3. 运行示例

```bash
# 运行示例程序
make run

# 或者直接运行
./bin/lmdb_example
```

## 详细说明

### 教程文档

- **LMDB_C_Tutorial.md**: 包含详细的C语言LMDB教程，涵盖：
  - 基础概念和环境准备
  - 创建和打开数据库
  - 写入、读取、删除数据
  - 遍历和查询数据
  - 事务管理和错误处理
  - 性能优化技巧

### 代码示例

- **lmdb_example.c**: 完整的C语言示例程序，演示：
  - 创建数据库目录
  - 创建和打开数据库
  - 单条和批量写入数据
  - 数据读取和存在性检查
  - 数据库遍历
  - 前缀查询
  - 数据删除
  - 数据库统计信息

### 编译工具

- **Makefile**: 提供了完整的编译和管理功能：
  - 编译程序（调试版本和发布版本）
  - 运行程序
  - 清理文件
  - 依赖检查
  - 数据库管理
  - 安装和卸载

## 使用说明

### 编译选项

```bash
# 标准编译
make

# 调试版本（包含调试信息）
make debug

# 发布版本（优化编译）
make release

# 检查依赖
make check-deps
```

### 运行选项

```bash
# 编译并运行
make run

# 只运行（需要先编译）
./bin/lmdb_example
```

### 数据库管理

```bash
# 创建数据库目录
make create-db-dir

# 查看数据库统计信息
make db-stats

# 导出数据库内容
make db-dump

# 清理数据库文件
make clean-db
```

### 清理选项

```bash
# 清理编译文件
make clean

# 清理数据库文件
make clean-db

# 完全清理
make clean-all
```

## 程序输出示例

运行程序后，你将看到类似以下的输出：

```
LMDB C语言完整示例程序
========================

==================================================
   1. 创建数据库目录
==================================================
创建数据库目录: /home/wyp/workbench/Data/lmdb_tutorial/test_db

==================================================
   2. 创建和打开数据库
==================================================
数据库创建和打开成功

==================================================
   3. 写入单条数据
==================================================
写入成功: test_key = test_value

==================================================
   4. 批量写入数据
==================================================
批量写入 15 条数据...
  [1] battery/001/step_001 = CC_Charge_Step_1
  [2] battery/001/step_002 = CC_Discharge_Step_2
  ...
批量写入完成: 15 条记录

==================================================
   5. 读取数据
==================================================
读取成功: test_key = test_value
读取成功: battery/001/step_001 = CC_Charge_Step_1
读取成功: config/max_voltage = 4.2V
键不存在: nonexistent_key

==================================================
   6. 遍历所有数据
==================================================
遍历所有数据:
  [1] battery/001/step_001 = CC_Charge_Step_1
  [2] battery/001/step_002 = CC_Discharge_Step_2
  ...
遍历完成，总共 16 条记录

==================================================
   7. 前缀查询
==================================================
前缀查询 'battery':
  [1] battery/001/step_001 = CC_Charge_Step_1
  [2] battery/001/step_002 = CC_Discharge_Step_2
  ...
找到 6 条匹配记录

==================================================
   8. 数据库统计信息
==================================================
数据库统计信息:
  数据库路径: /home/wyp/workbench/Data/lmdb_tutorial/test_db
  总记录数: 16
  分支页数: 0
  叶子页数: 1
  溢出页数: 0
  页面大小: 4096 bytes
  数据库深度: 1
  映射大小: 1073741824 bytes (1024.00 MB)
  ...

示例程序执行完成!
数据库文件位置: /home/wyp/workbench/Data/lmdb_tutorial/test_db
```

## 文件路径说明

所有路径都使用绝对路径：

- **数据库路径**: `/home/wyp/workbench/Data/lmdb_tutorial/test_db`
- **源代码路径**: `/home/wyp/workbench/Data/lmdb_tutorial/`
- **可执行文件**: `/home/wyp/workbench/Data/lmdb_tutorial/bin/lmdb_example`

## 故障排除

### 常见问题

1. **编译错误 "lmdb.h: No such file or directory"**
   ```bash
   # 安装 LMDB 开发库
   sudo apt-get install liblmdb-dev
   ```

2. **链接错误 "undefined reference to mdb_*"**
   ```bash
   # 确保链接了 LMDB 库
   gcc -o lmdb_example lmdb_example.c -llmdb
   ```

3. **运行时错误 "Permission denied"**
   ```bash
   # 确保有创建数据库目录的权限
   chmod 755 /home/wyp/workbench/Data/lmdb_tutorial/
   ```

4. **数据库路径不存在**
   ```bash
   # 创建数据库目录
   make create-db-dir
   ```

### 调试

如果遇到问题，可以：

1. 使用调试版本编译：`make debug`
2. 使用 gdb 调试：`gdb ./bin/lmdb_example`
3. 检查系统日志：`dmesg | tail`
4. 验证 LMDB 安装：`make check-deps`

## 学习建议

1. **先阅读教程文档**：`LMDB_C_Tutorial.md` 包含详细的概念说明
2. **运行示例程序**：理解实际的代码执行过程
3. **修改代码实验**：尝试修改示例代码，观察结果
4. **查看 LMDB 文档**：访问官方文档了解更多高级功能

## 扩展学习

- [LMDB 官方文档](http://www.lmdb.tech/doc/)
- [LMDB GitHub 仓库](https://github.com/LMDB/lmdb)
- [LMDB API 参考](http://www.lmdb.tech/doc/group__mdb.html)

## 版权信息

本教程和示例代码仅供学习使用。LMDB 本身遵循 OpenLDAP Public License。