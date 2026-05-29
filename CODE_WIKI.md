# Exile UI (原 Lailloken UI) — Code Wiki

## 1. 项目概述

**Exile UI** 是一款轻量级的 AutoHotkey (AHK) 悬浮窗工具，为 **Path of Exile 1** 和 **Path of Exile 2** 提供丰富的 UI（用户界面）和 QoL（品质体验）功能。项目曾用名为 **Lailloken UI**。

- **语言/平台:** AutoHotkey v1.1 (>=1.1.36, <2.0 64-bit)
- **当前版本:** v1.63.0
- **许可证:** 查看 [LICENSE.md](file:///workspace/LICENSE.md)
- **GitHub 仓库:** [Lailloken/Exile-UI](https://github.com/Lailloken/Exile-UI)
- **Wiki:** https://github.com/Lailloken/Lailloken-UI/wiki

### 1.1 设计理念

- 强调易用性、极简设计、低快捷键需求、与游戏客户端无缝集成
- 通过读取游戏 `client.txt` 日志获取游戏状态
- 通过画面检测（像素/图像搜索）实现上下文感知
- 严格遵循 GGG (Grinding Gear Games) 的第三方工具政策

### 1.2 主要功能模块

| 功能 | 描述 | PoE1 | PoE2 |
|------|------|:----:|:----:|
| **Clone-frames** | 通过克隆/投射屏幕区域实现界面自定义 | ✅ | ✅ |
| **Item-info** | 紧凑可定制的物品信息提示框 | ✅ | ✅ |
| **Act-Tracker** | 战役升级指南/追踪器 | ✅ | ✅ |
| **Act-Decoder** | 战役地图布局提示 | ✅ | ✅ |
| **Stash-Ninja** | poe.ninja 价格悬浮窗 | ✅ | ✅ |
| **Chat Macros** | 快捷聊天宏 / 聊天轮盘 | ✅ | ✅ |
| **Map-Tracker** | 地图数据收集/统计/导出 | ✅ | ✅ |
| **Map-info Panel** | 地图词缀信息面板 | ✅ | ✅ |
| **Sanctum/Sekhema Planner** | 圣殿/试炼规划器 | ✅ | ✅ |
| **Enchant Finder** | 附魔计算器（油/情感） | ✅ | ✅ |
| **FilterSpoon** | 客户端内战利品过滤器编辑器 | ✅ | ✅ |
| **Recombination Simulator** | 重组合成模拟器 | ✅ | |
| **Context-menu** | 物品右键上下文菜单 | ✅ | ✅ |
| **Statlas** | 快速访问的地图集悬浮窗 | | ✅ |
| **Search-strings** | 自定义游戏内搜索字符串 | ✅ | ✅ |
| **Vaal Street** | 货币兑换 / 异步交易管理 | ✅ | ✅ |
| **Cheat-sheet Toolkit** | 自定义上下文感知的速查表 | ✅ | ✅ |
| **TLDR-Tooltips** | 屏幕信息摘要提示框 | ✅ | |
| **Betrayal-info** | 背叛联盟信息面板 | ✅ | |
| **Seed-explorer** | 永恒珠宝探索器 (legacy) | ✅ | |
| **Minor QoL** | 便笺、计时器、实验室追踪等 | ✅ | ✅ |

---

## 2. 项目整体架构

### 2.1 架构总览图

```
┌──────────────────────────────────────────────────────────┐
│                    Exile UI.ahk (主入口)                   │
│  ┌────────────────────────────────────────────────────┐  │
│  │          初始化阶段：依次调用 Init_*()                │  │
│  │  读取配置 → 窗口检测 → 多语言 → 各模块初始化          │  │
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │          运行阶段：定时器驱动                         │  │
│  │  SetTimer Loop, 1000    (1秒循环)                    │  │
│  │  SetTimer Loop_main, 50 (50ms 主循环)                │  │
│  │  SetTimer Log_Loop, 1000 (日志监听循环)              │  │
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │         #Include 26个功能模块 + 3个核心库             │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
         │                    │                    │
         ▼                    ▼                    ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│ _secondary       │ │ _ocr thread     │ │ 外部数据源       │
│ thread.ahk       │ │ .ahk            │ │ • client.txt     │
│ (多线程辅助线程)  │ │ (OCR 识别线程)  │ │ • poe.ninja      │
│                  │ │                 │ │ • PoB 导出码     │
└─────────────────┘ └─────────────────┘ └─────────────────┘
```

### 2.2 架构模式

Exile UI 采用 **单体脚本 + #Include 模块化** 的架构模式：

- **主线程** (`Exile UI.ahk`): 负责初始化、UI 渲染、定时器循环、事件处理
- **辅助线程** (`_secondary thread.ahk`): 处理 CPU 密集型任务，如克隆帧画面捕获
- **OCR 线程** (`_ocr thread.ahk`): 处理 Statlas 的 OCR 文本识别任务

线程间通过 Windows 消息 (`WM_COPYDATA`, `0x004A`) 和 `OnMessage` 进行通信。

### 2.3 目录结构

```
Exile UI/
├── Exile UI.ahk                    # 主脚本入口
├── README.md                       # 项目说明
├── LICENSE.md                      # 许可证
├── .gitattributes                  # Git 属性配置
├── .gitignore                      # Git 忽略规则
│
├── data/                           # 数据目录
│   ├── Class_CustomFont.ahk         #   自定义字体加载类
│   ├── External Functions.ahk       #   GDI+ 库及外部函数
│   ├── JSON.ahk                     #   JSON 解析库
│   ├── Fontin-SmallCaps.ttf         #   默认字体文件
│   ├── Resolutions.ini              #   分辨率相关坐标
│   ├── versions.json                #   版本信息
│   ├── changelog.json               #   更新日志
│   ├── announcements.json           #   公告信息
│   ├── zlib1.dll                    #   zlib 压缩库
│   ├── english/                     #   英文语言数据
│   │   ├── UI.txt                   #     UI 文本
│   │   ├── client.txt               #     客户端文本映射
│   │   ├── [leveltracker] *.json    #     升级追踪器数据
│   │   ├── [sanctum] *.json         #     圣殿数据
│   │   ├── anoints.json             #     附魔数据
│   │   ├── Betrayal.json            #     背叛数据
│   │   ├── TLDR-tooltips.json       #     TLDR 提示框数据
│   │   ├── essences.json            #     精华数据
│   │   ├── help tooltips.json       #     帮助提示框
│   │   ├── maps 2.json              #     地图数据 (PoE2)
│   │   ├── map-info.txt             #     地图信息文本 (PoE1)
│   │   ├── map-info 2.txt           #     地图信息文本 (PoE2)
│   │   ├── mercenaries.json         #     佣兵数据
│   │   └── timeless jewels.json     #     永恒珠宝数据
│   └── global/                      #   全局数据
│       ├── leagues.json             #     赛季数据
│       ├── item mods.json           #     物品词缀数据
│       ├── item bases.json          #     物品基底数据
│       ├── item drop-tiers.json     #     物品掉落等级
│       ├── [stash-ninja] *.json     #     藏身忍者数据
│       ├── [leveltracker] *.json    #     天赋树数据
│       └── [legion] *.csv           #     军团珠宝数据
│
├── modules/                        # 功能模块目录 (26个 .ahk 文件)
│   ├── _functions.ahk              #   核心工具函数库
│   ├── _secondary thread.ahk       #   多线程辅助线程
│   ├── _ocr thread.ahk             #   OCR 识别线程
│   ├── GUI.ahk                     #   GUI 渲染函数
│   ├── hotkeys.ahk                 #   快捷键管理
│   ├── omni-key.ahk                #   万能键功能
│   ├── client log.ahk              #   日志解析
│   ├── screen-checks.ahk           #   画面检测
│   ├── settings menu.ahk           #   设置菜单
│   ├── languages.ahk               #   多语言支持
│   ├── act-decoder.ahk             #   战役解图
│   ├── anoints.ahk                 #   附魔查找
│   ├── betrayal-info.ahk           #   背叛信息
│   ├── cheat sheets.ahk            #   速查表
│   ├── clone-frames.ahk            #   克隆帧
│   ├── exchange.ahk                #   瓦尔街(交易)
│   ├── item-checker.ahk            #   物品信息
│   ├── leveling tracker.ahk        #   升级追踪
│   ├── lootfilter.ahk              #   FilterSpoon
│   ├── macros.ahk                  #   聊天宏
│   ├── map-info.ahk                #   地图信息
│   ├── map tracker.ahk             #   地图追踪
│   ├── ocr.ahk                     #   OCR
│   ├── qol tools.ahk               #   小工具
│   ├── recombination.ahk           #   重组模拟
│   ├── sanctum.ahk                 #   圣殿规划
│   ├── search-strings.ahk          #   搜索字符串
│   ├── seed-explorer.ahk           #   种子探索
│   ├── stash-ninja.ahk             #   藏身忍者
│   └── statlas.ahk                 #   Statlas
│
├── img/                            # 图片资源目录
│   ├── GUI/                        #   GUI 界面图片
│   │   ├── act-decoder/            #     解图相关图片
│   │   ├── anoints/                #     附魔相关图标
│   │   ├── currency/               #     通货图标
│   │   ├── item info/              #     物品信息图标
│   │   ├── leveling tracker/       #     升级追踪图标
│   │   ├── lootfilter/             #     FilterSpoon 图标
│   │   ├── map-info/               #     地图信息图标
│   │   ├── mapping tracker/        #     地图追踪图标
│   │   ├── radial menu/            #     环形菜单图标
│   │   ├── screen-checks/          #     画面检测基准图
│   │   ├── statlas/                #     Statlas 地图截图
│   │   └── *.png (通用GUI图标)     #     关闭/帮助/拖拽等
│   └── readme/                     #   README 展示图片
│
├── ini/                            # 运行时配置文件(运行时生成)
└── exports/                        # 导出数据目录(运行时生成)
```

---

## 3. 主入口文件: Exile UI.ahk

[Exile UI.ahk](file:///workspace/Exile UI.ahk) 是整个工具的入口，负责：

### 3.1 脚本元配置 (第1-13行)

```autohotkey
#NoEnv
#SingleInstance Force
#Requires AutoHotkey >=1.1.36 <2 64-bit
#InstallKeybdHook
#InstallMouseHook
#MaxThreads 255
#MaxMem 1024
#Include data\Class_CustomFont.ahk
#Include data\External Functions.ahk
#Include data\JSON.ahk
```

- 强制单实例运行
- 要求 64 位 AHK v1.1.36+
- 安装键盘/鼠标钩子
- 引入三个核心库

### 3.2 启动流程

```
1. 等待游戏客户端窗口出现 (POEWindowClass 或 GeForceNOW)
2. CheckClient() → 判断 PoE1 还是 PoE2
3. 版本更新检查 (UpdateCheck)
4. Init_vars() → 初始化全局变量对象
5. Startup() → 启动配置、GDI+、窗口检测、日志文件定位、辅助线程
6. Init_*() × 20+ → 依次初始化各功能模块
7. Resolution_check() → 检查分辨率兼容性
8. SetTimer Loop / Loop_main → 启动主循环
```

### 3.3 主循环

- **Loop** (1000ms): 检查游戏窗口存在性、版本更新检查、工具崩溃检测
- **Loop_main** (50ms): 核心渲染循环，处理所有 GUI 交互、画面检测、鼠标悬停检测
- **Log_Loop** (1000ms): 监听 client.txt 日志变化

### 3.4 退出处理 (Exit 标签)

[Exit](file:///workspace/Exile UI.ahk#L147-L168) 函数在脚本退出时：
- 关闭 GDI+ 
- 关闭日志文件
- 通知辅助线程退出
- 保存缓存状态（背叛面板、升级计时器等）

---

## 4. 核心库与依赖

### 4.1 内部库

#### 4.1.1 Core Utilities: `_functions.ahk`

[modules/_functions.ahk](file:///workspace/modules/_functions.ahk) 是整个项目的基础工具函数库，所有模块都依赖它。

**关键函数：**

| 函数 | 说明 |
|------|------|
| `Blank(var)` | 判断变量是否为空 |
| `CheckClient()` | 检测是 PoE1 还是 PoE2 |
| `DB_Load(database)` | 通用数据库加载函数 |
| `Init_client()` | 初始化客户端配置（分辨率、窗口位置等） |
| `Init_general()` | 初始化通用设置、功能开关 |
| `Init_vars()` | 初始化全局变量结构 |
| `Loop()` | 1秒循环定时器 |
| `Loop_main()` | 50ms 主循环定时器 |
| `MouseHover()` | 持续追踪鼠标位置与悬停窗口 |
| `Startup()` | 启动配置函数 |
| `Resolution_check()` | 分辨率兼容性检查 |
| `Economy_Update()` | 获取 poe.ninja 价格数据 |
| `News()` | 公告/新闻获取 |
| `LLK_Log(msg)` | 写日志 |
| `LLK_Error(msg)` | 错误提示与退出 |
| `LLK_IniRead()` | 增强版 IniRead |
| `LLK_FileRead()` | 增强版 FileRead |
| `LLK_FilePermissionError()` | 文件权限错误处理 |

**核心全局数据结构：**

| 变量 | 说明 |
|------|------|
| `vars` | 全局运行时变量对象（窗口句柄、像素/图像搜索数据、各模块状态等） |
| `settings` | 全局设置对象（功能开关、各模块配置） |
| `db` | 全局数据库对象（缓存加载的 JSON 数据） |
| `json` | JSON 解析实例 |

#### 4.1.2 CustomFont: `Class_CustomFont.ahk`

[data/Class_CustomFont.ahk](file:///workspace/data/Class_CustomFont.ahk) - 自定义字体加载类，无需系统安装字体即可从文件或资源加载。

```autohotkey
Class CustomFont {
    __New(FontFile, FontName="", FontSize=30)
    AddFromFile(FontFile)     -- 从文件加载字体
    AddFromResource(...)      -- 从资源加载字体
    ApplyTo(hCtrl)            -- 应用到控件
    __Delete()                -- 销毁时移除字体资源
}
```

#### 4.1.3 JSON: `JSON.ahk`

[data/JSON.ahk](file:///workspace/data/JSON.ahk) - 基于 [cocobelgica/AutoHotkey-JSON](https://github.com/cocobelgica/AutoHotkey-JSON) 的 JSON 解析库。

```autohotkey
Class JSON {
    Load(text [, reviver])    -- 解析 JSON 字符串
    Dump(value [, replacer])  -- 将 AHK 对象转为 JSON 字符串
}
```

#### 4.1.4 GDI+ Library: `External Functions.ahk`

[data/External Functions.ahk](file:///workspace/data/External Functions.ahk) - 整合了以下第三方 GDI+ 函数：

- **Gdip_All.ahk**: GDI+ 图形渲染（由 marius-sucan 维护的社区编译版 v1.96）
- **Gdip_ImageSearch**: 图像搜索功能
- **OCR with UWP API**: Windows OCR 文本识别
- **base64 decode**: Base64 解码（用于 PoB 导出码）
- **zlib wrapper**: zlib 解压（用于 PoB 导出码）

### 4.2 外部依赖

| 依赖 | 来源 | 用途 |
|------|------|------|
| AutoHotkey v1.1.36+ | 官方 | 脚本运行时 |
| GDI+ | 系统组件 | 图形渲染 |
| zlib1.dll | 系统/内置 | PoB 导出码解压 |
| UWP OCR API | Windows 10/11 | Statlas OCR 文字识别 |
| poe.ninja API | 在线 | 通货/物品价格数据 |
| GitHub API | 在线 | 版本检查与公告 |

### 4.3 模块间依赖关系图

```
                    Exile UI.ahk (主入口)
                          │
           ┌──────────────┼──────────────┐
           ▼              ▼              ▼
     _functions.ahk   JSON.ahk    Class_CustomFont.ahk
     External Functions.ahk (GDI+)
           │
     ┌─────┴─────┬──────────┬──────────┬───────────┐
     ▼           ▼          ▼          ▼           ▼
  GUI.ahk   hotkeys.ahk  omni-key.ahk  languages.ahk  settings menu.ahk
     │           │          │          │           │
     │     ┌─────┴──────────┴──────────┴───────────┴─────┐
     │     │          screen-checks.ahk                   │
     │     │          client log.ahk                      │
     │     └──────────────────┬───────────────────────────┘
     │                        ▼
     │     ┌──────────────────┴───────────────────────────┐
     │     │  功能模块 (按需初始化，通过 settings.features 控制) │
     │     ├─── clone-frames.ahk                          │
     │     ├─── item-checker.ahk                          │
     │     ├─── leveling tracker.ahk                      │
     │     ├─── act-decoder.ahk                           │
     │     ├─── stash-ninja.ahk                           │
     │     ├─── macros.ahk                                │
     │     ├─── map tracker.ahk                           │
     │     ├─── map-info.ahk                              │
     │     ├─── sanctum.ahk                               │
     │     ├─── anoints.ahk                               │
     │     ├─── lootfilter.ahk                            │
     │     ├─── recombination.ahk                         │
     │     ├─── cheat sheets.ahk                          │
     │     ├─── exchange.ahk                              │
     │     ├─── search-strings.ahk                        │
     │     ├─── qol tools.ahk                             │
     │     ├─── ocr.ahk                                   │
     │     ├─── betrayal-info.ahk                         │
     │     ├─── seed-explorer.ahk                         │
     │     └─── statlas.ahk (PoE2 only)                   │
     └──────────────────┬──────────────────────────────────┘
                        ▼
     ┌──────────────────┴──────────────────────────────────┐
     │   辅助线程 (_secondary thread.ahk)                    │
     │   OCR线程 (_ocr thread.ahk)                         │
     └─────────────────────────────────────────────────────┘
```

---

## 5. 基础设施模块详解

### 5.1 GUI 渲染: `GUI.ahk`

[modules/GUI.ahk](file:///workspace/modules/GUI.ahk) 提供通用 GUI 渲染工具函数。

| 函数 | 说明 |
|------|------|
| `Gui_CheckBounds(ByRef x, ByRef y, w, h)` | 确保 GUI 窗口不超出屏幕边界 |
| `Gui_ClientFiller(mode)` | 创建黑色全屏 GUI 填充自定义分辨率下的空白区域 |
| `Gui_CreateGraph(width, height, graph, color)` | 使用 GDI+ 绘制折线图 |
| `Gui_DropDownList(object, coord_array)` | 创建下拉列表控件 |
| `LLK_PanelDimensions()` | 计算面板尺寸 |
| `LLK_Overlay()` | 通用悬浮窗创建/显示/隐藏/销毁 |
| `LLK_ToolTip()` | 通用提示框创建 |
| `LLK_FontDimensions()` | 计算字体尺寸 |
| `LLK_FontDefault()` | 获取默认字体大小 |
| `LLK_ImageCache()` | 图片缓存加载 |
| `Gui_MenuWidget()` | 环形菜单小部件 |

### 5.2 快捷键管理: `hotkeys.ahk`

[modules/hotkeys.ahk](file:///workspace/modules/hotkeys.ahk) 管理所有全局和上下文敏感快捷键。

**核心变量结构：**

```autohotkey
settings.hotkeys := {
    omnikey: "capslock",       -- 万能键
    omnikey2: "",              -- 第二万能键
    emergencykey: "space",     -- 紧急退出键
    movekey: "lbutton",        -- 移动键
    tab: "tab",                -- Tab 键替换
    tabblock: 0,               -- 阻止 Tab 键原生功能
}
```

| 函数 | 说明 |
|------|------|
| `Init_hotkeys()` | 读取配置、绑定所有热键 |
| `Hotkeys_Convert(key)` | 将用户输入的热键字符串转换为 AHK 内部扫描码 |
| `Omnikey()` | 万能键主处理函数 |
| `Omnikey2()` | 第二万能键处理函数 |
| `Hotkeys_Tab()` | Tab 键的自定义行为 |

### 5.3 万能键: `omni-key.ahk`

[modules/omni-key.ahk](file:///workspace/modules/omni-key.ahk) 实现"万能键"——根据当前游戏上下文智能切换功能的统一热键入口。

**上下文检测逻辑：**
- 鼠标悬停在物品上 → 打开物品右键上下文菜单 / 物品信息
- 打开地图页面 → 地图信息面板
- 打开天赋树 → 搜索字符串
- 打开圣殿 → 圣殿规划器
- 打开 stash → FilterSpoon / Stash-Ninja
- 打开交换界面 → Vaal Street 交易工具

### 5.4 画面检测: `screen-checks.ahk`

[modules/screen-checks.ahk](file:///workspace/modules/screen-checks.ahk) 通过像素搜索和图像搜索检测当前游戏界面状态。

**检测方法：**

| 方法 | 函数 | 用途 |
|------|------|------|
| 像素搜索 | `Screenchecks_PixelSearch(type)` | 检测游戏画面、背包、关闭按钮等 |
| 图像搜索 | `Screenchecks_ImageSearch(type)` | 检测天赋树、圣殿、交换、异步交易等 UI |

**检测目标：**
- `gamescreen` - 是否在游戏主界面（非菜单/加载界面）
- `inventory` - 背包是否打开
- `close_button` - 关闭按钮是否可见
- `skilltree` - 天赋树是否打开
- `betrayal` - 背叛面板是否打开
- `atlas` - 地图集是否打开 (PoE2)
- `sanctum` - 圣殿界面是否打开
- `exchange` - 货币交换界面是否打开
- `stash` - 存储箱是否打开
- `async1/async2` - 异步交易界面

### 5.5 日志解析: `client log.ahk`

[modules/client log.ahk](file:///workspace/modules/client log.ahk) 读取并解析 Path of Exile 的 `client.txt` 日志文件。

| 函数 | 说明 |
|------|------|
| `Init_log()` | 定位日志文件、搜索最近的角色信息和区域切换 |
| `Log_Backup()` | 备份日志解析状态 |
| `Log_Parse()` | 解析日志行，提取 区域ID/名称/种子/等级/阶层/章节/时间 等 |
| `Log_CharacterInfo()` | 从日志中提取角色信息 |
| `Log_Loop()` | 定期检查日志文件更新 |
| `Log_LineCheck()` | 检查新日志行内容 |

**解析数据流：**
```
client.txt → Log_Parse() → vars.log {
    areaID, areaname, areaseed, arealevel,
    areatier, act, level, date_time,
    character_class
}
```

### 5.6 多语言: `languages.ahk`

[modules/languages.ahk](file:///workspace/modules/languages.ahk) 支持多语言界面的翻译系统。

| 函数 | 说明 |
|------|------|
| `Init_Lang()` | 加载语言文件 (UI.txt + client.txt) |
| `Lang_Load(file)` | 解析语言文本文件 |
| `Lang_Trans(key)` | 根据 key 获取翻译文本 |
| `Lang_Match(text, array)` | 多语言文本匹配 |
| `Lang_Trim(text, array)` | 多语言文本裁剪 |

### 5.7 设置菜单: `settings menu.ahk`

[modules/settings menu.ahk](file:///workspace/modules/settings menu.ahk) 提供全局统一的设置管理界面。

| 函数 | 说明 |
|------|------|
| `Settings_menu(feature)` | 打开指定功能的设置界面 |
| `Settings_Save(feature)` | 保存设置到 INI 文件 |

---

## 6. 功能模块详解

### 6.1 Clone-frames (`clone-frames.ahk`)

通过克隆并投射游戏画面区域来定制界面布局——例如将 buff 栏、冷却计时器等元素重新定位。

```
工作流程：
1. 用户在游戏中拖拽选择区域
2. 辅助线程持续捕获该区域的像素
3. 主线程在自定义位置渲染克隆的像素画面
4. 支持多帧、缩放、透明度调整
```

### 6.2 Item-info (`item-checker.ahk`)

紧凑型物品信息提示框，在物品悬停时显示关键属性摘要。

```
工作流程：
1. 检测鼠标悬停在物品上
2. 发送 Ctrl+C 复制物品信息
3. 解析剪贴板中的物品属性文本
4. 在游戏内渲染自定义提示框
5. 高亮显示重要属性（抗性、生命、伤害等）
```

### 6.3 Act-Tracker (`leveling tracker.ahk`)

战役升级指南追踪器，提供自动化引导、PoB 天赋树叠加层、宝石设置叠加层。

```
功能组成：
- "指南"：按区域显示推荐任务和物品
- "宝石"：显示宝石获取位置和推荐设置
- "天赋树"：从 PoB 导入并叠加显示天赋树
- "计时器"：记录跑图/区域用时
- "动态角色追踪"：自动跟随角色切换
```

### 6.4 Act-Decoder (`act-decoder.ahk`)

战役地图布局解码器，通过画面提示帮助识别区域布局方向。

```
工作原理：
1. 检测当前区域（从 client.txt 日志）
2. 加载该区域的布局图片数据库
3. 根据画面特征匹配当前布局变体
4. 显示布局指示器/全图叠加层
```

### 6.5 Stash-Ninja (`stash-ninja.ahk`)

poe.ninja 价格覆盖层，在存储箱中显示物品价格标签。

```
数据流：
1. Economy_Update() 定时从 poe.ninja 获取价格数据
2. 数据缓存到 data/global/[stash-ninja] prices.ini
3. 检测物品悬停，匹配价格数据
4. 渲染价格标签/趋势图
```

### 6.6 Sanctum/Sekhema Planner (`sanctum.ahk`)

圣殿（PoE1 Sanctum / PoE2 试炼）规划器。

```
功能：
- 楼层扫描器：读取游戏画面识别房间和路径
- 交互规划器：规划最优路径
- 遗物管理器：管理遗物库存
- 画像识别：自动检测当前楼层布局
```

### 6.7 FilterSpoon (`lootfilter.ahk`)

游戏内战利品过滤器编辑器，在悬停物品时显示匹配的过滤规则。

### 6.8 Map-Tracker (`map tracker.ahk`)

地图数据追踪器，收集、保存、查看和导出地图运行数据。

```
追踪数据：
- 击杀数 (kill-count)
- 死亡数
- 地图词缀
- 笔记
- Boss 信息
- 赛季机制
- 战利品统计
```

### 6.9 Vaal Street (`exchange.ahk`)

货币交换与异步交易工具。

```
功能：
- 货币交换比例计算器
- 可选交易日志记录
- 可选余额追踪
- 快速改价功能
- 销售追踪/日志
- 购买日志
```

### 6.10 其他功能模块

| 模块 | 文件 | 简要说明 |
|------|------|----------|
| Enchant Finder | `anoints.ahk` | 附魔计算器，支持油/情感输入和正则过滤 |
| Betrayal-info | `betrayal-info.ahk` | 背叛联盟信息面板，含图像识别自动追踪 |
| Cheat-sheets | `cheat sheets.ahk` | 自定义上下文感知速查表覆盖层 |
| Chat Macros | `macros.ahk` | 聊天宏和聊天轮盘 |
| Map-info | `map-info.ahk` | 地图词缀信息面板（简化版地图词缀提示） |
| OCR | `ocr.ahk` | OCR 文字识别，用于 TLDR 提示框和地图词缀读取 |
| Search-strings | `search-strings.ahk` | 自定义游戏内搜索字符串快捷菜单 |
| Recombination | `recombination.ahk` | 重组合成结果模拟器 |
| QoL Tools | `qol tools.ahk` | 闹钟、便笺、实验室追踪、倒计时、地图事件通知 |
| Seed-explorer | `seed-explorer.ahk` | 永恒珠宝（军团珠宝）探索器 (legacy) |
| Statlas | `statlas.ahk` | PoE2 专属：地图集覆盖层，显示地图布局、Boss、统计信息 |

---

## 7. 数据流与通信机制

### 7.1 游戏状态感知

```
┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  client.txt      │────▶│  client log.ahk  │────▶│  vars.log.*      │
│  (日志文件)       │     │  Log_Parse()     │     │  (区域/等级/角色)  │
└──────────────────┘     └──────────────────┘     └──────────────────┘

┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  游戏画面         │────▶│  screen-checks   │────▶│  vars.pixels.*   │
│  (像素/图像)      │     │  .ahk            │     │  vars.imagesearch│
└──────────────────┘     └──────────────────┘     └──────────────────┘

┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  剪贴板           │────▶│  item-checker    │────▶│  物品提示框       │
│  (Ctrl+C 复制)   │     │  .ahk            │     │                   │
└──────────────────┘     └──────────────────┘     └──────────────────┘
```

### 7.2 线程间通信

```
主线程 (Exile UI.ahk)
  │                              ▲
  │ StringSend(json_string)      │ OnMessage(0x004A, "StringReceive")
  │ (WM_COPYDATA)                │
  ▼                              │
辅助线程 (_secondary thread.ahk) ─┘

主线程 ←── OnMessage(0x004A) ←── OCR线程 (_ocr thread.ahk)
```

通信使用 Windows `WM_COPYDATA` 消息，数据以 JSON 字符串格式传输。

### 7.3 外部数据源

```
poe.ninja API ──▶ Economy_Update() ──▶ vars.economy.* ──▶ Stash-Ninja / Vaal Street

GitHub API ──▶ News() / UpdateCheck() ──▶ vars.news / vars.update

PoB 导出码 ──▶ base64 decode → zlib decompress → JSON parse → 天赋树导入
```

---

## 8. 配置系统

### 8.1 INI 配置文件

所有用户配置以 INI 格式存储在 `ini/` 目录下：

| 文件 | 用途 |
|------|------|
| `ini/config.ini` | 全局设置（通用跨 PoE 版本设置） |
| `ini 2/config.ini` | PoE2 版本的全局设置 |
| `ini{ 2}/screen checks ({h}p).ini` | 画面检测坐标 |
| `ini{ 2}/hotkeys.ini` | 快捷键配置 |
| `ini{ 2}/clone frames.ini` | 克隆帧配置 |
| `ini{ 2}/item-checker.ini` | 物品信息配置 |
| `ini{ 2}/leveling tracker.ini` | 升级追踪器配置 |
| `ini{ 2}/map tracker.ini` | 地图追踪器配置 |
| `ini{ 2}/vaal street.ini` | 交易功能配置 |
| `ini{ 2}/stash-ninja.ini` | Stash-Ninja 配置 |
| `ini{ 2}/sanctum planner.ini` | 圣殿规划器配置 |
| `ini{ 2}/anoints.ini` | 附魔功能配置 |
| `ini{ 2}/betrayal info.ini` | 背叛信息配置 |
| `ini{ 2}/lootfilter.ini` | FilterSpoon 配置 |
| `ini{ 2}/cheat-sheets.ini` | 速查表配置 |
| `ini{ 2}/geforce now.ini` | GeForce NOW 配置 |
| `data/global/[stash-ninja] prices.ini` | poe.ninja 价格缓存 |

### 8.2 版本兼容性

项目通过 `poe_version` 变量（空字符串 = PoE1，`" 2"` = PoE2）来区分两个游戏版本的配置和数据文件，多数文件路径使用模板 `"ini" vars.poe_version "\..."` 来动态切换。

### 8.3 分辨率系统

[data/Resolutions.ini](file:///workspace/data/Resolutions.ini) 定义了从 720p 到 2160p 的支持分辨率及其对应的画面检测坐标和字号。

---

## 9. 项目运行方式

### 9.1 环境要求

- **操作系统**: Windows 10/11 (64-bit)
- **AutoHotkey**: v1.1.36 或更高版本 (64-bit) — [下载](https://www.autohotkey.com/)
- **Path of Exile**: 1 或 2，窗口化全屏模式
- **网络**: 部分功能需要网络连接（价格获取、版本检查）

### 9.2 安装

1. 下载最新版本: https://github.com/Lailloken/Lailloken-UI/releases
2. 解压到任意文件夹
3. 运行 `Exile UI.ahk`（确保已安装 AutoHotkey）
4. 脚本启动前先打开游戏客户端

### 9.3 启动过程

```
1. 启动 Exile UI.ahk
2. 脚本等待检测到 PoE 游戏窗口
3. 自动检测 PoE1 / PoE2 版本
4. 读取客户端配置文件，检测分辨率
5. 初始化各功能模块
6. 开始运行（托盘图标可见）
7. 右键托盘图标 → Settings 打开设置菜单
```

### 9.4 关键快捷键

| 功能 | 默认快捷键 |
|------|-----------|
| 万能键（上下文操作） | CapsLock |
| 紧急退出/重置 | Ctrl+Alt+Space |
| 环形菜单 | 悬停在托盘图标区域 |

### 9.5 调试/开发模式

在 `ini/config.ini` 中设置：
```ini
[Settings]
dev=1
```

---

## 10. 命名约定与代码风格

### 10.1 函数命名

| 前缀/模式 | 含义 | 示例 |
|-----------|------|------|
| `LLK_*` | 通用工具函数（原 Lailloken 缩写） | `LLK_Log()`, `LLK_IniRead()` |
| `Gui_*` | GUI 相关函数 | `Gui_ClientFiller()`, `Gui_DropDownList()` |
| `Init_*` | 初始化函数 | `Init_vars()`, `Init_hotkeys()` |
| `*_Thread()` | 线程相关函数 | `Cloneframes_Thread()` |
| `*_Search()` | 搜索函数 | `Screenchecks_PixelSearch()` |
| `*_Check()` | 检查函数 | `Resolution_check()` |
| `*_Hover()` | 鼠标悬停处理 | `MouseHover()`, `Legion_Hover()` |

### 10.2 变量命名

| 前缀 | 含义 | 示例 |
|------|------|------|
| `vars.` | 全局运行时状态变量 | `vars.client`, `vars.pixels`, `vars.hwnd` |
| `settings.` | 用户配置变量 | `settings.features`, `settings.general` |
| `db.` | 数据库缓存 | `db.leveltracker`, `db.item_mods` |
| `hwnd` | 窗口句柄变量 | `hwnd_gui`, `hwnd_ddl` |
| `ini.*` | 临时 INI 读取结果 | `ini.settings`, `ini.features` |

### 10.3 代码风格

- 所有函数声明使用 `{` 与函数名同行
- 使用 `local` 声明局部变量作用域
- 使用 `global` 声明引用的全局变量
- 布尔值使用 `1`/`0` 表示
- 大量使用 AHK 内置三元逻辑 `condition ? true_val : false_val`
- 使用 `LLK_CloneObject()` 深拷贝对象
- 使用 `LLK_HasVal()` 检查对象是否包含某值
- 使用 `LLK_PatternMatch()` 进行模式匹配
- 使用 `LLK_IsBetween()` 进行区间判断

---

## 11. 已停用的老旧功能

| 功能 | 说明 |
|------|------|
| Archnemesis Recipe Scanner | 赛季结束后退役 |
| Delve-helper | 地下挖矿助手 |
| Necropolis Lantern Highlighting | 赛末退役 |
| Overlayke (Lake of Kalandra) | 赛季结束后退役 |
| Sanctum-room tooltip overlays | 被新版 Sanctum Planner 取代 |

---

## 12. 安全与合规说明

- **不修改游戏内存**: 仅读取 client.txt 日志文件和画面像素
- **不注入**: 仅通过标准按键操作（发送按键）与游戏交互
- **遵守 GGG 第三方工具政策**: https://www.pathofexile.com/developer/docs/index#policy
- **不使用自动化**: 所有操作都需要用户主动触发
- **不读取游戏数值**: 不检查生命值、魔力值等游戏数据

---

*文档生成时间: 2026-05-29 | 项目版本: v1.63.0*