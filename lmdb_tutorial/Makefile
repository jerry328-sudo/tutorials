# LMDB C语言示例程序 Makefile
# 用于编译和管理 LMDB 示例程序

# 编译器和编译选项
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LDFLAGS = -llmdb

# 目录设置
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# 源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/lmdb_example

# 数据库路径
DB_PATH = /home/wyp/workbench/Data/lmdb_tutorial/test_db

# 默认目标
.PHONY: all clean run install uninstall help

all: $(TARGET)

# 编译目标程序
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "编译完成: $@"

# 编译目标文件
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "编译: $< -> $@"

# 创建目录
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# 运行程序
run: $(TARGET)
	@echo "运行 LMDB 示例程序..."
	@echo "数据库路径: $(DB_PATH)"
	./$(TARGET)

# 清理编译文件
clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "清理完成"

# 清理数据库文件
clean-db:
	rm -rf $(DB_PATH)
	@echo "数据库文件已清理: $(DB_PATH)"

# 完全清理
clean-all: clean clean-db
	@echo "完全清理完成"

# 安装程序
install: $(TARGET)
	install -d /usr/local/bin
	install -m 755 $(TARGET) /usr/local/bin/lmdb_example
	@echo "安装完成: /usr/local/bin/lmdb_example"

# 卸载程序
uninstall:
	rm -f /usr/local/bin/lmdb_example
	@echo "卸载完成"

# 检查依赖
check-deps:
	@echo "检查 LMDB 依赖..."
	@echo "检查 LMDB 头文件..."
	@test -f /usr/include/lmdb.h || (echo "错误: 未找到 lmdb.h" && exit 1)
	@echo "检查 LMDB 库文件..."
	@ldconfig -p | grep lmdb || (echo "错误: 未找到 LMDB 库文件" && exit 1)
	@echo "LMDB 工具检查..."
	@which mdb_stat > /dev/null 2>&1 && echo "找到 mdb_stat 工具" || echo "警告: 未找到 mdb_stat 工具（不影响编译）"
	@echo "依赖检查完成"

# 调试编译
debug: CFLAGS += -DDEBUG -g3 -O0
debug: $(TARGET)
	@echo "调试版本编译完成"

# 性能优化编译
release: CFLAGS += -DNDEBUG -O3
release: $(TARGET)
	@echo "发布版本编译完成"

# 创建数据库目录
create-db-dir:
	mkdir -p $(DB_PATH)
	@echo "创建数据库目录: $(DB_PATH)"

# 查看数据库统计信息
db-stats:
	@if [ -d "$(DB_PATH)" ]; then \
		echo "数据库统计信息:"; \
		if which mdb_stat > /dev/null 2>&1; then \
			mdb_stat $(DB_PATH); \
		else \
			echo "警告: mdb_stat 工具未找到，请使用程序内的统计功能"; \
		fi; \
	else \
		echo "数据库不存在: $(DB_PATH)"; \
	fi

# 导出数据库
db-dump:
	@if [ -d "$(DB_PATH)" ]; then \
		echo "导出数据库内容:"; \
		mdb_dump $(DB_PATH); \
	else \
		echo "数据库不存在: $(DB_PATH)"; \
	fi

# 帮助信息
help:
	@echo "LMDB C语言示例程序 Makefile"
	@echo "使用方法:"
	@echo "  make              - 编译程序"
	@echo "  make run          - 编译并运行程序"
	@echo "  make clean        - 清理编译文件"
	@echo "  make clean-db     - 清理数据库文件"
	@echo "  make clean-all    - 完全清理"
	@echo "  make install      - 安装程序到系统"
	@echo "  make uninstall    - 从系统卸载程序"
	@echo "  make check-deps   - 检查依赖"
	@echo "  make debug        - 编译调试版本"
	@echo "  make release      - 编译发布版本"
	@echo "  make create-db-dir - 创建数据库目录"
	@echo "  make db-stats     - 查看数据库统计信息"
	@echo "  make db-dump      - 导出数据库内容"
	@echo "  make help         - 显示帮助信息"
	@echo ""
	@echo "文件位置:"
	@echo "  源代码: $(SRCDIR)/"
	@echo "  目标文件: $(OBJDIR)/"
	@echo "  可执行文件: $(BINDIR)/"
	@echo "  数据库路径: $(DB_PATH)"