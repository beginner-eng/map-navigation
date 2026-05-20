/**
 * @file    graph.h
 * @brief   图模块头文件 - 定义图的数据结构、常量与基本操作接口
 *
 * 本模块基于邻接矩阵实现有向/无向图，支持：
 *   - 顶点的增删查
 *   - 边的增删改
 *   - 邻接矩阵打印
 *   - 地图数据文件的读写
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ================================================================
 * 符号常量
 * ================================================================ */
#define MAX_VERTEX  20          /* 地图最大地点数量            */
#define MAX_NAME    32          /* 地点名称最大长度（字节）    */
#define INF         99999       /* 无穷大，表示两点间无边      */

/* ================================================================
 * 数据结构
 * ================================================================ */

/**
 * @struct Graph
 * @brief  图结构体
 *
 * @field vertex       一维字符串数组，vertex[i] 为第 i 个顶点的名称
 * @field edge         二维整型数组（邻接矩阵），edge[i][j] 表示 i→j 的权值
 * @field vertexCount  当前顶点总数
 */
typedef struct {
    char vertex[MAX_VERTEX][MAX_NAME];  /* 顶点名称列表          */
    int  edge[MAX_VERTEX][MAX_VERTEX];  /* 邻接矩阵（权值表）    */
    int  vertexCount;                   /* 已添加的顶点数量      */
} Graph;

/* ================================================================
 * 核心 API 声明
 * ================================================================ */

/**
 * @brief  初始化图为空状态
 * @param  g  指向图结构体的指针
 */
void initGraph(Graph *g);

/**
 * @brief  向图中添加一个地点（顶点）
 * @param  g     指向图结构体的指针
 * @param  name  地点名称（字符串）
 * @return 成功返回顶点索引（≥0），失败返回 -1
 *
 * 失败场景：
 *   - 已达 MAX_VERTEX 上限
 *   - 名称已存在（重复添加）
 *   - 名称为空
 */
int  addVertex(Graph *g, const char *name);

/**
 * @brief  在两个已有顶点之间添加一条有向边（同时也会添加反向边以保证双向通行）
 * @param  g      指向图结构体的指针
 * @param  start  起点顶点索引
 * @param  end    终点顶点索引
 * @param  weight 边权值（距离）
 * @return 成功返回 0，失败返回 -1
 *
 * 失败场景：
 *   - 索引越界
 *   - 权值 ≤ 0
 *   - 起点 == 终点（自环不允许）
 */
int  addEdge(Graph *g, int start, int end, int weight);

/**
 * @brief  修改已有边的权值
 * @param  g      指向图结构体的指针
 * @param  start  起点顶点索引
 * @param  end    终点顶点索引
 * @param  weight 新的边权值
 * @return 成功返回 0，失败返回 -1
 */
int  modifyEdge(Graph *g, int start, int end, int weight);

/**
 * @brief  查找顶点名称对应的索引
 * @param  g     指向图结构体的指针
 * @param  name  地点名称
 * @return 找到返回索引（≥0），未找到返回 -1
 */
int  findVertex(Graph *g, const char *name);

/**
 * @brief  以可读形式打印所有地点及其连接关系
 * @param  g  指向图结构体的指针
 */
void showGraph(Graph *g);

/**
 * @brief  打印邻接矩阵（行/列均为顶点名称）
 * @param  g  指向图结构体的指针
 */
void printMatrix(Graph *g);

/**
 * @brief  从文本文件中加载地图数据
 * @param  g        指向图结构体的指针
 * @param  filename 文件路径
 * @return 成功返回 0，失败返回 -1
 *
 * 文件格式见 data/map.txt 示例
 */
int  loadFromFile(Graph *g, const char *filename);

/**
 * @brief  将当前地图保存到文本文件
 * @param  g        指向图结构体的指针
 * @param  filename 文件路径
 * @return 成功返回 0，失败返回 -1
 */
int  saveToFile(Graph *g, const char *filename);

#endif /* GRAPH_H */
