/**
 * @file    json_output.c
 * @brief   JSON 输出模块实现 - 将最短路径结果序列化为 JSON 文件
 *
 * 本文件是前后端通信的关键桥梁：
 *   C 后端计算 Dijkstra 最短路径 → 生成 result.json → 前端 JS 读取并可视化
 *
 * 不依赖第三方 JSON 库，纯手写 JSON 序列化。
 * 对字符串中的特殊字符进行转义，确保输出合法 JSON。
 */

#include "json_output.h"
#include <string.h>

/* ================================================================
 * JSON 字符串转义
 *
 * 将 C 字符串中的 " \ 控制字符等转义为 JSON 合法形式。
 * 例如: "A"B" → "A\"B"
 * ================================================================ */
void jsonEscape(const char *src, char *dst, int size)
{
    if (src == NULL || dst == NULL || size <= 0) {
        return;
    }

    int j = 0;
    for (int i = 0; src[i] != '\0' && j < size - 2; i++) {
        switch (src[i]) {
        case '"':
            if (j + 1 < size - 1) { dst[j++] = '\\'; dst[j++] = '"';  }
            break;
        case '\\':
            if (j + 1 < size - 1) { dst[j++] = '\\'; dst[j++] = '\\'; }
            break;
        case '\n':
            if (j + 1 < size - 1) { dst[j++] = '\\'; dst[j++] = 'n';  }
            break;
        case '\r':
            if (j + 1 < size - 1) { dst[j++] = '\\'; dst[j++] = 'r';  }
            break;
        case '\t':
            if (j + 1 < size - 1) { dst[j++] = '\\'; dst[j++] = 't';  }
            break;
        default:
            dst[j++] = src[i];
            break;
        }
    }
    dst[j] = '\0';
}

/* ================================================================
 * 构建路径顶点名称列表
 *
 * 从终点向前回溯 prev[] 数组，收集路径上所有顶点索引，
 * 再映射为顶点名称字符串。
 *
 * 算法：
 *   1. 从 end 开始，沿 prev[] 向前追溯，将索引收集到栈中
 *   2. 直到 prev[i] == -1（即起点）
 *   3. 再将栈中的索引映射为顶点名称
 *   4. 结果按起点→终点顺序排列
 * ================================================================ */
int buildPath(Graph *g, int prev[], int start, int end,
              char pathNames[MAX_VERTEX][MAX_NAME])
{
    if (g == NULL || prev == NULL || pathNames == NULL) {
        return 0;
    }
    (void)start;  /* 保留参数以备扩展（如双向路径验证） */

    /* ---- 第 1 步：从终点回溯，收集路径索引 ---- */
    int stack[MAX_VERTEX];   /* 用作栈（倒序收集） */
    int top = 0;              /* 栈顶指针           */
    int cur = end;

    while (cur != -1) {
        if (top >= MAX_VERTEX) {
            break;  /* 防止溢出 */
        }
        stack[top++] = cur;
        cur = prev[cur];
    }

    /* ---- 第 2 步：反转栈（将起点→终点顺序写入 pathNames） ---- */
    int pathLen = top;
    for (int i = 0; i < pathLen; i++) {
        int idx = stack[pathLen - 1 - i];  /* 反转索引 */
        strncpy(pathNames[i], g->vertex[idx], MAX_NAME - 1);
        pathNames[i][MAX_NAME - 1] = '\0';
    }

    return pathLen;
}

/* ================================================================
 * 写入 result.json
 *
 * JSON 格式：
 * {
 *   "distance": 12,
 *   "path": ["火车站", "中心广场", "机场"],
 *   "start": "火车站",
 *   "end": "机场",
 *   "pathIndices": [0, 2, 8],
 *   "status": "ok"
 * }
 *
 * status 取值：
 *   "ok"           - 成功找到路径
 *   "unreachable"  - 不可达
 *   "same_node"    - 起点终点相同
 *   "invalid"      - 输入参数无效
 * ================================================================ */
int writeResultJSON(Graph *g, int start, int end, int distance,
                    int prev[], const char *filename)
{
    if (g == NULL || filename == NULL) {
        fprintf(stderr, "[错误] writeResultJSON: 参数为空。\n");
        return -1;
    }

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "[错误] 无法创建 JSON 文件 \"%s\"。\n", filename);
        return -1;
    }

    /* ---- 确定状态 ---- */
    const char *status;
    int pathIndices[MAX_VERTEX];
    int pathLen = 0;

    if (start < 0 || start >= g->vertexCount ||
        end   < 0 || end   >= g->vertexCount) {
        status = "invalid";
    } else if (start == end) {
        status = "same_node";
        pathIndices[0] = start;
        pathLen = 1;
    } else if (distance == INF) {
        status = "unreachable";
    } else {
        status = "ok";
        /* 从 prev[] 构建路径索引列表 */
        int stack[MAX_VERTEX];
        int top = 0;
        int cur = end;
        while (cur != -1 && top < MAX_VERTEX) {
            stack[top++] = cur;
            cur = prev[cur];
        }
        /* 反转 */
        pathLen = top;
        for (int i = 0; i < pathLen; i++) {
            pathIndices[i] = stack[pathLen - 1 - i];
        }
    }

    /* ---- 写入 JSON ---- */
    fprintf(fp, "{\n");

    /* distance */
    if (distance == INF) {
        fprintf(fp, "  \"distance\": null,\n");
    } else {
        fprintf(fp, "  \"distance\": %d,\n", distance);
    }

    /* start / end 名称 */
    char escaped[MAX_NAME * 2];
    jsonEscape((start >= 0 && start < g->vertexCount) ? g->vertex[start] : "",
               escaped, sizeof(escaped));
    fprintf(fp, "  \"start\": \"%s\",\n", escaped);

    jsonEscape((end >= 0 && end < g->vertexCount) ? g->vertex[end] : "",
               escaped, sizeof(escaped));
    fprintf(fp, "  \"end\": \"%s\",\n", escaped);

    /* path 顶点名称数组 */
    fprintf(fp, "  \"path\": [");
    if (pathLen > 0 && status[0] == 'o') {  /* ok */
        for (int i = 0; i < pathLen; i++) {
            jsonEscape(g->vertex[pathIndices[i]], escaped, sizeof(escaped));
            fprintf(fp, "\"%s\"%s", escaped, (i < pathLen - 1) ? ", " : "");
        }
    } else if (status[0] == 's') {  /* same_node */
        jsonEscape(g->vertex[start], escaped, sizeof(escaped));
        fprintf(fp, "\"%s\"", escaped);
    }
    fprintf(fp, "],\n");

    /* pathIndices 数组（方便前端直接索引） */
    fprintf(fp, "  \"pathIndices\": [");
    if (pathLen > 0 && (status[0] == 'o' || status[0] == 's')) {
        for (int i = 0; i < pathLen; i++) {
            fprintf(fp, "%d%s", pathIndices[i], (i < pathLen - 1) ? ", " : "");
        }
    }
    fprintf(fp, "],\n");

    /* status */
    fprintf(fp, "  \"status\": \"%s\"\n", status);

    fprintf(fp, "}\n");

    fclose(fp);
    printf("[成功] 结果已写入 \"%s\"。\n", filename);
    return 0;
}
