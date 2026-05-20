/**
 * @file    dijkstra.h
 * @brief   Dijkstra 最短路径算法模块头文件
 *
 * 本模块封装了：
 *   - Dijkstra 最短路径计算
 *   - 路径回溯与打印（递归方式）
 *   - 最短距离查询
 *
 * 依赖 graph.h 中定义的 Graph 结构体。
 */

#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

/**
 * @brief  使用 Dijkstra 算法计算从 start 到 end 的最短路径
 * @param  g     指向已构建好的图
 * @param  start 起点顶点索引
 * @param  end   终点顶点索引
 *
 * 算法说明：
 *   1. 初始化 dist[] = INF, visited[] = 0, prev[] = -1
 *   2. 从起点开始，dist[start] = 0
 *   3. 每次选取未访问顶点中 dist 最小的 u
 *   4. 以 u 为中介，尝试松弛其所有邻接点
 *   5. 重复直至所有顶点访问完毕 或 到达终点
 *   6. 输出最短距离及完整路径
 *
 * 异常处理：
 *   - 起点/终点不存在 → 提示并返回
 *   - 起点==终点 → 距离为 0
 *   - 不可达 → 提示无路径
 */
void dijkstra(Graph *g, int start, int end);

/**
 * @brief  递归回溯 prev[] 数组，打印从 start 到 end 的完整路径
 * @param  prev  前驱节点数组（prev[i] 表示 i 的前一个顶点）
 * @param  g     指向图结构体（用于获取顶点名称）
 * @param  end   终点索引
 *
 * 采用递归方式：
 *   - 若 prev[end] == -1，说明已回溯到起点或不可达
 *   - 先递归输出前缀路径，再输出本节点
 *   - 输出格式：A -> B -> C -> D
 */
void printPathRecursive(int prev[], Graph *g, int end);

/**
 * @brief  获取从 start 到 end 的最短距离（不打印路径）
 * @param  g     指向图结构体
 * @param  start 起点索引
 * @param  end   终点索引
 * @return 最短距离值，若不可达返回 INF
 *
 * 内部复用 Dijkstra 算法的核心逻辑，但只返回 dist[end]。
 */
int  getShortestDistance(Graph *g, int start, int end);

#endif /* DIJKSTRA_H */
