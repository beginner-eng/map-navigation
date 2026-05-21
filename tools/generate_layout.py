#!/usr/bin/env python3
"""
generate_layout.py — 为城市地图生成布局坐标文件

用法: python tools/generate_layout.py maps/<city>.txt [output_path]
      若不指定 output_path，默认输出 data/layouts/<city>.json

算法: 弹簧嵌入 (spring-embedding)
  1. 初始化：节点圆形排列 + 小幅随机扰动
  2. 迭代：
     a) 有边连接的节点产生弹簧力（理想长度 = 权值 * SCALE）
     b) 所有节点对有斥力（防止重叠）
     c) 中心引力（防止漂移）
  3. 收敛或达最大迭代次数后输出
"""

import json
import math
import random
import sys
import os

random.seed(42)  # 固定种子，确保可复现

# ---- 参数 ----
SCALE = 18.0       # 像素/距离单位
SPRING_K = 0.25    # 弹簧刚度
REPULSION = 8000   # 斥力强度
DAMPING = 0.88     # 阻尼
GRAVITY = 0.008    # 中心引力
MAX_ITER = 400     # 最大迭代次数
CONVERGE_EPS = 0.05
DT = 0.6
CX, CY = 450, 340  # 画布中心


def parse_map(filepath):
    """解析 map.txt，返回 (locations, roads)"""
    locations = []
    roads = []
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if line[0] in ('V', 'v'):
                parts = line.split(None, 1)
                if len(parts) >= 2:
                    locations.append(parts[1])
            elif line[0] in ('E', 'e'):
                parts = line.split()
                if len(parts) >= 4:
                    roads.append([int(parts[1]), int(parts[2]), int(parts[3])])
    return locations, roads


def compute_layout(locations, roads):
    """返回 {str(idx): {"x": x, "y": y}} 的字典"""
    n = len(locations)
    if n == 0:
        return {}

    # ---- Floyd-Warshall 全对最短距离 (用于初始化) ----
    INF = float('inf')
    dist = [[INF] * n for _ in range(n)]
    for i in range(n):
        dist[i][i] = 0
    for u, v, w in roads:
        if w < dist[u][v]:
            dist[u][v] = w
            dist[v][u] = w
    for k in range(n):
        for i in range(n):
            for j in range(n):
                if dist[i][k] + dist[k][j] < dist[i][j]:
                    dist[i][j] = dist[i][k] + dist[k][j]

    # 找最远点对作为主轴
    max_d = -1
    pa, pb = 0, 0
    for i in range(n):
        for j in range(i + 1, n):
            if dist[i][j] < INF and dist[i][j] > max_d:
                max_d = dist[i][j]
                pa, pb = i, j

    # 沿主轴初始化
    axis_len = max_d * SCALE if max_d > 0 else 600
    pos_x = [0.0] * n
    pos_y = [0.0] * n
    vel_x = [0.0] * n
    vel_y = [0.0] * n

    for i in range(n):
        da = dist[i][pa] if dist[i][pa] < INF else max_d
        db = dist[i][pb] if dist[i][pb] < INF else max_d
        total = da + db
        if total > 0 and total < INF:
            t = da / total
            pos_x[i] = CX + (t - 0.5) * axis_len
            perp = ((i % 7) - 3) * 35 + random.uniform(-10, 10)
            pos_y[i] = CY + perp
        else:
            pos_x[i] = CX + random.uniform(-100, 100)
            pos_y[i] = CY + random.uniform(-100, 100)

    # ---- 弹簧嵌入迭代 ----
    for it in range(MAX_ITER):
        fx = [0.0] * n
        fy = [0.0] * n

        # 边弹簧力
        for u, v, weight in roads:
            dx = pos_x[v] - pos_x[u]
            dy = pos_y[v] - pos_y[u]
            d = math.sqrt(dx * dx + dy * dy)
            if d < 0.5:
                d = 0.5
                dx = random.uniform(-1, 1)
                dy = random.uniform(-1, 1)
            ideal = weight * SCALE
            force = (d - ideal) * SPRING_K
            fx_u = (dx / d) * force
            fy_u = (dy / d) * force
            fx[u] += fx_u
            fy[u] += fy_u
            fx[v] -= fx_u
            fy[v] -= fy_u

        # 全局斥力
        for i in range(n):
            for j in range(i + 1, n):
                dx = pos_x[j] - pos_x[i]
                dy = pos_y[j] - pos_y[i]
                d = math.sqrt(dx * dx + dy * dy)
                if d < 0.5:
                    d = 0.5
                    dx = random.uniform(-1, 1)
                    dy = random.uniform(-1, 1)
                rep = REPULSION / (d * d)
                f = rep
                fx_i = (dx / d) * f
                fy_i = (dy / d) * f
                fx[i] -= fx_i
                fy[i] -= fy_i
                fx[j] += fx_i
                fy[j] += fy_i

        # 中心引力
        for i in range(n):
            fx[i] += (CX - pos_x[i]) * GRAVITY
            fy[i] += (CY - pos_y[i]) * GRAVITY

        # 更新位置
        max_disp = 0
        for i in range(n):
            vel_x[i] = vel_x[i] * DAMPING + fx[i] * DT
            vel_y[i] = vel_y[i] * DAMPING + fy[i] * DT
            speed = math.sqrt(vel_x[i] ** 2 + vel_y[i] ** 2)
            if speed > 15:
                vel_x[i] = (vel_x[i] / speed) * 15
                vel_y[i] = (vel_y[i] / speed) * 15
            pos_x[i] += vel_x[i] * DT
            pos_y[i] += vel_y[i] * DT
            disp = abs(vel_x[i] * DT) + abs(vel_y[i] * DT)
            if disp > max_disp:
                max_disp = disp

        if max_disp < CONVERGE_EPS and it > 50:
            break

    # 组装结果
    positions = {}
    for i in range(n):
        positions[str(i)] = {"x": round(pos_x[i], 1), "y": round(pos_y[i], 1)}
    return positions


def main():
    if len(sys.argv) < 2:
        print("用法: python tools/generate_layout.py maps/<city>.txt [output_path]")
        sys.exit(1)

    map_path = sys.argv[1]
    if not os.path.exists(map_path):
        print(f"文件不存在: {map_path}")
        sys.exit(1)

    city = os.path.splitext(os.path.basename(map_path))[0]
    out_path = sys.argv[2] if len(sys.argv) > 2 else f"data/layouts/{city}.json"

    locations, roads = parse_map(map_path)
    positions = compute_layout(locations, roads)

    nodes = [{"id": idx, "x": pos["x"], "y": pos["y"]}
             for idx, pos in sorted(positions.items(), key=lambda x: int(x[0]))]
    output = {"nodes": nodes}

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'w', encoding='utf-8') as f:
        json.dump(output, f, ensure_ascii=False, indent=2)
    print(f"[完成] {city}: {len(nodes)} 个节点 → {out_path}")


if __name__ == '__main__':
    main()
