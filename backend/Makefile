# ============================================================
#  Makefile - 地图导航系统 C 后端
# ============================================================
#  使用方法:
#    make              —— 编译后端程序
#    make clean        —— 删除编译产物
#    make rebuild      —— clean + 重新编译
#    make test         —— 编译并运行一次测试查询
#    make help         —— 显示帮助
# ============================================================

# ---- 编译器与选项 ----
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -pedantic
OPT      = -O2
DEBUG    = -g

# ---- 目录定义 ----
IDIR     = include
SDIR     = src
ODIR     = build

# ---- 可执行文件名 ----
TARGET   = map_navigation_backend

# ---- 头文件搜索路径 ----
INCLUDES = -I$(IDIR)

# ---- 源文件 ----
SRCS     = $(SDIR)/main.c $(SDIR)/graph.c $(SDIR)/dijkstra.c $(SDIR)/json_output.c

# ---- 目标文件 ----
OBJS     = $(SRCS:$(SDIR)/%.c=$(ODIR)/%.o)

# ---- 默认目标 ----
.PHONY: all
all: $(TARGET)

# ---- 链接 ----
$(TARGET): $(OBJS)
	@echo "============================================="
	@echo "  链接生成可执行文件: $(TARGET)"
	@echo "============================================="
	$(CC) $(CFLAGS) $(OPT) $(INCLUDES) -o $@ $^
	@echo "[成功] 编译完成！"

# ---- 编译每个源文件 ----
$(ODIR)/%.o: $(SDIR)/%.c
	@mkdir -p $(ODIR)
	@echo "  编译 $< ..."
	$(CC) $(CFLAGS) $(OPT) $(DEBUG) $(INCLUDES) -c -o $@ $<

# ---- 清理 ----
.PHONY: clean
clean:
	@echo "  清理编译产物..."
	rm -rf $(ODIR) $(TARGET) $(TARGET).exe
	@echo "[完成] 已清理。"

# ---- 重新编译 ----
.PHONY: rebuild
rebuild: clean all

# ---- 测试（查询 火车站(0) → 机场(8)） ----
.PHONY: test
test: $(TARGET)
	@echo "  运行测试: 火车站(0) → 机场(8)..."
	./$(TARGET) 0 8 ../data/map.txt ../data/result.json

# ---- 帮助 ----
.PHONY: help
help:
	@echo "============================================="
	@echo "  地图导航系统 - C 后端 Makefile"
	@echo "============================================="
	@echo "  make           编译项目"
	@echo "  make clean     清理编译产物"
	@echo "  make rebuild   重新编译"
	@echo "  make test      编译并运行测试"
	@echo "  make help      显示此帮助"
	@echo ""
	@echo "  手动运行:"
	@echo "    ./$(TARGET) <起点> <终点> [地图文件] [输出JSON]"
	@echo "============================================="
