/**
 * @file    main.c
 * @brief   后端程序入口 - 地图导航系统 C 后端
 *
 * 使用方式：
 *   ./map_navigation_backend <起点索引> <终点索引> [地图文件] [输出文件]
 *
 * 示例：
 *   ./map_navigation_backend 0 8
 *   ./map_navigation_backend 0 8 data/map.txt data/result.json
 *
 * 工作流程：
 *   1. 解析命令行参数（起点索引、终点索引）
 *   2. 初始化图结构
 *   3. 从 map.txt 加载地图数据
 *   4. 执行 Dijkstra 最短路径算法
 *   5. 将结果输出为 result.json
 *   6. 前端 JS 读取 result.json 并更新可视化
 */

#include "graph.h"
#include "dijkstra.h"
#include "json_output.h"

/**
 * @brief  打印使用说明
 */
static void printUsage(const char *progName)
{
    printf("地图导航系统 - C 后端 (v2.0)\n");
    printf("基于 Dijkstra 最短路径算法\n\n");
    printf("用法: %s <起点索引> <终点索引> [地图文件] [输出JSON文件]\n\n", progName);
    printf("参数说明:\n");
    printf("  起点索引     - 出发地点的编号（整数，从 0 开始）\n");
    printf("  终点索引     - 目的地点的编号（整数，从 0 开始）\n");
    printf("  地图文件     - 地图数据文件路径（可选，默认: data/map.txt）\n");
    printf("  输出JSON文件 - 结果输出路径（可选，默认: data/result.json）\n\n");
    printf("示例:\n");
    printf("  %s 0 8                     # 查询 火车站→机场 的最短路径\n", progName);
    printf("  %s 0 8 mymap.txt out.json  # 使用自定义文件\n", progName);
}

/**
 * @brief  程序入口
 * @param  argc  参数个数
 * @param  argv  参数列表
 * @return 0 正常退出, 1 参数错误, 2 运行错误
 */
int main(int argc, char *argv[])
{
    /* ========== 第 1 步：解析命令行参数 ========== */

    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    /* 起点索引 */
    int start = atoi(argv[1]);

    /* 终点索引 */
    int end = atoi(argv[2]);

    /* 地图文件（可选参数） */
    const char *mapFile = (argc >= 4) ? argv[3] : "data/map.txt";

    /* JSON 输出文件（可选参数） */
    const char *outFile = (argc >= 5) ? argv[4] : "data/result.json";

    /* ========== 第 2 步：初始化图 ========== */

    Graph map;
    initGraph(&map);

    /* ========== 第 3 步：加载地图数据 ========== */

    printf("\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║     地图导航系统 - C 后端 (v2.0)            ║\n");
    printf("║     基于 Dijkstra 最短路径算法              ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    printf("[信息] 正在加载地图数据: \"%s\"\n", mapFile);

    if (loadFromFile(&map, mapFile) != 0) {
        fprintf(stderr, "[错误] 无法加载地图文件 \"%s\"。\n", mapFile);
        return 2;
    }

    /* ========== 第 4 步：参数校验 ========== */

    if (start < 0 || start >= map.vertexCount ||
        end   < 0 || end   >= map.vertexCount) {
        fprintf(stderr, "\n[错误] 地点编号无效！\n");
        fprintf(stderr, "  有效范围: 0 ~ %d\n", map.vertexCount - 1);
        fprintf(stderr, "  您输入了: 起点=%d, 终点=%d\n\n", start, end);
        fprintf(stderr, "  当前地点列表:\n");
        for (int i = 0; i < map.vertexCount; i++) {
            fprintf(stderr, "    [%d] %s\n", i, map.vertex[i]);
        }

        /* 即使参数无效也输出 JSON（状态为 invalid） */
        int prev[MAX_VERTEX] = {0};
        writeResultJSON(&map, start, end, INF, prev, outFile);
        return 2;
    }

    printf("[信息] 查询路径: \"%s\" → \"%s\"\n\n",
           map.vertex[start], map.vertex[end]);

    /* ========== 第 5 步：执行 Dijkstra 算法 ========== */

    /* 复用 dijkstra 模块的核心逻辑 */
    int n = map.vertexCount;
    int dist[MAX_VERTEX];
    int visited[MAX_VERTEX];
    int prev[MAX_VERTEX];

    for (int i = 0; i < n; i++) {
        dist[i]    = INF;
        visited[i] = 0;
        prev[i]    = -1;
    }

    dist[start] = 0;

    for (int count = 0; count < n; count++) {
        /* 选取未访问顶点中距离最小的 */
        int u = -1;
        int minVal = INF;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < minVal) {
                minVal = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visited[u] = 1;
        if (u == end) break;

        /* 松弛邻接点 */
        for (int v = 0; v < n; v++) {
            if (!visited[v]
                && map.edge[u][v] != INF
                && map.edge[u][v] != 0
                && dist[u] != INF
                && dist[u] + map.edge[u][v] < dist[v]) {
                dist[v] = dist[u] + map.edge[u][v];
                prev[v] = u;
            }
        }
    }

    /* ========== 第 6 步：输出结果 ========== */

    int distance = dist[end];

    printf("══════════════════════════════════════════════\n");
    printf("           最 短 路 径 查 询 结 果\n");
    printf("══════════════════════════════════════════════\n\n");
    printf("  起点: %s [%d]\n", map.vertex[start], start);
    printf("  终点: %s [%d]\n", map.vertex[end], end);

    if (distance == INF) {
        printf("\n  【警告】从 \"%s\" 无法到达 \"%s\"。\n",
               map.vertex[start], map.vertex[end]);
    } else {
        printf("  最短距离: %d\n", distance);
        printf("\n  最短路径:\n    ");
        printPathRecursive(prev, &map, end);
        printf("\n");
    }

    printf("\n══════════════════════════════════════════════\n\n");

    /* ========== 第 7 步：写入 result.json ========== */

    printf("[信息] 正在写入结果 JSON: \"%s\"\n", outFile);

    if (writeResultJSON(&map, start, end, distance, prev, outFile) != 0) {
        fprintf(stderr, "[错误] 写入 JSON 文件失败。\n");
        return 2;
    }

    printf("\n[完成] 后端计算结束。前端可读取 result.json 进行可视化。\n\n");
    return 0;
}
