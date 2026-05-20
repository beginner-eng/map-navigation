/**
 * ============================================================
 *  script.js - 地图导航系统前端核心脚本
 * ============================================================
 *
 *  功能模块:
 *    1. 地图数据定义（20个地点 + 道路网络）
 *    2. Cytoscape.js 图可视化初始化
 *    3. 节点/边样式配置
 *    4. 下拉框填充
 *    5. 后端通信（请求Python桥接服务器）
 *    6. 最短路径高亮显示
 *    7. 交互事件处理
 *
 *  通信流程:
 *    前端点击"计算" → Fetch /api/route?start=X&end=Y
 *    → Python 桥接服务器运行 C 后端
 *    → C 后端生成 result.json
 *    → Python 桥接返回 JSON → 前端高亮路径
 *
 *  依赖: Cytoscape.js (CDN)
 * ============================================================ */

/* ================================================================
 * 第 1 部分：地图数据定义
 * ================================================================ */

/** 地点数组 — 与 C 后端 data/map.txt 保持同步（上海路网） */
const LOCATIONS = [
    "上海火车站", "人民广场", "外滩", "南京东路", "陆家嘴",
    "东方明珠", "豫园", "徐家汇", "上海交通大学", "上海体育馆",
    "虹桥机场", "虹桥火车站", "上海南站", "世博园", "张江高科技园区",
    "上海科技馆", "复旦大学", "五角场", "迪士尼乐园", "浦东国际机场"
];

/** 道路数组 [起点索引, 终点索引, 距离] — 上海城市路网 */
const ROADS = [
    // 主城区核心路网
    [0, 1, 4],   [1, 2, 3],   [1, 3, 2],   [2, 4, 5],
    [4, 5, 1],   [2, 6, 2],   [3, 6, 3],
    // 徐家汇/高校区域
    [1, 7, 6],   [7, 8, 3],   [7, 9, 2],   [8, 12, 5],
    [9, 12, 4],
    // 虹桥交通枢纽
    [7, 10, 10], [10, 11, 2], [11, 12, 9], [0, 11, 12],
    // 浦东区域
    [4, 13, 7],  [13, 14, 8], [14, 15, 3], [4, 15, 4],
    // 杨浦/大学区域
    [1, 16, 11], [16, 17, 2], [17, 14, 10],
    // 迪士尼/浦东机场
    [15, 18, 14],[18, 19, 18],[14, 19, 20],
    // 增强连通性
    [5, 15, 5],  [6, 13, 6],  [3, 7, 7],   [8, 16, 13],
    [10, 19, 30],[4, 14, 9],  [11, 19, 35],[2, 5, 6],
    [13, 18, 12]
];

/* ================================================================
 * 第 2 部分：距离比例布局算法 (Distance-Proportional Layout)
 *
 * 算法目标:
 *   视觉上边的长度 ≈ 实际道路距离 × 缩放因子
 *
 * 实现:
 *   自研力导向(Force-Directed)迭代算法:
 *     1. 弹簧力: 每条边 e(u,v,w) 产生弹簧力，理想长度 = w × SCALE
 *        - 若当前距离 > 理想长度 → 拉力（缩短）
 *        - 若当前距离 < 理想长度 → 推力（伸长）
 *     2. 节点斥力: 所有节点对之间有斥力，防止重叠
 *     3. 中心引力: 所有节点被轻微拉向中心，防止漂移
 *     4. 阻尼: 逐步衰减速度，使系统收敛
 *     5. 迭代至稳定
 *
 * 效果:
 *   - 距离 5 的道路 ≈ 90px
 *   - 距离 25 的道路 ≈ 450px
 *   - 视觉上一目了然哪条路更长
 * ================================================================ */

/** 全局 Cytoscape 实例 */
let cy;

/**
 * 将内部数据转换为 Cytoscape.js 元素格式
 */
function buildCyElements() {
    const elements = [];

    LOCATIONS.forEach((name, idx) => {
        elements.push({
            data: { id: String(idx), label: name }
        });
    });

    ROADS.forEach(([from, to, weight]) => {
        elements.push({
            data: {
                id: `e${from}_${to}`,
                source: String(from),
                target: String(to),
                weight: weight
            }
        });
    });

    return elements;
}

/* ================================================================
 * 距离比例布局核心算法
 * ================================================================ */

/**
 * 构建无向图的邻接表（从 ROADS 数组）
 * @returns {Array<Array<[neighborIdx, weight]>>}
 */
function buildAdjList() {
    const adj = new Array(LOCATIONS.length);
    for (let i = 0; i < LOCATIONS.length; i++) {
        adj[i] = [];
    }
    ROADS.forEach(([u, v, w]) => {
        adj[u].push([v, w]);
        adj[v].push([u, w]);
    });
    return adj;
}

/**
 * 使用 Floyd-Warshall 计算所有节点对之间的最短路径距离
 * 用于初始化布局 — 尝试在 2D 平面上逼近高维距离矩阵
 * @param {Array} adj 邻接表
 * @returns {Array<Array<number>>} dist[u][v] = 最短距离
 */
function computeAllPairsShortestPath(adj) {
    const n = LOCATIONS.length;
    const dist = new Array(n);
    for (let i = 0; i < n; i++) {
        dist[i] = new Array(n).fill(Infinity);
        dist[i][i] = 0;
    }

    // 直接边权
    ROADS.forEach(([u, v, w]) => {
        dist[u][v] = Math.min(dist[u][v], w);
        dist[v][u] = Math.min(dist[v][u], w);
    });

    // Floyd-Warshall
    for (let k = 0; k < n; k++) {
        for (let i = 0; i < n; i++) {
            for (let j = 0; j < n; j++) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }

    return dist;
}

/**
 * 基于距离比例的力导向布局
 *
 * 算法参数:
 *   SCALE       - 像素/距离单位（边权×SCALE = 理想视觉像素长度）
 *   SPRING_K    - 弹簧刚度（越大收敛越快但可能震荡）
 *   REPULSION   - 节点斥力强度（防止重叠）
 *   DAMPING     - 速度衰减系数（0~1，越大收敛越慢但越稳定）
 *   GRAVITY     - 中心引力（防止节点飘走）
 *
 * @returns {Object}  { id: {x, y} }  位置字典
 */
function computeDistanceBasedLayout() {
    const n = LOCATIONS.length;
    const adj = buildAdjList();
    const allPairsDist = computeAllPairsShortestPath(adj);

    // ---- 可调参数 ----
    const SCALE       = 18.0;   // 像素/单位：w=5→90px, w=25→450px
    const SPRING_K    = 0.25;   // 弹簧刚度
    const REPULSION   = 8000;   // 斥力常数
    const DAMPING     = 0.88;   // 速度阻尼
    const GRAVITY     = 0.008;  // 中心引力强度
    const MAX_ITER    = 350;    // 最大迭代次数
    const CONVERGE_EPS = 0.06; // 收敛判定：最大位移阈值（像素）
    const DT          = 0.6;    // 时间步长

    // ---- 初始化位置（使用 Pivot MDS 思想：选最远两点做主轴） ----
    const posX = new Float64Array(n);
    const posY = new Float64Array(n);
    const velX = new Float64Array(n);
    const velY = new Float64Array(n);

    // 找最远点对作为主轴两端（直径）
    let maxDist = -1, pivotA = 0, pivotB = 0;
    for (let i = 0; i < n; i++) {
        for (let j = i + 1; j < n; j++) {
            if (allPairsDist[i][j] > maxDist && allPairsDist[i][j] < Infinity) {
                maxDist = allPairsDist[i][j];
                pivotA = i;
                pivotB = j;
            }
        }
    }

    // 使用主轴 + 最短路径距离初始化 2D 坐标
    const axisLen = maxDist * SCALE;
    const cx = 450, cy = 340;  // 画布中心（≈800×680/2）
    const dx = axisLen * 0.5;
    const dy = axisLen * 0.3;

    for (let i = 0; i < n; i++) {
        // 投影到主轴 AB 上
        const da = allPairsDist[i][pivotA];
        const db = allPairsDist[i][pivotB];
        const total = da + db;

        if (total > 0 && total < Infinity) {
            // 沿 AB 主轴放置
            const t = da / total;
            posX[i] = cx + (t - 0.5) * axisLen;
            // 垂直方向用节点编号做轻微偏移（打破共线）
            const perpOffset = ((i % 7) - 3) * 35 + (Math.random() - 0.5) * 20;
            posY[i] = cy + perpOffset;
        } else {
            // 孤立节点放在角落
            posX[i] = cx + (Math.random() - 0.5) * 200;
            posY[i] = cy + (Math.random() - 0.5) * 200;
        }
        velX[i] = 0;
        velY[i] = 0;
    }

    // ---- 主迭代循环 ----
    console.log('[布局] 开始距离比例力导向迭代...');

    for (let iter = 0; iter < MAX_ITER; iter++) {
        const forceX = new Float64Array(n);
        const forceY = new Float64Array(n);

        // ---- 1. 边弹簧力（核心：让边长度逼近 w × SCALE） ----
        for (const [u, v, weight] of ROADS) {
            let dx = posX[v] - posX[u];
            let dy = posY[v] - posY[u];
            let dist = Math.sqrt(dx * dx + dy * dy);

            if (dist < 0.5) {
                // 避免除零：随机微扰打破重合
                dist = 0.5;
                dx = (Math.random() - 0.5) * 2;
                dy = (Math.random() - 0.5) * 2;
            }

            const ideal = weight * SCALE;          // 理想长度（像素）
            const displacement = dist - ideal;      // 正=太长需缩短，负=太短需伸长
            const forceMag = displacement * SPRING_K;

            const fx = (dx / dist) * forceMag;
            const fy = (dy / dist) * forceMag;

            forceX[u] += fx;
            forceX[v] -= fx;
            forceY[u] += fy;
            forceY[v] -= fy;
        }

        // ---- 2. 全局节点斥力（避免重叠） ----
        for (let i = 0; i < n; i++) {
            for (let j = i + 1; j < n; j++) {
                let dx = posX[j] - posX[i];
                let dy = posY[j] - posY[i];
                let dist = Math.sqrt(dx * dx + dy * dy);

                if (dist < 0.5) {
                    dist = 0.5;
                    dx = (Math.random() - 0.5) * 2;
                    dy = (Math.random() - 0.5) * 2;
                }

                const repForce = REPULSION / (dist * dist);
                const fx = (dx / dist) * repForce;
                const fy = (dy / dist) * repForce;

                forceX[i] -= fx;
                forceX[j] += fx;
                forceY[i] -= fy;
                forceY[j] += fy;
            }
        }

        // ---- 3. 中心引力（防止漂移） ----
        for (let i = 0; i < n; i++) {
            forceX[i] += (cx - posX[i]) * GRAVITY;
            forceY[i] += (cy - posY[i]) * GRAVITY;
        }

        // ---- 4. 更新速度和位置 ----
        let maxDisp = 0;
        for (let i = 0; i < n; i++) {
            velX[i] = velX[i] * DAMPING + forceX[i] * DT;
            velY[i] = velY[i] * DAMPING + forceY[i] * DT;

            // 限速
            const speed = Math.sqrt(velX[i] * velX[i] + velY[i] * velY[i]);
            const maxSpeed = 15;
            if (speed > maxSpeed) {
                velX[i] = (velX[i] / speed) * maxSpeed;
                velY[i] = (velY[i] / speed) * maxSpeed;
            }

            posX[i] += velX[i] * DT;
            posY[i] += velY[i] * DT;

            const disp = Math.abs(velX[i] * DT) + Math.abs(velY[i] * DT);
            if (disp > maxDisp) maxDisp = disp;
        }

        // 收敛判断
        if (maxDisp < CONVERGE_EPS && iter > 50) {
            console.log(`[布局] 迭代 ${iter + 1} 次后收敛（maxDisp=${maxDisp.toFixed(3)}）`);
            break;
        }

        // 每 100 轮报告一次
        if ((iter + 1) % 100 === 0) {
            console.log(`[布局] 迭代 ${iter + 1}/${MAX_ITER}, maxDisp=${maxDisp.toFixed(2)}`);
        }

        if (iter === MAX_ITER - 1) {
            console.log(`[布局] 达最大迭代次数 ${MAX_ITER}（maxDisp=${maxDisp.toFixed(3)}）`);
        }
    }

    // ---- 度量布局质量 ----
    let totalError = 0;
    let edgeCount = 0;
    for (const [u, v, weight] of ROADS) {
        const dx = posX[u] - posX[v];
        const dy = posY[u] - posY[v];
        const euclidean = Math.sqrt(dx * dx + dy * dy);
        const ideal = weight * SCALE;
        const err = Math.abs(euclidean - ideal) / ideal; // 相对误差
        totalError += err;
        edgeCount++;
    }
    const avgError = (totalError / edgeCount * 100).toFixed(1);
    console.log(`[布局] 平均相对误差: ${avgError}%（${edgeCount}条边，SCALE=${SCALE}）`);

    // 暴露布局质量到全局，供 toolbar 显示
    window._layoutQuality = avgError;

    // ---- 组装结果 ----
    const positions = {};
    for (let i = 0; i < n; i++) {
        positions[String(i)] = { x: posX[i], y: posY[i] };
    }

    return positions;
}

/* ================================================================
 * Cytoscape.js 初始化
 * ================================================================ */

function initCytoscape() {
    const elements = buildCyElements();

    // 使用距离比例布局（替代硬编码坐标）
    const positions = computeDistanceBasedLayout();

    cy = cytoscape({
        container: document.getElementById('cy'),

        elements: elements,

        style: [
            /* ---- 节点默认 ---- */
            {
                selector: 'node',
                style: {
                    'background-color': '#4ec9b0',
                    'label': 'data(label)',
                    'color': '#e0e0e0',
                    'font-size': '10px',
                    'font-family': '"PingFang SC", "Microsoft YaHei", sans-serif',
                    'text-valign': 'bottom',
                    'text-halign': 'center',
                    'text-margin-y': 5,
                    'width': 20,
                    'height': 20,
                    'border-width': 2,
                    'border-color': '#2d8a75',
                    'transition-property': 'background-color, border-color, width, height',
                    'transition-duration': '0.3s',
                    'text-outline-width': 2,
                    'text-outline-color': '#1a1a28'
                }
            },

            /* ---- 边默认 ---- */
            {
                selector: 'edge',
                style: {
                    'width': 2.5,
                    'line-color': '#3a3d41',
                    'target-arrow-color': '#3a3d41',
                    'target-arrow-shape': 'triangle',
                    'arrow-scale': 1.2,
                    'curve-style': 'bezier',
                    'label': 'data(weight)',
                    'color': '#707070',
                    'font-size': '10px',
                    'font-family': '"Consolas", monospace',
                    'text-background-color': '#1a1a28',
                    'text-background-opacity': 0.8,
                    'text-background-padding': '2px',
                    'text-background-shape': 'roundrectangle',
                    'transition-property': 'line-color, target-arrow-color, width',
                    'transition-duration': '0.3s'
                }
            },

            /* ---- 起点 ---- */
            {
                selector: 'node.start-node',
                style: {
                    'background-color': '#569cd6',
                    'border-color': '#3a7ab5',
                    'border-width': 4,
                    'width': 28,
                    'height': 28,
                    'font-weight': 'bold'
                }
            },

            /* ---- 终点 ---- */
            {
                selector: 'node.end-node',
                style: {
                    'background-color': '#ce9178',
                    'border-color': '#a86a51',
                    'border-width': 4,
                    'width': 28,
                    'height': 28,
                    'font-weight': 'bold'
                }
            },

            /* ---- 路径节点 ---- */
            {
                selector: 'node.path-node',
                style: {
                    'background-color': '#ff6b6b',
                    'border-color': '#cc4444',
                    'border-width': 3,
                    'width': 24,
                    'height': 24,
                    'z-index': 10
                }
            },

            /* ---- 路径边高亮 ---- */
            {
                selector: 'edge.path-edge',
                style: {
                    'line-color': '#ff6b6b',
                    'target-arrow-color': '#ff6b6b',
                    'width': 5,
                    'z-index': 10,
                    'label': 'data(weight)',
                    'color': '#ff6b6b',
                    'font-weight': 'bold',
                    'font-size': '11px'
                }
            },

            /* ---- 鼠标悬停 ---- */
            {
                selector: 'node:hover',
                style: { 'border-color': '#ffffff', 'border-width': 3, 'z-index': 99 }
            },
            {
                selector: 'edge:hover',
                style: { 'line-color': '#888888', 'width': 4, 'z-index': 99 }
            }
        ],

        /* 使用计算出的距离比例位置 */
        layout: {
            name: 'preset',
            positions: positions,
            fit: true,
            padding: 60
        },

        /* 交互配置 */
        wheelSensitivity: 0.3,
        minZoom: 0.3,
        maxZoom: 3.0
    });

    // 布局完成后适配视图
    cy.ready(function() {
        cy.fit(undefined, 55);
    });

    /* ---- 事件监听 ---- */
    setupEvents();

    /* 更新图信息 */
    updateGraphInfo();

    /* 填充下拉框 */
    populateSelects();
}

/* ================================================================
 * 第 3 部分：交互事件
 * ================================================================ */

/**
 * 设置 Cytoscape 交互事件
 */
function setupEvents() {
    // 点击节点时在控制台显示信息
    cy.on('tap', 'node', function(evt) {
        const node = evt.target;
        console.log(`[点击] 节点 ${node.id()}: ${node.data('label')}`);
    });

    // 鼠标悬停节点时显示提示
    cy.on('mouseover', 'node', function(evt) {
        const node = evt.target;
        const connectedEdges = node.connectedEdges().length;
        document.getElementById('cyContainer').style.cursor = 'pointer';
    });

    cy.on('mouseout', 'node', function() {
        document.getElementById('cyContainer').style.cursor = 'grab';
    });

    // 布局完成后适配
    cy.on('layoutstop', function() {
        cy.fit(undefined, 60);
    });
}

/**
 * 更新图信息显示
 */
function updateGraphInfo() {
    if (!cy) return;
    const nodeCount = cy.nodes().length;
    const edgeCount = cy.edges().length;
    const quality = window._layoutQuality || '—';
    document.getElementById('graphInfo').textContent =
        `节点: ${nodeCount} | 边: ${edgeCount} | 布局误差: ${quality}%`;
}

/**
 * 填充起点/终点下拉框
 */
function populateSelects() {
    const startSelect = document.getElementById('startSelect');
    const endSelect = document.getElementById('endSelect');

    // 清空（保留默认选项）
    startSelect.innerHTML = '<option value="">-- 请选择起点 --</option>';
    endSelect.innerHTML = '<option value="">-- 请选择终点 --</option>';

    LOCATIONS.forEach((name, idx) => {
        const opt1 = document.createElement('option');
        opt1.value = idx;
        opt1.textContent = `[${idx}] ${name}`;

        const opt2 = document.createElement('option');
        opt2.value = idx;
        opt2.textContent = `[${idx}] ${name}`;

        startSelect.appendChild(opt1);
        endSelect.appendChild(opt2);
    });
}

/* ================================================================
 * 第 4 部分：后端通信 & 路径高亮
 * ================================================================ */

/**
 * 调用后端计算最短路径
 *
 * 工作流程：
 *   1. 获取用户选择的起点/终点
 *   2. 通过 Fetch 调用 Python 桥接服务器
 *   3. 桥接服务器运行 C 后端程序
 *   4. 获取 JSON 结果
 *   5. 在 Cytoscape.js 上高亮最短路径
 *   6. 显示结果卡片
 */
async function calculateRoute() {
    const startSelect = document.getElementById('startSelect');
    const endSelect   = document.getElementById('endSelect');
    const calcBtn     = document.getElementById('calcBtn');

    // ---- 输入校验 ----
    if (!startSelect.value || !endSelect.value) {
        showToast('请选择起点和终点！', 'error');
        return;
    }

    const startIdx = parseInt(startSelect.value);
    const endIdx   = parseInt(endSelect.value);

    if (startIdx === endIdx) {
        showToast('起点和终点相同，距离为 0。', 'error');
        return;
    }

    // ---- UI: 加载状态 ----
    const originalHTML = calcBtn.innerHTML;
    calcBtn.innerHTML = '<span class="loading-spinner"></span> 计算中...';
    calcBtn.disabled = true;

    try {
        // ---- 第 1 步：重置之前的高亮 ----
        resetHighlight(false);

        // ---- 第 2 步：标记起点和终点 ----
        markStartEnd(startIdx, endIdx);

        // ---- 第 3 步：调用后端 API ----
        const apiUrl = `/api/route?start=${startIdx}&end=${endIdx}`;
        console.log(`[请求] ${apiUrl}`);

        const response = await fetch(apiUrl);
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }

        const result = await response.json();
        console.log('[响应]', result);

        // ---- 第 4 步：处理结果 ----
        displayResult(result, startIdx, endIdx);

        // ---- 第 5 步：高亮最短路径 ----
        if (result.status === 'ok' && result.pathIndices && result.pathIndices.length > 0) {
            highlightPath(result.pathIndices);
            showToast(`查询成功！最短距离: ${result.distance}`, 'success');
        } else if (result.status === 'same_node') {
            showToast('起点与终点相同。', 'error');
        } else if (result.status === 'unreachable') {
            showToast('两地之间没有可达路径！', 'error');
        } else {
            showToast('查询失败，请重试。', 'error');
        }

    } catch (err) {
        console.error('[错误]', err);
        showToast('后端通信失败！请确保桥接服务器已启动。', 'error');
    } finally {
        // ---- 恢复按钮 ----
        calcBtn.innerHTML = originalHTML;
        calcBtn.disabled = false;
    }
}

/**
 * 标记起点和终点
 */
function markStartEnd(startIdx, endIdx) {
    if (!cy) return;

    const startNode = cy.$id(String(startIdx));
    const endNode   = cy.$id(String(endIdx));

    if (startNode.length) {
        startNode.addClass('start-node');
    }
    if (endNode.length) {
        endNode.addClass('end-node');
    }
}

/**
 * 高亮最短路径
 *
 * @param {Array<number>} pathIndices  路径顶点索引数组，如 [0, 2, 6, 8]
 */
function highlightPath(pathIndices) {
    if (!cy || !pathIndices || pathIndices.length < 2) return;

    // 对路径上的每对相邻节点，高亮对应边
    for (let i = 0; i < pathIndices.length - 1; i++) {
        const u = String(pathIndices[i]);
        const v = String(pathIndices[i + 1]);

        // 查找连接 u 和 v 的边（无向图，双向都查）
        let edge = cy.$id(`e${u}_${v}`);
        if (edge.length === 0) {
            edge = cy.$id(`e${v}_${u}`);
        }

        if (edge.length > 0) {
            edge.addClass('path-edge');
        }

        // 对路径中间节点添加 path-node 样式（起点/终点保留自己的样式）
        if (i > 0 && i < pathIndices.length - 1) {
            const node = cy.$id(u);
            if (node.length > 0 && !node.hasClass('start-node') && !node.hasClass('end-node')) {
                node.addClass('path-node');
            }
        }
    }

    // 动画：对路径上的边做一个脉冲效果
    animatePathEdges();
}

/**
 * 路径边脉冲动画（视觉提示）
 */
function animatePathEdges() {
    if (!cy) return;

    const pathEdges = cy.edges('.path-edge');
    if (pathEdges.length === 0) return;

    let pulseCount = 0;
    const maxPulses = 3;

    const pulseInterval = setInterval(() => {
        if (pulseCount >= maxPulses) {
            clearInterval(pulseInterval);
            // 恢复最终样式
            pathEdges.style({
                'width': 5,
                'line-color': '#ff6b6b',
                'target-arrow-color': '#ff6b6b'
            });
            return;
        }

        // 脉冲：变粗变亮
        pathEdges.style({
            'width': 8,
            'line-color': '#ff9999',
            'target-arrow-color': '#ff9999'
        });

        setTimeout(() => {
            pathEdges.style({
                'width': 5,
                'line-color': '#ff6b6b',
                'target-arrow-color': '#ff6b6b'
            });
        }, 300);

        pulseCount++;
    }, 600);
}

/**
 * 重置所有高亮样式
 * @param {boolean} clearResult 是否同时清空结果卡片
 */
function resetHighlight(clearResult = true) {
    if (!cy) return;

    // 移除所有特殊样式类
    cy.nodes().removeClass('start-node end-node path-node');
    cy.edges().removeClass('path-edge');

    // 恢复边的默认样式（以防脉冲动画残留）
    cy.edges().style({
        'width': 2.5,
        'line-color': '#3a3d41',
        'target-arrow-color': '#3a3d41'
    });

    if (clearResult) {
        hideResult();
    }
}

/**
 * 显示查询结果卡片
 */
function displayResult(result, startIdx, endIdx) {
    const resultCard = document.getElementById('resultCard');

    document.getElementById('resultDistance').textContent =
        (result.distance !== null && result.distance !== undefined)
            ? String(result.distance) : '不可达';

    if (result.path && result.path.length > 0) {
        document.getElementById('resultPath').textContent =
            result.path.join('  →  ');
    } else {
        document.getElementById('resultPath').textContent = '无路径';
    }

    const statusMap = {
        'ok': '✓ 查询成功',
        'unreachable': '✗ 不可达',
        'same_node': '◎ 同节点',
        'invalid': '✗ 参数无效'
    };
    document.getElementById('resultStatus').textContent =
        statusMap[result.status] || result.status;

    resultCard.style.display = 'block';
    resultCard.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
}

/**
 * 隐藏结果卡片
 */
function hideResult() {
    const resultCard = document.getElementById('resultCard');
    resultCard.style.display = 'none';
}

/* ================================================================
 * 第 5 部分：Toast 通知
 * ================================================================ */

/**
 * 显示临时通知
 * @param {string} message  消息内容
 * @param {string} type     类型: 'success' | 'error'
 */
function showToast(message, type = 'success') {
    // 移除旧 Toast
    const oldToast = document.querySelector('.toast');
    if (oldToast) oldToast.remove();

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.textContent = message;
    document.body.appendChild(toast);

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transition = 'opacity 0.3s ease';
        setTimeout(() => toast.remove(), 300);
    }, 3000);
}

/* ================================================================
 * 第 6 部分：启动初始化
 * ================================================================ */

/**
 * 页面加载完成后初始化
 */
document.addEventListener('DOMContentLoaded', () => {
    console.log('[启动] 地图导航系统前端初始化...');
    console.log(`[数据] ${LOCATIONS.length} 个地点, ${ROADS.length} 条道路`);

    // 初始化 Cytoscape.js
    initCytoscape();

    console.log('[就绪] 系统初始化完成，等待用户操作。');
});
