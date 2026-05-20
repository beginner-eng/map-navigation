#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
============================================================================
 server.py  —  地图导航系统 桥接服务器 (Bridge Server)
============================================================================

 作用:
   1. 提供静态文件服务（前端 HTML/CSS/JS）
   2. 接收前端 AJAX 请求
   3. 调用 C 后端程序计算最短路径
   4. 读取 C 后端生成的 result.json 并返回给前端

 技术栈:
   - Python 3 标准库（http.server, subprocess, json）
   - 零外部依赖，开箱即用

 使用方式:
   python server.py                # 默认端口 8080
   python server.py 3000           # 自定义端口
   python server.py 8080 --debug   # 开启调试模式

 通信流程:
   浏览器 ──GET /api/route?start=0&end=8──▶ Python 服务器
                                            │
                     ┌──────────────────────┘
                     ▼
              执行: ./map_navigation_backend 0 8
                     │
                     ▼
              读取: data/result.json
                     │
                     ▼
   浏览器 ◀─────── JSON 响应 ────────────── Python 服务器
============================================================================
"""

import http.server
import json
import os
import subprocess
import sys
import urllib.parse
from pathlib import Path

# ============================================================
# 配置
# ============================================================

DEFAULT_PORT = 8080
BACKEND_EXE  = "map_navigation_backend"       # C 后端可执行文件名
BACKEND_DIR  = "backend"                       # C 后端所在目录
FRONTEND_DIR = "frontend"                      # 前端静态文件目录
MAP_FILE     = "data/map.txt"                  # 地图数据文件（相对于项目根目录）
RESULT_FILE  = "data/result.json"              # 结果 JSON 文件（相对于项目根目录）

# 项目根目录 = server.py 所在目录的上一级
PROJECT_ROOT = Path(__file__).resolve().parent

DEBUG = False


def log(msg, level="INFO"):
    """简单的彩色日志"""
    colors = {"INFO": "\033[36m", "OK": "\033[32m", "ERR": "\033[31m", "WARN": "\033[33m"}
    reset = "\033[0m"
    c = colors.get(level, "")
    print(f"{c}[{level}]{reset} {msg}")


def find_backend_exe():
    """查找 C 后端可执行文件"""
    backend_dir = PROJECT_ROOT / BACKEND_DIR

    # 尝试多种可能的文件名
    candidates = [
        backend_dir / BACKEND_EXE,
        backend_dir / f"{BACKEND_EXE}.exe",     # Windows
        PROJECT_ROOT / BACKEND_DIR / BACKEND_EXE,
    ]

    for path in candidates:
        if path.is_file():
            return str(path)

    return str(backend_dir / BACKEND_EXE)


def parse_map_file():
    """
    解析 data/map.txt，返回 {"locations": [...], "roads": [[from,to,weight],...]}
    """
    map_path = PROJECT_ROOT / MAP_FILE
    locations = []
    roads = []

    try:
        with open(map_path, 'r', encoding='utf-8') as f:
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
    except FileNotFoundError:
        log(f"地图文件不存在: {map_path}", "ERR")
        return {"locations": [], "roads": []}

    log(f"地图加载完成: {len(locations)} 个地点, {len(roads)} 条道路", "OK")
    return {"locations": locations, "roads": roads}


def run_c_backend(start, end):
    """
    调用 C 后端计算最短路径

    参数:
        start (int): 起点索引
        end   (int): 终点索引

    返回:
        dict: 包含 distance, path, start, end, status 的结果字典
    """
    exe_path = find_backend_exe()
    map_path = str(PROJECT_ROOT / MAP_FILE)
    result_path = str(PROJECT_ROOT / RESULT_FILE)

    # 确保 data/ 目录存在
    (PROJECT_ROOT / "data").mkdir(parents=True, exist_ok=True)

    cmd = [exe_path, str(start), str(end), map_path, result_path]

    if DEBUG:
        log(f"执行命令: {' '.join(cmd)}", "INFO")

    try:
        # 执行 C 后端程序
        proc = subprocess.run(
            cmd,
            cwd=str(PROJECT_ROOT),
            capture_output=True,
            text=True,
            timeout=10  # 10 秒超时
        )

        if DEBUG:
            if proc.stdout:
                log(f"C 后端输出:\n{proc.stdout}", "INFO")
            if proc.stderr:
                log(f"C 后端错误:\n{proc.stderr}", "WARN")

        # 读取生成的 result.json
        result_json_path = PROJECT_ROOT / RESULT_FILE
        if result_json_path.is_file():
            with open(result_json_path, 'r', encoding='utf-8') as f:
                result = json.load(f)
            log(f"查询成功: {start}->{end}, 距离={result.get('distance')}", "OK")
            return result
        else:
            log(f"result.json 未生成 (返回码: {proc.returncode})", "ERR")
            return {
                "distance": None,
                "path": [],
                "pathIndices": [],
                "start": "",
                "end": "",
                "status": "error"
            }

    except subprocess.TimeoutExpired:
        log("C 后端执行超时！", "ERR")
        return {
            "distance": None, "path": [], "pathIndices": [],
            "start": "", "end": "", "status": "timeout"
        }
    except FileNotFoundError:
        log(f"找不到 C 后端可执行文件: {exe_path}", "ERR")
        log("请先编译 C 后端: cd backend && make", "WARN")
        return {
            "distance": None, "path": [], "pathIndices": [],
            "start": "", "end": "", "status": "backend_not_found"
        }
    except json.JSONDecodeError:
        log("result.json 格式错误", "ERR")
        return {
            "distance": None, "path": [], "pathIndices": [],
            "start": "", "end": "", "status": "json_error"
        }


class NavHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """
    自定义 HTTP 请求处理器

    - GET /              → 返回 frontend/index.html
    - GET /api/route?... → 调用 C 后端，返回 JSON
    - GET /*.css|*.js    → 返回对应静态文件
    """

    def __init__(self, *args, **kwargs):
        # 设置静态文件根目录为 frontend/
        super().__init__(*args, directory=str(PROJECT_ROOT / FRONTEND_DIR), **kwargs)

    def do_GET(self):
        """处理 GET 请求"""
        parsed = urllib.parse.urlparse(self.path)

        # ---- API 路由: /api/route ----
        if parsed.path == '/api/route':
            self.handle_api_route(parsed)
            return

        # ---- API 路由: /api/map (地图数据) ----
        if parsed.path == '/api/map':
            self.handle_api_map()
            return

        # ---- 静态文件服务 ----
        super().do_GET()

    def handle_api_route(self, parsed):
        """处理路径查询 API 请求"""
        params = urllib.parse.parse_qs(parsed.query)

        # 获取参数
        try:
            start = int(params.get('start', [None])[0])
            end   = int(params.get('end',   [None])[0])
        except (TypeError, ValueError):
            self.send_json_response(400, {
                "error": "参数无效，需要 start 和 end 整数参数。"
            })
            return

        if start is None or end is None:
            self.send_json_response(400, {
                "error": "缺少参数，需要 start 和 end。"
            })
            return

        log(f"收到查询请求: start={start}, end={end}", "INFO")

        # 调用 C 后端
        result = run_c_backend(start, end)

        # 返回结果
        self.send_json_response(200, result)

    def handle_api_map(self):
        """返回地图数据（地点列表和道路列表）"""
        data = parse_map_file()
        self.send_json_response(200, data)


    def send_json_response(self, status_code, data):
        """发送 JSON 响应"""
        body = json.dumps(data, ensure_ascii=False, indent=2).encode('utf-8')

        self.send_response(status_code)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(body)))
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cache-Control', 'no-cache')
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, format, *args):
        """自定义日志格式"""
        if DEBUG:
            log(f"{self.client_address[0]} - {format % args}", "INFO")
        else:
            # 仅记录 API 请求
            if '/api/' in str(args[0]) if args else '':
                log(f"HTTP {format % args}", "INFO")


def main():
    """启动服务器"""
    global DEBUG

    # 解析命令行参数
    port = DEFAULT_PORT
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            if sys.argv[1] == '--debug':
                DEBUG = True
            else:
                print(f"错误: 无效的端口号 '{sys.argv[1]}'")
                sys.exit(1)

    if '--debug' in sys.argv:
        DEBUG = True

    # 检查 C 后端是否已编译
    exe_path = find_backend_exe()
    if not os.path.isfile(exe_path):
        log(f"警告: C 后端未编译！({exe_path})", "WARN")
        log("请执行: cd backend && make", "WARN")
    else:
        log(f"C 后端就绪: {exe_path}", "OK")

    # 检查前端文件
    frontend = PROJECT_ROOT / FRONTEND_DIR
    if not (frontend / "index.html").exists():
        log(f"警告: 找不到 frontend/index.html", "WARN")

    # 启动 HTTP 服务器
    server = http.server.HTTPServer(('0.0.0.0', port), NavHTTPRequestHandler)

    print()
    print("=" * 55)
    print("  地图导航系统 - 桥接服务器")
    print("=" * 55)
    print(f"  项目路径:   {PROJECT_ROOT}")
    print(f"  前端目录:   {PROJECT_ROOT / FRONTEND_DIR}")
    print(f"  后端程序:   {exe_path}")
    print(f"  监听地址:   http://localhost:{port}")
    print(f"  调试模式:   {'开启' if DEBUG else '关闭'}")
    print("=" * 55)
    print()
    print(f"  \033[32m▶ 请在浏览器中打开: http://localhost:{port}\033[0m")
    print()
    print("  按 Ctrl+C 停止服务器。")
    print()

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n\n服务器已停止。")
        server.server_close()


if __name__ == '__main__':
    main()
