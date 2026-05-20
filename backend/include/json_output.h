/**
 * @file    json_output.h
 * @brief   JSON 输出模块头文件 - 将最短路径结果写入 JSON 文件
 *
 * 本模块负责将 Dijkstra 算法计算结果输出为前端可读的 JSON 格式。
 * 输出文件为 result.json，供前端 AJAX/Fetch 读取。
 *
 * JSON 格式：
 *   {
 *     "distance": 12,
 *     "path": ["A", "C", "D", "F"],
 *     "start": "A",
 *     "end": "F",
 *     "status": "ok"
 *   }
 */

#ifndef JSON_OUTPUT_H
#define JSON_OUTPUT_H

#include "graph.h"
#include "dijkstra.h"

/**
 * @brief  将最短路径结果写入 JSON 文件
 * @param  g        指向图结构体的指针（用于获取顶点名称）
 * @param  start    起点索引
 * @param  end      终点索引
 * @param  distance 最短距离值
 * @param  prev     前驱节点数组（用于回溯路径）
 * @param  filename 输出文件路径（通常为 "data/result.json"）
 * @return 成功返回 0，失败返回 -1
 *
 * 生成的 JSON 包含：
 *   - distance: 最短距离数值
 *   - path:     路径顶点名称数组
 *   - start:    起点名称
 *   - end:      终点名称
 *   - status:   "ok" | "unreachable" | "same_node"
 */
int writeResultJSON(Graph *g, int start, int end, int distance,
                    int prev[], const char *filename);

/**
 * @brief  从 prev[] 数组构建路径顶点名称列表
 * @param  g     图结构体指针
 * @param  prev  前驱节点数组
 * @param  start 起点索引
 * @param  end   终点索引
 * @param  pathNames 输出：路径顶点名称数组（调用者分配 MAX_VERTEX 个 MAX_NAME 大小）
 * @return 路径中顶点数量
 */
int  buildPath(Graph *g, int prev[], int start, int end,
               char pathNames[MAX_VERTEX][MAX_NAME]);

/**
 * @brief  转义 JSON 字符串中的特殊字符
 * @param  src  源字符串
 * @param  dst  目标缓冲区
 * @param  size 目标缓冲区大小
 */
void jsonEscape(const char *src, char *dst, int size);

#endif /* JSON_OUTPUT_H */
