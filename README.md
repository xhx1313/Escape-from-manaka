# SecretFlasherManaka: Escape from Manaka

面向 Windows x64、Unity IL2CPP 游戏 `SecretFlasherManaka` 的原生扩展。
项目将 `winhttp.dll` 作为代理加载，不修改 `GameAssembly.dll`，也不依赖
BepInEx、MelonLoader 等插件框架。

> 本 MOD 与特定游戏版本强绑定。游戏本体更新后，类名、方法签名、字段偏移和
> 场景布局都需要重新核对。

## 安装

1. 关闭游戏。
2. 运行 `SecretFlasherManaka.EscapeFromManaka.Setup.exe`。
3. 选择包含以下文件的游戏目录：

```text
SecretFlasherManaka.exe
UnityPlayer.dll
GameAssembly.dll
SecretFlasherManaka_Data/
```

4. 点击 `Install / Update`。
5. 安装完成后直接启动 `SecretFlasherManaka.exe`。

安装器会：

- 备份并安装 `winhttp.dll`；
- 安装隐藏更新宿主 `EscapeFromManaka/UpdateHost.exe`；
- 安装完整装备、地图、音频、字体和版本资源；
- 安装 `EscapeFromManaka/uninstall.exe`；
- 删除旧版公开启动器，不创建桌面快捷方式。

游戏运行时不能替换 `winhttp.dll`。安装或更新前必须关闭游戏。

## 开始游戏

1. 启动 `SecretFlasherManaka.exe`。
2. 在标题页点击 `Escape from Manaka`。
3. 如果这是全新游戏环境，MOD 会通过原版流程创建默认手动存档槽 0。
4. 进入黑房仓库后，可整理装备、选择场景并开始撤离。

MOD 使用原游戏存档作为角色来源，但装备、背包、安全箱、仓库、快捷栏和展览柜
保存在独立的 MOD 库存文件中。

## 更新

### 用户频道

普通用户始终直接启动 `SecretFlasherManaka.exe`，使用公开的 `stable` 频道。

点击标题页 `Escape from Manaka` 时，MOD 会在进入仓库前检查更新：

- 无更新：正常进入；
- 有更新：显示版本、文件数和下载大小；
- 点击更新：显示下载进度；
- 下载完成后：自动关闭游戏、替换文件并重新启动；
- 点击稍后：继续使用当前版本。

更新清单和文件均会校验：

- TLS 证书固定指纹；
- 频道签名和 `keyId`；
- 发布序号；
- 游戏版本哈希；
- 每个文件的 SHA-256。

同频道默认只升不降。stable 与 dev 之间切换时，会明确提示目标频道。

### 开发频道

开发人员使用 `Start-Dev.exe`：

1. 将 `Start-Dev.exe` 放到游戏目录，与 `SecretFlasherManaka.exe` 同级。
2. 双击运行。
3. 首次使用选择一个只包含开发 Token 的文本文件。
4. 程序使用当前 Windows 用户的 DPAPI 加密保存 Token。
5. 点击“启动开发频道”进入 dev。

`Start-Dev.exe` 和开发 Token 不属于普通用户安装内容。直接启动游戏不会进入
开发频道。

## 黑房仓库

- 使用原存档角色模型、骨架、Animator 和服装系统。
- 展示模型支持拖动旋转和动作切换。
- 装备、背包、安全箱和仓库之间支持拖放。
- 装备变化立即写入当前槽位的 MOD 库存文件。
- 提供 `1` / `2` / `3` 三个成人用品快捷格。
- 快捷格绑定实际物品，不会复制物品。

### 展览柜

展览柜按装备系列和类别显示物品，支持：

- 默认排序；
- 放入时间正序/倒序；
- 价值正序/倒序；
- 磨损度正序/倒序。

未放入物品的展格显示灰色蒙版。红色已鉴定物品可放入展览柜；同一物品定义最多
收藏一件。点击展格可选择同类红色已鉴定物品进行替换，原展品会返回库存。

馆藏值用于展览柜等级、安全箱容量和鉴定品质概率计算。安全箱最多扩展到
`3x4`，等级详情可从展览柜进度条打开。

### 安全箱

安全箱容量会同步到局内和仓库界面。装备栏、背包栏、安全箱栏使用自适应正方形
格子，并按可用空间调整尺寸。

## 局内玩法

- 使用原游戏 `PlayerController`、`PlayerLocomotion` 和 `InputResolver`。
- 使用原游戏场景和场景切换流程。
- 圆形小地图只显示玩家附近和本局可见信息。
- `M` 打开楼层大地图，支持缩放、拖动、楼层切换和点位追踪。
- 地图点支持物资、撤离、转移和任务随机点。
- 服装店和购物中心可加载撤离点配置。
- 支持搜索物资、任务、奖励物资、撤离和转移。
- 检测到原版任务失败界面后，自动结束本局并返回黑房。

## 控制

| 按键 | 功能 |
|---|---|
| `W` / `A` / `S` / `D` | 移动 |
| `Shift` | 冲刺 |
| `C` | 蹲下 |
| `Q` | 切换大衣/露出状态 |
| `E` | 动作菜单 |
| `F` | 交互 |
| `T` | 搭讪 |
| `R` | 切换视角 |
| `M` | 小地图楼层大地图 |
| `Tab` | 装备/背包/物资栏 |
| `F1` | MOD 玩法指南 |
| `` ` `` | 原游戏动作列表 |
| `F5` | 完整装备目录预览 |
| `F6` | 批量导出装备截图 |
| `N` | 地图点标注模式 |
| `B` | 地图点编辑 |
| `Esc` | 关闭当前界面或打开原版系统菜单 |

MOD 模式下，原版系统菜单中的“教程”会打开独立玩法指南，不修改原版教程内容或
已读状态。

## 文件与存档

安装后的主要内容：

```text
winhttp.dll
winhttp_orig.dll
EscapeFromManaka/
  Assets/
  Audio/
  Checkpoints/
  Maps/
  Fonts/
  Logs/
  UpdateHost.exe
  Update/
  installed.json
  LauncherState.json
  uninstall.exe
```

MOD 库存和展览柜存档：

```text
%USERPROFILE%\AppData\LocalLow\SheableSoft\SecretFlasherManaka\
  SaveData\EscapeFromManaka\inventory_<槽位>.json
```

安装器的 `--install-silent` 参数：

```powershell
Start-Process .\SecretFlasherManaka.EscapeFromManaka.Setup.exe `
  -ArgumentList @('--install-silent', '<游戏目录>') `
  -Wait
```

## 卸载

1. 关闭游戏。
2. 运行 `EscapeFromManaka/uninstall.exe`。

卸载会移除代理、更新宿主、安装资源和日志，但不会删除 MOD 库存文件。完全清理时
再手动删除：

```text
%USERPROFILE%\AppData\LocalLow\SheableSoft\SecretFlasherManaka\
  SaveData\EscapeFromManaka
```

## 日志与排查

### 原生 MOD 日志

```text
<游戏目录>\EscapeFromManaka\Logs\ManakaMultiplayer.Native.log
```

每次游戏启动会重新创建，只保留当前进程会话。日志由独立写线程批量写入，
正常退出时排空队列并写入 `SessionStop`。

### 更新日志

```text
<游戏目录>\EscapeFromManaka\Logs\UpdateHost.log
```

记录频道、清单版本、待下载文件、下载、校验、备份、替换和重启。更新失败时先检查
此文件。

### 安装错误日志

静默安装失败时会生成：

```text
ManakaMultiplayer.Setup.error.log
```

后续安装会将其迁移到 `EscapeFromManaka/Logs`。

常见排查：

- 标题页没有入口：检查游戏版本和 `ManakaMultiplayer.Native.log` 中的 Hook 初始化。
- 地图卡死或 Esc 穿透：检查 `UI`、`INPUT`、`LIFECYCLE` 分类中的关闭和原版菜单日志。
- 更新下载失败：检查 `UpdateHost.log` 中的频道、对象 URL 路径和 HTTP 状态。
- 更新后重复提示同一文件：检查正式宿主哈希和清单中的 `UpdateHost.new.exe` 哈希。
- 更新后回到用户频道：直接启动游戏属于 `stable`，需要 dev 时使用 `Start-Dev.exe`。

## 当前限制

- 与特定游戏版本绑定，不是通用 IL2CPP 插件。
- 联机大厅和跳蚤市场仍是占位功能。
- 旧联机模块仅保留基础网络探测，不包含玩家和库存同步。
- 出售功能未实现。
- 多数世界交互和原版剧情传送会被主动过滤。
- 部分场景的默认点位覆盖不完整，可使用地图点编辑功能补充。

## 开发说明

项目版本以根目录 `VERSION` 为唯一来源。构建命令：

```powershell
.\build.ps1
```

构建输出位于 `artifacts/`，一键安装包位于 `publish/`。

根 `README.md` 是唯一进入用户安装包和服务器更新清单的 Markdown。其他开发文档
只保留在源码目录，不随作品发布。

## 第三方与许可

项目当前未声明项目级许可证。内置 MinHook 的授权条款位于：

```text
native/minhook/LICENSE.txt
```
