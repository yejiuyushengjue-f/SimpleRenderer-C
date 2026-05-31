# SimpleRenderer

一个用于学习经典 CPU 渲染管线的简易软件渲染器。

## Current Features

- Win32 窗口创建与 CPU 端 BGRA 像素缓冲区显示。
- 颜色缓冲区和深度缓冲区，支持逐像素深度测试。
- 基础数学库：`Vec2`、`Vec3`、`Vec4`、`Mat4`、点乘、叉乘、归一化、平移、X/Y/Z 旋转、透视投影和正交投影。
- 主循环结构，包含场景更新、渲染和帧缓冲提交。
- 摄像机模块，支持键盘移动、方向键视角旋转、鼠标右键拖动视角，并生成 View/Projection 矩阵；初始视角会略微俯视场景，方便直接观察地面阴影。
- 摄像机俯仰角限制在 `-86` 到 `86` 度，避免视角上下翻转。
- Render Mode / Debug View 系统，可通过数字键切换并在窗口标题显示当前模式。
- Render Mode 会按模式跳过不需要的贴图采样、阴影查询和光照计算，便于观察模块时减少额外开销。
- 传统 Material 系统：支持 ambient、diffuse、specular 颜色，环境/漫反射/高光强度，shininess，以及可选 diffuse 纹理。
- 鲁棒性防护：对窗口/帧缓冲/纹理/阴影图尺寸做参数检查，过滤 NaN/Inf 深度和插值结果，窗口失焦时停止响应全局输入。
- 基于 Draw Command 的基础管线结构，支持网格顶点、模型变换和可选纹理绑定。
- 三角形列表网格渲染，每个网格可以包含多个三角形。
- Wavefront OBJ 模型加载，支持位置、UV、法线、负索引和多边形三角化。
- 背面剔除：在屏幕空间保留面向摄像机的三角形并剔除背面，减少无效光栅化。
- 视锥体裁剪：在齐次裁剪空间按左右、上下、近远六个平面裁剪三角形，跨越视锥边界的面会被保留并切分。
- 使用屏幕空间包围盒和重心坐标进行三角形填充光栅化。
- 顶点属性：位置、UV、法线和顶点颜色。
- 纹理映射，支持通过 Windows Imaging Component 加载 JPEG 纹理。
- 当图片不存在或解码失败时，自动生成棋盘格备用纹理。
- 对 UV、法线、世界坐标和观察空间坐标进行透视校正插值。
- 像素着色支持材质颜色、纹理颜色与顶点颜色共同参与计算。
- 多光源着色：环境光、多个方向光、点光源、材质化 Lambert 漫反射和 Blinn-Phong 高光。
- 主方向光 Shadow Mapping，使用 CPU 生成的光源空间深度图；支持 constant bias、slope-scale bias 和 3x3 加权 PCF 软阴影。
- 程序化三维网格：UV 球体、带纹理立方体和带棋盘格材质、可接收阴影的地面。
- 测试场景会加载 `res/Model` 或 `res/Models` 下找到的第一个 OBJ 模型，使其面向初始摄像机并保持水平旋转；场景包含一个后方立方体和一块地面，用于观察投影阴影；如果没有 OBJ，则使用前方球体和后方立方体作为备用场景。

## Debug Views

- `1` Final：完整最终渲染，包含材质、纹理、顶点颜色、多光源、Blinn-Phong 高光和阴影。
- `2` Albedo：显示纹理采样、顶点颜色和材质 diffuse 颜色共同得到的基础颜色，用于检查材质、纹理与 UV。
- `3` Normal：将观察空间法线映射到 RGB，用于检查法线方向和模型表面连续性。
- `4` Depth：显示当前摄像机视角下的 NDC 深度，用于检查深度缓冲和遮挡关系。
- `5` UV：将 UV 坐标映射到颜色，用于观察纹理坐标展开和重复。
- `6` Shadow Factor：显示经过 bias 和 PCF 后的阴影因子，白色为受光，深色为阴影，中间灰度为软阴影过渡。
- `7` Light：使用白色表面显示当前材质的纯光照响应，用于观察多光源、漫反射、高光、shininess 和阴影对亮度的影响。
- `8` Light-space Depth：显示当前像素投影到主方向光光源空间后的深度，黑色更靠近光源，白色更远；紫色表示超出当前光源正交投影范围。

## Assets

默认场景会查找这些纹理文件：

- `res/Texture/Frosted Metal Texture.jpeg`
- `res/Texture/Brushed metal texture.jpeg`

渲染器会从当前工作目录以及常见的构建输出父目录中查找资源。如果图片存在但无法解码，程序会退回到自动生成的棋盘格纹理，不会因为资源问题直接退出。

OBJ 模型可以放在以下任意目录：

- `res/Model`
- `res/Models`

加载器会读取找到的第一个 `.obj` 文件，将模型归一化到适合当前场景的尺寸，并使用磨砂金属纹理作为材质纹理。支持的面格式包括 `v`、`v/vt`、`v//vn` 和 `v/vt/vn`。

## Controls

- `W` / `S`：前进和后退。
- `A` / `D`：向左和向右移动。
- `Q` / `E`：向下和向上移动。
- `Space`：向上移动。
- 方向键：旋转视角。
- 按住鼠标右键并拖动：旋转视角。
- `Shift`：按住后加速移动。
- `1` 到 `8`：切换 Render Mode / Debug View。

## Build

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\SimpleRenderer.exe
```

如果你的 CMake 生成器创建了不同的配置目录，请使用 CMake 输出中显示的可执行文件路径。

交互测试复杂模型时，建议使用 Release 构建：

```powershell
cmake --build build --config Release
.\build\Release\SimpleRenderer.exe
```

本项目是纯 CPU 渲染器。高面数 OBJ、视锥体裁剪、背面剔除、Debug View、Shadow Mapping 和 PCF 阴影采样都在 CPU 上完成，Debug 构建会明显更慢；查看复杂模型时请优先使用 Release 版本。
