# Bob's Toy Renderer

## 关于此仓库
本仓库是由我学习图形学过程中开发的一个玩具渲染器重构而来。由于旧渲染器代码已较为臃肿且存在一些设计问题，因此计划从头重构整个项目。旧渲染器的功能展示视频在[这里](https://www.bilibili.com/video/BV1EWGJeGEcS/)。  

**目前仓库已经完成除全局光照外的大部分功能的迁移。旧渲染器的全部GLSL代码也已经在仓库中提供。**  

与渲染相关的主要目录及功能如下：
|  目录   | 功能描述  |
|  ----  | ----  |
| Asset/BuildIn/Shader/  | 项目已完成迁移的着色器代码，包括GPU-Driven, EVSM, PCF+CSM, PBR, Bloom，FXAA, Tone Mapping等 |
| Asset/BuildIn/Shader/legacy  | 旧渲染器的全部着色器代码，包括暂未迁移的Volumetric Fog, SSSR, DDGI, TAA等 |
| src/Runtime/Function/Render/RHI/  | 基本的RHI实现, 封装了Vulkan后端，暂未支持光追 |
| src/Runtime/Function/Render/RDG/  | 基本的RDG实现 |
| src/Runtime/Function/Render/RenderResource/  | 上层绘制资源封装及Bindless管理 |
| src/Runtime/Function/Render/RenderPass/  | 基于RDG构建的渲染管线，包含较完整的绘制流程 |
| src/Test/  | 展示了RHI及RDG层的基本使用(deprecated) |

仓库的其他部分也包含了HAL抽象，资源系统，Component-Entity-Scene等功能的简单实现。

## 构建
本仓库使用xmake作为构建工具。目前HAL抽象仅实现了windows平台，RHI仅支持Vulkan后端。

```shell
git clone https://github.com/CPJ-BO/ToyRenderer.git
cd ToyRenderer/renderer
xmake build renderer
xmake run renderer
```

## 参考链接
本仓库的学习和开发过程参考了许多优秀的开源仓库，十分感谢各位大佬的无私分享。

https://github.com/SakuraEngine/SakuraEngine  
https://github.com/google/filament  
https://github.com/EpicGames/UnrealEngine  
https://github.com/BoomingTech/Piccolo  
https://github.com/jjiangweilan/WeilanEngine  
https://github.com/Code-Guy/Bamboo  
https://github.com/flwmxd/LuxGI  
https://github.com/diharaw/hybrid-rendering  
https://github.com/liameitimie/learn-nanite  
https://github.com/KhronosGroup/Vulkan-Samples  


## 截图
![](snapshot/bunny.png)  

**（以下是旧渲染器的部分截图）**
![](snapshot/GPU-Driven.png)
![](snapshot/DDGI.png)
![](snapshot/PBR.png)