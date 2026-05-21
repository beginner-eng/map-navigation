# 地图导航系统 (Map Navigation System)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/backend-C99-555555.svg)](backend/)
[![Frontend](https://img.shields.io/badge/frontend-Vanilla%20JS%20%2B%20Cytoscape.js-4ec9b0.svg)](frontend/)
[![Python](https://img.shields.io/badge/bridge-Python%203-3776ab.svg)](server.py)

> 基于 **图数据结构 + Dijkstra 最短路径算法** 的全栈地图导航系统
>
> 后端：C 语言 | 前端：HTML/CSS/JS + Cytoscape.js | 桥接：Python

---

## 项目简介

本项目是一个**前后端分离**的完整地图导航系统：

- **后端**：纯 C 语言实现，基于邻接矩阵的图数据结构 + 手写 Dijkstra 最短路径算法
- **前端**：现代 Web 界面，使用 Cytoscape.js 实现图可视化，支持交互式路径查询
- **桥接层**：Python 服务器连接前后端，实现 HTTP API → C 程序 → JSON 响应

适用于：
- 大学数据结构课程设计 / 大作业
- 图论与最短路径算法学习
- 全栈项目练习（C + Web + 可视化）

---

## 系统架构

```
┌─────────────────────────────────────────────────────┐
│                    浏览器 (Browser)                  │
│  ┌─────────────────────────────────────────────────┐│
│  │  前端: HTML + CSS + JS + Cytoscape.js           ││
│  │  · 地图节点/边可视化                             ││
│  │  · 下拉框选择起点/终点                           ││
│  │  · 按钮触发计算                                  ││
│  │  · 路径高亮 + 距离显示                           ││
│  └──────────────────┬──────────────────────────────┘│
└─────────────────────┼───────────────────────────────┘
                      │ HTTP GET /api/route?start=X&end=Y
                      ▼
┌─────────────────────────────────────────────────────┐
│               Python 桥接服务器 (server.py)          │
│  · 接收 HTTP 请求                                    │
│  · 调用 C 后端程序                                   │
│  · 读取 result.json 返回 JSON 响应                   │
└──────────────────┬──────────────────────────────────┘
                   │ subprocess: map_navigation_backend X Y
                   ▼
┌─────────────────────────────────────────────────────┐
│            C 后端 (map_navigation_backend)          │
│  · 图结构 (邻接矩阵)                                 │
│  · Dijkstra 最短路径算法                             │
│  · 输出 result.json                                 │
└─────────────────────────────────────────────────────┘
```

---

## 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| 前端 | HTML5 + CSS3 + JavaScript (ES6+) | 交互界面 |
| 图可视化 | Cytoscape.js 3.28 | 图论可视化库 |
| 桥接 | Python 3 (标准库) | HTTP 服务器 + 进程调用 |
| 后端 | C 语言 (C99) | 图结构 + Dijkstra 算法 |
| 编译 | GCC + Makefile | C 代码编译 |
| 数据 | JSON | 前后端数据交换格式 |

---

## 工程结构

```
dijkstra-map-navigator/
│
├── backend/                     C 语言后端
│   ├── include/
│   │   ├── graph.h              图模块头文件
│   │   ├── dijkstra.h           Dijkstra 算法头文件
│   │   └── json_output.h        JSON 输出模块头文件
│   ├── src/
│   │   ├── main.c               后端入口（命令行参数驱动）
│   │   ├── graph.c              图操作实现
│   │   ├── dijkstra.c           Dijkstra 算法实现
│   │   └── json_output.c        JSON 序列化实现
│   └── Makefile                 编译脚本
│
├── frontend/                    Web 前端
│   ├── index.html               HTML 页面结构
│   ├── style.css                CSS 样式（深色科技风主题）
│   └── script.js                JS 核心脚本（Cytoscape.js + 通信）
│
├── tools/                       开发工具
│   └── generate_layout.py        布局文件生成器
│
├── prompts/                     AI 提示词模板
│   └── city_map_generator.md     城市地图生成器
│
├── maps/                       多城市地图数据
│   └── shanghai.txt             上海地图（内置）
│
├── data/                        共享数据
│   ├── map.txt                  默认地图定义（精简版回退）
│   ├── layouts/                 预设布局坐标
│   │   └── shanghai.json
│   └── result.json              查询结果（运行时生成）
│
├── server.py                    Python 桥接服务器
├── .gitignore                   Git 忽略规则
└── README.md                    本文件
```

---

## 快速开始

### 1. 环境准备

- **GCC 编译器**（MinGW / GCC 5.0+）
- **Python 3.6+**（标准库即可，无需 pip 安装）
- **现代浏览器**（Chrome / Edge / Firefox）

### 2. 编译 C 后端

**方式一：使用 make（推荐）**

```bash
cd backend
make
```

**方式二：手动编译（无需 make）**

```bash
cd backend
gcc -Wall -Wextra -std=c99 -pedantic -O2 -Iinclude -o map_navigation_backend src/main.c src/graph.c src/dijkstra.c src/json_output.c
```

编译成功后会生成 `map_navigation_backend` (Linux/Mac) 或 `map_navigation_backend.exe` (Windows)。

### 3. 启动桥接服务器

```bash
# 在项目根目录下执行
python server.py

# 或指定端口
python server.py 3000

# 开启调试模式
python server.py 8080 --debug
```

### 4. 打开浏览器

访问：**http://localhost:8080**

### 5. 使用系统

1. 在左侧面板选择**起点**和**终点**
2. 点击「计算最短路径」按钮
3. 地图自动高亮最短路径（红色边 + 节点）
4. 结果卡片显示最短距离和路径详情

---

## 界面预览

![界面截图](docs/screenshot.png)

---

## 功能列表

| 功能 | 前端 | 后端 |
|------|:----:|:----:|
| 地图节点可视化（上限50个） | ✓ | — |
| 道路边显示（带权值标注） | ✓ | — |
| 下拉框选择起点/终点 | ✓ | — |
| 最短路径计算 | — | ✓ (Dijkstra) |
| 最短路径高亮（红色动画） | ✓ | — |
| 最短距离显示 | ✓ | — |
| 路径顺序显示 | ✓ | — |
| 重复查询 | ✓ | ✓ |
| 图例说明 | ✓ | — |
| 鼠标缩放/拖拽/悬停 | ✓ | — |
| 邻接矩阵维护 | — | ✓ |
| JSON 结果输出 | — | ✓ |
| 文件加载/保存地图 | — | ✓ |
| Toast 通知提示 | ✓ | — |
| 响应式布局 | ✓ | — |

---

## API 接口

### GET /api/maps

返回可用城市列表。

**响应示例**：

```json
["shanghai", "beijing"]
```

### GET /api/map

返回地图数据。支持 `?city=` 参数选择城市（对应 `maps/<city>.txt`），省略时使用 `data/map.txt`。

**响应示例**：

```json
{
  "locations": ["上海火车站", "人民广场", ...],
  "roads": [[0, 1, 4], [1, 2, 3], ...]
}
```

### GET /api/route

**参数**：

| 参数 | 类型 | 说明 |
|------|------|------|
| start | int | 起点顶点索引 |
| end | int | 终点顶点索引 |
| city | string | 可选，城市名（对应 `maps/<city>.txt`） |

**响应示例**：

```json
{
  "distance": 12,
  "start": "上海火车站",
  "end": "陆家嘴",
  "path": ["上海火车站", "人民广场", "外滩", "陆家嘴"],
  "pathIndices": [0, 1, 2, 4],
  "status": "ok"
}
```

**status 取值**：
- `ok` — 查询成功，已找到最短路径
- `unreachable` — 不可达
- `same_node` — 起点终点相同
- `invalid` — 参数无效
- `backend_not_found` — C 后端未编译
- `timeout` — 计算超时

---

## 预设地图数据

项目支持**多个城市地图**，启动时通过下拉框切换。地图文件存放在 `maps/` 目录，内置**上海**（48 个地点）。

> **想添加新城市？**
> 
> 1. 把 `prompts/city_map_generator.md` 的内容发给任意 AI，获取 `map.txt`
> 2. 放入 `maps/` 目录（如 `maps/beijing.txt`）
> 3. 运行 `python tools/generate_layout.py maps/<城市名>.txt` 生成固定布局
> 4. 刷新浏览器，新城市出现在下拉框中
> 
> 第 3 步只做一次。跳过也可用，但地图每次刷新形状会变化。

**路网示意**：

```
     虹桥机场 ── 虹桥火车站 ── 上海火车站
        │            │              │
        │        上海南站       人民广场 ── 南京东路 ── 外滩
        │            │            │  │                    │
        └────── 徐家汇 ────────── 豫园                 陆家嘴 ── 东方明珠
                  │  │                                   │
            上海交大 上海体育馆                   世博园 ─ 张江 ─ 上海科技馆
                  │                                      │        │
            复旦大学 ── 五角场                      浦东机场   迪士尼
```

> 以上为上海路网示意。成都等城市有类似的分区结构，详见 `maps/` 目录下的地图文件。

---

## Dijkstra 算法实现

### 核心流程

```
1. 初始化:
   dist[start] = 0,  其余 dist[i] = INF
   visited[i] = 0,   prev[i] = -1

2. 重复 n 次:
   a) 从未访问点中选 dist 最小的 u
   b) visited[u] = 1
   c) if u == end: break
   d) 对 u 的每个邻接点 v:
      if dist[u] + w(u,v) < dist[v]:
         dist[v] = dist[u] + w(u,v)
         prev[v] = u

3. 从 prev[] 回溯输出完整路径
```

- **时间复杂度**：O(V²)
- **空间复杂度**：O(V²) (邻接矩阵)
- **路径回溯**：使用 prev[] 数组 + 栈反转

### 关键代码位置

| 文件 | 行数 | 内容 |
|------|------|------|
| `backend/src/dijkstra.c` | ~250行 | Dijkstra 完整实现 |
| `backend/src/graph.c` | ~400行 | 图操作 + 文件 I/O |
| `backend/src/json_output.c` | ~200行 | JSON 序列化 |
| `backend/src/main.c` | ~190行 | 命令行入口 |

---

## Cytoscape.js 使用说明

### 初始化

```javascript
cy = cytoscape({
    container: document.getElementById('cy'),
    elements: [...],   // 节点 + 边
    style: [...],      // CSS 样式规则
    layout: { name: 'cose' }  // 力导向布局
});
```

### 路径高亮

```javascript
// 添加 CSS 类
edge.addClass('path-edge');
node.addClass('path-node');

// 对应 CSS 规则
// edge.path-edge { line-color: #ff6b6b; width: 5px; }
```

### 交互

- 滚轮缩放、拖拽平移
- 鼠标悬停节点高亮
- 点击节点查看信息

---

## 数据分层设计

项目采用**三层数据分离**架构，map 数据、布局坐标、查询结果各自独立：

| 文件 | 职责 | 格式 |
|------|------|------|
| `maps/<城市>.txt` | 图结构 + 边权值 | `V` / `E` 文本 |
| `data/layouts/<城市>.json` | 节点固定坐标 | JSON，`preset` 布局 |
| `data/result.json` | 最短路径查询结果 | JSON（运行时生成） |

- **map.txt**：定义有哪些地点、哪些道路、每条路多长。适合 C 语言解析和 Dijkstra 算法。
- **layout.json**：预计算每个节点的固定 x/y 坐标。前端加载后使用 Cytoscape.js 的 `preset` 布局，所有节点位置固定，每次刷新一致。若缺少此文件，自动回退力导向布局。
- **result.json**：最短路径计算结果，由 C 后端生成。

使用 `python tools/generate_layout.py maps/<城市>.txt` 可为新城市生成布局文件。

---

## 为什么同一张地图刷新后形状可能变化？

项目已采用 **preset 布局**（`data/layouts/<城市>.json`），大部分情况下刷新后形状一致。但在以下场景仍会触发力导向算法回退：

- 新城市尚无布局文件
- 布局文件加载失败
- 布局文件被删除

力导向算法以边的权值作为弹簧的理想长度进行迭代，初始位置带有随机扰动，因此同一组距离关系可能对应多种合理的几何形态。**这不会影响最短路径计算结果**——图结构、边权值和 Dijkstra 算法输出始终一致。

如需彻底固定地图形状，运行 `python tools/generate_layout.py maps/<城市>.txt` 生成布局文件。

---

## 扩展方向

- [ ] 添加地点/道路功能（前端表单 + 后端 API）
- [ ] A* 算法支持（已在 dijkstra.h 预留接口）
- [ ] 多路径查询（K-最短路径）
- [ ] WebSocket 实时通信
- [ ] 数据库存储地图数据
- [ ] Docker 容器化部署
- [ ] 路径动画（逐段显示）
- [ ] 移动端适配

---

## License

MIT License — 自由使用、修改、分发。
