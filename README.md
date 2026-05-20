# 地图导航系统 (Map Navigation System)

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
│                    浏览器 (Browser)                   │
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
│            C 后端 (map_navigation_backend)            │
│  · 图结构 (邻接矩阵)                                 │
│  · Dijkstra 最短路径算法                             │
│  · 输出 result.json                                  │
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
map_navigation/
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
├── data/                        共享数据
│   ├── map.txt                  地图定义（20地点 + 40+道路）
│   └── result.json              查询结果（C 生成 → JS 读取）
│
├── server.py                    Python 桥接服务器
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

## 功能列表

| 功能 | 前端 | 后端 |
|------|:----:|:----:|
| 地图节点可视化（20个地点） | ✓ | — |
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

### GET /api/route

**参数**：

| 参数 | 类型 | 说明 |
|------|------|------|
| start | int | 起点顶点索引（0-19） |
| end | int | 终点顶点索引（0-19） |

**响应示例**：

```json
{
  "distance": 25,
  "start": "火车站",
  "end": "机场",
  "path": ["火车站", "机场"],
  "pathIndices": [0, 8],
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

`data/map.txt` 包含 20 个地点和 40+ 条道路：

**地点**：上海火车站、人民广场、外滩、南京东路、陆家嘴、东方明珠、豫园、徐家汇、上海交通大学、上海体育馆、虹桥机场、虹桥火车站、上海南站、世博园、张江高科技园区、上海科技馆、复旦大学、五角场、迪士尼乐园、浦东国际机场

> 想生成其他城市的地图？将 `prompts/city_map_generator.md` 的内容发给任意 AI（Claude、ChatGPT 等），即可自动生成对应城市的 `map.txt` 和 `layout.json`。

**路网示意**：

```
  大学城 ── 机场 ── 高铁站 ── 植物园
    │                  │
  理工大学 ── 市政府 ── 高新区
    │  │       │  │
  火车站 ─ 中心广场 │ 体育中心
    │         │     │
  滨江公园  商业中心 万达广场
    │         │     │
  人民公园  图书馆  人民公园
    │         │     │
  会展中心 ─ 动物园 ─ 植物园
```

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
| `backend/src/main.c` | ~150行 | 命令行入口 |

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
