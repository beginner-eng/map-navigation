/**
 * @file    dijkstra.c
 * @brief   Dijkstra 最短路径算法实现
 *
 * 本文件是项目中最核心的算法模块，包含：
 *   - dijkstra()              完整的 Dijkstra 算法 + 结果输出
 *   - printPathRecursive()    递归打印完整路径
 *   - getShortestDistance()   仅获取最短距离值（内部复用算法逻辑）
 *
 * 算法复杂度：O(V²)，其中 V 为顶点数。
 * 使用邻接矩阵存储，适合稠密图。
 */

#include "dijkstra.h"

/* ================================================================
 * 内部辅助函数：找到未访问顶点中距离最小的索引
 *
 * @param  dist     距离数组
 * @param  visited  访问标记数组
 * @param  n        顶点总数
 * @return 最小距离顶点的索引，若全部已访问返回 -1
 * ================================================================ */
static int minDistance(int dist[], int visited[], int n)
{
    int minValue = INF;
    int minIndex = -1;

    for (int i = 0; i < n; i++) {
        if (!visited[i] && dist[i] < minValue) {
            minValue = dist[i];
            minIndex = i;
        }
    }

    return minIndex;
}

/* ================================================================
 * 递归打印路径
 *
 * 原理：
 *   prev[end] 存放着到达 end 的前一个顶点。
 *   递归向前追溯直到 prev[i] == -1（即起点），
 *   然后在回溯过程中依次输出顶点名称。
 *
 *   举例：prev = [-1, 0, 1, 2]，要打印 0→3 路径：
 *     printPathRecursive(prev, g, 3)
 *       → printPathRecursive(prev, g, 2)
 *           → printPathRecursive(prev, g, 1)
 *               → printPathRecursive(prev, g, 0)
 *                   → 输出 "A"
 *               → 输出 " -> B"
 *           → 输出 " -> C"
 *       → 输出 " -> D"
 *   最终输出：A -> B -> C -> D
 * ================================================================ */
void printPathRecursive(int prev[], Graph *g, int end)
{
    /* 递归基：回溯到起点（prev[end] == -1） */
    if (prev[end] == -1) {
        printf("%s", g->vertex[end]);
        return;
    }

    /* 先递归输出前面的路径 */
    printPathRecursive(prev, g, prev[end]);

    /* 再输出当前顶点 */
    printf(" -> %s", g->vertex[end]);
}

/* ================================================================
 * Dijkstra 最短路径算法（完整版：含输出）
 *
 * 算法流程：
 *   ┌─────────────────────────────────────────┐
 *   │ 1. 初始化 dist[start]=0, 其余为 INF     │
 *   │ 2. 初始化 prev[] = -1                  │
 *   │ 3. 重复 n 次：                          │
 *   │    a) 从未访问点中选 dist 最小的 u       │
 *   │    b) 标记 u 为已访问                   │
 *   │    c) 对 u 的每个邻接点 v 尝试松弛:      │
 *   │       若 dist[u]+w(u,v) < dist[v]:      │
 *   │         更新 dist[v] = dist[u]+w(u,v)   │
 *   │         更新 prev[v] = u                │
 *   │ 4. 输出 dist[end] 与完整路径            │
 *   └─────────────────────────────────────────┘
 * ================================================================ */
void dijkstra(Graph *g, int start, int end)
{
    /* ==================== 异常处理 ==================== */

    if (g == NULL) {
        fprintf(stderr, "[错误] 图指针为空。\n");
        return;
    }

    if (start < 0 || start >= g->vertexCount ||
        end   < 0 || end   >= g->vertexCount) {
        fprintf(stderr, "[错误] 地点编号无效（有效范围: 0 ~ %d）。\n",
                g->vertexCount - 1);
        return;
    }

    /* 起点与终点相同 */
    if (start == end) {
        printf("\n");
        printf("══════════════════════════════════════════════\n");
        printf("           最 短 路 径 查 询 结 果\n");
        printf("══════════════════════════════════════════════\n\n");
        printf("    【起点与终点相同】\n");
        printf("    起点/终点: %s\n", g->vertex[start]);
        printf("    最短距离: 0\n");
        printf("    路径: %s（您已在目的地）\n", g->vertex[start]);
        printf("\n══════════════════════════════════════════════\n\n");
        return;
    }

    /* ==================== 初始化 ==================== */

    int n = g->vertexCount;          /* 顶点总数               */
    int dist[MAX_VERTEX];            /* 最短距离数组           */
    int visited[MAX_VERTEX];         /* 访问标记数组           */
    int prev[MAX_VERTEX];            /* 前驱节点数组           */

    for (int i = 0; i < n; i++) {
        dist[i]    = INF;            /* 初始距离为无穷大       */
        visited[i] = 0;              /* 所有点未访问           */
        prev[i]    = -1;             /* 前驱初始化为 -1        */
    }

    dist[start] = 0;                 /* 起点到自身距离为 0     */

    /* ==================== 主循环 ==================== */

    for (int count = 0; count < n; count++) {

        /* ---- 第 1 步：选取未访问点中距离最小的 u ---- */
        int u = minDistance(dist, visited, n);
        if (u == -1) {
            /* 所有未访问点都不可达，可提前结束 */
            break;
        }

        /* ---- 第 2 步：标记 u 为已访问 ---- */
        visited[u] = 1;

        /* 若已选到终点，可提前终止（优化） */
        if (u == end) {
            break;
        }

        /* ---- 第 3 步：松弛 u 的所有邻接点 ---- */
        for (int v = 0; v < n; v++) {

            /* 条件：
             *   - v 未访问
             *   - u→v 有边 (edge[u][v] != INF && != 0)
             *   - 通过 u 到达 v 比当前 dist[v] 更短
             */
            if (!visited[v]
                && g->edge[u][v] != INF
                && g->edge[u][v] != 0
                && dist[u] != INF
                && dist[u] + g->edge[u][v] < dist[v]) {

                dist[v] = dist[u] + g->edge[u][v];   /* 更新最短距离 */
                prev[v] = u;                          /* 记录前驱节点 */
            }
        }
    }

    /* ==================== 输出结果 ==================== */

    printf("\n");
    printf("══════════════════════════════════════════════\n");
    printf("           最 短 路 径 查 询 结 果\n");
    printf("══════════════════════════════════════════════\n\n");

    printf("    起点: %s\n", g->vertex[start]);
    printf("    终点: %s\n", g->vertex[end]);

    if (dist[end] == INF) {
        /* 不可达 */
        printf("\n    【警告】从 \"%s\" 无法到达 \"%s\"。\n",
               g->vertex[start], g->vertex[end]);
        printf("    请检查两地之间是否有道路连通。\n");
    } else {
        printf("    最短距离: %d\n", dist[end]);
        printf("\n    最短路径:\n\n        ");
        printPathRecursive(prev, g, end);
        printf("\n");
    }

    printf("\n══════════════════════════════════════════════\n\n");
}

/* ================================================================
 * getShortestDistance - 仅获取最短距离值（不输出任何内容）
 *
 * 内部逻辑与 dijkstra() 一致，区别在于：
 *   - 不打印任何内容
 *   - 仅返回 dist[end] 或 INF
 * ================================================================ */
int getShortestDistance(Graph *g, int start, int end)
{
    if (g == NULL) {
        return INF;
    }

    if (start < 0 || start >= g->vertexCount ||
        end   < 0 || end   >= g->vertexCount) {
        return INF;
    }

    if (start == end) {
        return 0;
    }

    int n = g->vertexCount;
    int dist[MAX_VERTEX];
    int visited[MAX_VERTEX];

    for (int i = 0; i < n; i++) {
        dist[i]    = INF;
        visited[i] = 0;
    }

    dist[start] = 0;

    for (int count = 0; count < n; count++) {
        int u = minDistance(dist, visited, n);
        if (u == -1) break;
        visited[u] = 1;
        if (u == end) break;

        for (int v = 0; v < n; v++) {
            if (!visited[v]
                && g->edge[u][v] != INF
                && g->edge[u][v] != 0
                && dist[u] != INF
                && dist[u] + g->edge[u][v] < dist[v]) {
                dist[v] = dist[u] + g->edge[u][v];
            }
        }
    }

    return dist[end];
}
