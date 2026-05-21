/**
 * @file    graph.c
 * @brief   图模块实现 - 基于邻接矩阵的图基本操作
 *
 * 本文件实现了：
 *   - 图的初始化          initGraph()
 *   - 顶点的添加与查找     addVertex() / findVertex()
 *   - 边的添加与修改       addEdge() / modifyEdge()
 *   - 地图显示与矩阵打印   showGraph() / printMatrix()
 *   - 文件读写             loadFromFile() / saveToFile()
 */

#include "graph.h"

/* ================================================================
 * 初始化图
 * ================================================================ */
void initGraph(Graph *g)
{
    /* 参数校验：指针不能为空 */
    if (g == NULL) {
        fprintf(stderr, "[错误] initGraph: 传入空指针。\n");
        return;
    }

    g->vertexCount = 0;  /* 顶点数归零 */

    /* 遍历邻接矩阵，将所有边初始化为 INF（无边） */
    for (int i = 0; i < MAX_VERTEX; i++) {
        for (int j = 0; j < MAX_VERTEX; j++) {
            if (i == j) {
                g->edge[i][j] = 0;      /* 到自身的距离为 0 */
            } else {
                g->edge[i][j] = INF;    /* 其他默认不可达    */
            }
        }
    }

    /* 初始化顶点名称数组为空字符串 */
    for (int i = 0; i < MAX_VERTEX; i++) {
        g->vertex[i][0] = '\0';
    }
}

/* ================================================================
 * 添加顶点
 * ================================================================ */
int addVertex(Graph *g, const char *name)
{
    /* ---- 参数校验 ---- */
    if (g == NULL || name == NULL) {
        fprintf(stderr, "[错误] addVertex: 参数为空。\n");
        return -1;
    }

    /* 名称为空 */
    if (strlen(name) == 0) {
        fprintf(stderr, "[错误] 地点名称不能为空。\n");
        return -1;
    }

    /* 去除首尾空白后再次判断 */
    char trimmed[MAX_NAME];
    strncpy(trimmed, name, MAX_NAME - 1);
    trimmed[MAX_NAME - 1] = '\0';

    /* 已满 */
    if (g->vertexCount >= MAX_VERTEX) {
        fprintf(stderr, "[错误] 已达到最大地点数量 %d，无法继续添加。\n", MAX_VERTEX);
        return -1;
    }

    /* 查重：名称不能重复 */
    if (findVertex(g, name) != -1) {
        fprintf(stderr, "[错误] 地点 \"%s\" 已存在，请不要重复添加。\n", name);
        return -1;
    }

    /* ---- 写入新顶点 ---- */
    int idx = g->vertexCount;
    strncpy(g->vertex[idx], name, MAX_NAME - 1);
    g->vertex[idx][MAX_NAME - 1] = '\0';  /* 确保字符串以 '\0' 结尾 */
    g->vertexCount++;

    printf("[成功] 已添加地点: \"%s\" （编号: %d）\n", name, idx);
    return idx;
}

/* ================================================================
 * 查找顶点索引
 * ================================================================ */
int findVertex(Graph *g, const char *name)
{
    if (g == NULL || name == NULL) {
        return -1;
    }

    for (int i = 0; i < g->vertexCount; i++) {
        if (strcmp(g->vertex[i], name) == 0) {
            return i;  /* 找到，返回索引 */
        }
    }

    return -1;  /* 未找到 */
}

/* ================================================================
 * 添加边（双向）
 * ================================================================ */
int addEdge(Graph *g, int start, int end, int weight)
{
    /* ---- 参数校验 ---- */
    if (g == NULL) {
        fprintf(stderr, "[错误] addEdge: 图指针为空。\n");
        return -1;
    }

    /* 索引越界检查 */
    if (start < 0 || start >= g->vertexCount ||
        end   < 0 || end   >= g->vertexCount) {
        fprintf(stderr, "[错误] 地点编号必须在 0 ~ %d 之间。\n", g->vertexCount - 1);
        return -1;
    }

    /* 自环不允许 */
    if (start == end) {
        fprintf(stderr, "[错误] 不允许起点和终点相同（自环）。\n");
        return -1;
    }

    /* 权值必须为正数 */
    if (weight <= 0) {
        fprintf(stderr, "[错误] 道路距离必须大于 0。\n");
        return -1;
    }

    /* 如果已经有边，提示是否需要修改，但仍然允许覆盖 */
    if (g->edge[start][end] != INF && g->edge[start][end] != 0) {
        printf("[提示] \"%s\" → \"%s\" 已存在道路（距离 %d），将覆盖为 %d。\n",
               g->vertex[start], g->vertex[end],
               g->edge[start][end], weight);
    }

    /* 设置双向边 */
    g->edge[start][end] = weight;
    g->edge[end][start] = weight;  /* 无向图，反向同权值 */

    printf("[成功] 已添加道路: \"%s\" <-> \"%s\"，距离: %d。\n",
           g->vertex[start], g->vertex[end], weight);
    return 0;
}

/* ================================================================
 * 修改边权值
 * ================================================================ */
int modifyEdge(Graph *g, int start, int end, int weight)
{
    if (g == NULL) {
        fprintf(stderr, "[错误] modifyEdge: 图指针为空。\n");
        return -1;
    }

    if (start < 0 || start >= g->vertexCount ||
        end   < 0 || end   >= g->vertexCount) {
        fprintf(stderr, "[错误] 地点编号必须在 0 ~ %d 之间。\n", g->vertexCount - 1);
        return -1;
    }

    if (start == end) {
        fprintf(stderr, "[错误] 不允许修改自环边。\n");
        return -1;
    }

    if (weight <= 0) {
        fprintf(stderr, "[错误] 道路距离必须大于 0。\n");
        return -1;
    }

    /* 检查此边是否存在 */
    if (g->edge[start][end] == INF) {
        fprintf(stderr, "[错误] \"%s\" → \"%s\" 之间不存在道路，无法修改。请先添加道路。\n",
                g->vertex[start], g->vertex[end]);
        return -1;
    }

    int oldWeight = g->edge[start][end];
    g->edge[start][end] = weight;
    g->edge[end][start] = weight;  /* 双向同步修改 */

    printf("[成功] 已修改道路: \"%s\" <-> \"%s\": %d → %d。\n",
           g->vertex[start], g->vertex[end], oldWeight, weight);
    return 0;
}

/* ================================================================
 * 显示地图信息
 * ================================================================ */
void showGraph(Graph *g)
{
    if (g == NULL || g->vertexCount == 0) {
        printf("\n[提示] 地图为空，请先添加地点和道路。\n\n");
        return;
    }

    printf("\n");
    printf("══════════════════════════════════════════════\n");
    printf("              地图导航系统 - 当前地图\n");
    printf("══════════════════════════════════════════════\n");
    printf("\n【地点列表】（共 %d 个）\n\n", g->vertexCount);

    /* 打印所有地点编号及名称 */
    for (int i = 0; i < g->vertexCount; i++) {
        printf("    [%2d]  %s\n", i, g->vertex[i]);
    }

    printf("\n【道路列表】\n\n");

    /* 遍历邻接矩阵上半三角，避免重复输出双向边 */
    int roadCount = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        for (int j = i + 1; j < g->vertexCount; j++) {
            if (g->edge[i][j] != INF && g->edge[i][j] != 0) {
                printf("    %s  <──%3d──>  %s\n",
                       g->vertex[i], g->edge[i][j], g->vertex[j]);
                roadCount++;
            }
        }
    }

    if (roadCount == 0) {
        printf("    （暂无道路连接）\n");
    } else {
        printf("\n    共 %d 条道路。\n", roadCount);
    }

    printf("\n══════════════════════════════════════════════\n\n");
}

/* ================================================================
 * 打印邻接矩阵
 * ================================================================ */
void printMatrix(Graph *g)
{
    if (g == NULL || g->vertexCount == 0) {
        printf("\n[提示] 地图为空，无法打印邻接矩阵。\n\n");
        return;
    }

    printf("\n");
    printf("══════════════════════════════════════════════\n");
    printf("                邻 接 矩 阵\n");
    printf("══════════════════════════════════════════════\n\n");

    /* ---------- 打印列标题 ---------- */
    printf("            ");  /* 留出左侧行标题空白 */
    for (int j = 0; j < g->vertexCount; j++) {
        printf("%-8s", g->vertex[j]);
    }
    printf("\n");

    /* ---------- 打印分隔线 ---------- */
    printf("    ────────");
    for (int j = 0; j < g->vertexCount; j++) {
        printf("────────");
    }
    printf("\n");

    /* ---------- 逐行打印矩阵 ---------- */
    for (int i = 0; i < g->vertexCount; i++) {
        printf("%-10s  │", g->vertex[i]);  /* 行标题 */
        for (int j = 0; j < g->vertexCount; j++) {
            if (g->edge[i][j] == INF) {
                printf("  INF   ");       /* 无边 */
            } else if (g->edge[i][j] == 0) {
                printf("   0    ");       /* 对角线 */
            } else {
                printf(" %-6d ", g->edge[i][j]);
            }
        }
        printf("\n");
    }

    printf("\n    （备注: INF = 不可达, 0 = 自身）\n");
    printf("\n══════════════════════════════════════════════\n\n");
}

/* ================================================================
 * 从文件加载地图数据
 *
 * 文件格式（每行一条记录）：
 *   V 地点名           —— 添加顶点
 *   E 起点索引 终点索引 权值  —— 添加边
 *   以 # 开头的行为注释
 * ================================================================ */
int loadFromFile(Graph *g, const char *filename)
{
    if (g == NULL || filename == NULL) {
        return -1;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "[错误] 无法打开文件 \"%s\"。\n", filename);
        return -1;
    }

    char line[256];
    int  lineNo = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        lineNo++;

        /* 去除行尾换行符 */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        /* 跳过空行和注释行 */
        if (len == 0 || line[0] == '#') {
            continue;
        }

        char type;
        if (sscanf(line, " %c", &type) != 1) {
            continue;
        }

        if (type == 'V' || type == 'v') {
            /* 顶点行: V 名称 */
            char name[MAX_NAME];
            if (sscanf(line, " %*c %63s", name) == 1) {
                addVertex(g, name);
            } else {
                fprintf(stderr, "[警告] 第 %d 行格式错误，已跳过。\n", lineNo);
            }
        }
        else if (type == 'E' || type == 'e') {
            /* 边行: E 起点索引 终点索引 权值 */
            int s, e, w;
            if (sscanf(line, " %*c %d %d %d", &s, &e, &w) == 3) {
                addEdge(g, s, e, w);
            } else {
                fprintf(stderr, "[警告] 第 %d 行格式错误，已跳过。\n", lineNo);
            }
        }
        else {
            fprintf(stderr, "[警告] 第 %d 行未知类型 '%c'，已跳过。\n", lineNo, type);
        }
    }

    fclose(fp);
    printf("[成功] 已从文件 \"%s\" 加载地图数据。\n", filename);
    return 0;
}

/* ================================================================
 * 保存地图到文件
 * ================================================================ */
int saveToFile(Graph *g, const char *filename)
{
    if (g == NULL || filename == NULL) {
        return -1;
    }

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "[错误] 无法创建文件 \"%s\"。\n", filename);
        return -1;
    }

    fprintf(fp, "# ===========================================\n");
    fprintf(fp, "# 地图数据文件 - 由地图导航系统自动生成\n");
    fprintf(fp, "# 格式说明:\n");
    fprintf(fp, "#   V 地点名称        —— 添加顶点\n");
    fprintf(fp, "#   E 起点 终点 距离  —— 添加边（双向）\n");
    fprintf(fp, "#   以 # 开头的行为注释\n");
    fprintf(fp, "# ===========================================\n\n");

    /* 写入所有顶点 */
    for (int i = 0; i < g->vertexCount; i++) {
        fprintf(fp, "V %s\n", g->vertex[i]);
    }

    fprintf(fp, "\n");

    /* 写入所有边（仅存储上半三角，避免重复） */
    for (int i = 0; i < g->vertexCount; i++) {
        for (int j = i + 1; j < g->vertexCount; j++) {
            if (g->edge[i][j] != INF && g->edge[i][j] != 0) {
                fprintf(fp, "E %d %d %d\n", i, j, g->edge[i][j]);
            }
        }
    }

    fclose(fp);
    printf("[成功] 地图数据已保存到 \"%s\"。\n", filename);
    return 0;
}
