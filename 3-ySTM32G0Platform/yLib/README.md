# YLib - 通用C数据结构和算法库

## 概述

YLib是一个高性能、轻量级的C语言数据结构和算法库，设计用于嵌入式和通用应用。该库提供了多种常用的数据结构实现，包括链表、队列、堆、红黑树等，以及相关的工具函数和宏定义。

## 版本信息

- **版本**: 2.0.0
- **作者**: Ryan
- **日期**: 2025/06/16
- **许可证**: [请根据实际情况填写]

## 目录结构

```
YLib/
├── inc/                    # 头文件目录
│   ├── yLib_def.h         # 通用定义和宏
│   ├── yLib_list.h        # 双向链表
│   ├── yLib_fifo.h        # FIFO队列
│   ├── yLib_ring.h        # 环形缓冲区
│   ├── yLib_heap.h        # 动态内存管理器
│   ├── yLib_mempool.h     # 内存池管理器
│   └── yLib_rbtree.h      # 红黑树
├── src/                   # 源文件目录
│   ├── yLib_list.c        # 双向链表实现
│   ├── yLib_fifo.c        # FIFO队列实现
│   ├── yLib_ring.c        # 环形缓冲区实现
│   ├── yLib_heap.c        # 动态内存管理器实现
│   ├── yLib_mempool.c     # 内存池管理器实现
│   └── yLib_rbtree.c      # 红黑树实现
├── yLib_config.h          # 统一配置文件
└── README.md              # 本文件
```

## 主要特性

### 🚀 高性能
- 优化的算法实现
- 内存对齐和缓存友好设计
- 支持编译器分支预测优化

### 🛡️ 安全可靠
- 全面的错误检查和断言
- 内存泄漏检测
- 魔术数字验证机制

### 🔧 灵活配置
- 统一的配置管理（yLib_config.h）
- 模块化设计，可独立使用
- 支持嵌入式和桌面环境

### 📊 丰富的调试功能
- 详细的调试日志
- 运行时统计信息
- 性能计数器（可选）

## 模块介绍

### 1. yLib_def.h - 通用定义
提供库的基础定义，包括：
- 通用宏定义（SWAP、MAX、MIN等）
- 内存操作宏
- 调试和日志宏
- 编译器特定的属性宏
- 位操作工具
- 错误处理宏

**关键特性：**
- 类型安全的宏定义
- 跨平台兼容性
- 丰富的调试工具

### 2. yLib_list.h - 双向链表
实现高效的双向链表数据结构。

**主要功能：**
- 插入、删除、查找操作
- 迭代器支持
- 安全检查机制

### 3. yLib_fifo.h - FIFO队列
先进先出队列的实现。

**主要功能：**
- 入队和出队操作
- 队列状态检查
- 可选的线程安全支持

### 4. yLib_ring.h - 环形缓冲区
高效的环形缓冲区实现，适用于数据流处理。

**主要功能：**
- 写入和读取操作
- 缓冲区状态管理
- 原子操作支持（可选）

### 5. yLib_heap.h - 动态内存管理器
FreeRTOS风格的动态内存管理器。

**主要功能：**
- 动态内存分配和释放
- 内存碎片管理
- 内存使用统计
- 内存泄漏检测

**配置项：**
- `configTOTAL_HEAP_SIZE`: 堆总大小（默认32KB）
- `configMINIMAL_BLOCK_SIZE`: 最小块大小（默认16字节）
- `configBYTE_ALIGNMENT`: 字节对齐（默认8字节）

### 6. yLib_mempool.h - 内存池管理器
uC/OS风格的固定大小内存池管理器。

**主要功能：**
- 固定大小块的快速分配
- 内存池统计
- 分区命名支持

**配置项：**
- `YLIB_MEM_NAME_EN`: 启用内存分区名称
- `YLIB_MEM_NAME_SIZE`: 分区名称最大长度

### 7. yLib_rbtree.h - 红黑树
自平衡二叉搜索树的实现。

**主要功能：**
- O(log n)时间复杂度的操作
- 自动平衡
- 中序遍历支持

## 快速开始

### 1. 包含头文件

```c
#include "yLib_config.h"    // 首先包含配置文件
#include "yLib_def.h"       // 基础定义
#include "yLib_list.h"      // 根据需要包含特定模块
```

### 2. 配置库

编辑 `yLib_config.h` 文件来配置库的行为：

```c
// 启用调试模式
#define DEBUG 1

// 配置堆大小
#define configTOTAL_HEAP_SIZE (64 * 1024)  // 64KB

// 启用详细错误信息
#define YLIB_VERBOSE_ERRORS 1
```

### 3. 基本使用示例

```c
#include "yLib_config.h"
#include "yLib_def.h"
#include "yLib_list.h"

int main() {
    // 使用链表示例
    yLib_list_t *list = yLib_list_create();
    
    // 添加元素
    int data = 42;
    yLib_list_push_back(list, &data);
    
    // 获取大小
    size_t size = yLib_list_size(list);
    printf("List size: %zu\n", size);
    
    // 清理
    yLib_list_destroy(list);
    
    return 0;
}
```

## 编译说明

### 基本编译

```bash
gcc -I./inc -c src/*.c
gcc -o myapp *.o main.c
```

### 带调试信息编译

```bash
gcc -DDEBUG -g -I./inc -c src/*.c
gcc -g -o myapp *.o main.c
```

### 嵌入式编译（以ARM为例）

```bash
arm-none-eabi-gcc -DYLIB_EMBEDDED_ENVIRONMENT=1 -I./inc -c src/*.c
arm-none-eabi-gcc -o myapp.elf *.o main.c
```

## 配置选项

YLib提供了丰富的配置选项，通过 `yLib_config.h` 文件进行管理：

### 调试配置
- `DEBUG`: 启用调试模式
- `ENABLE_COLOR_OUTPUT`: 启用彩色输出
- `YLIB_VERBOSE_ERRORS`: 详细错误信息

### 性能配置
- `YLIB_MEMORY_ALIGNMENT`: 内存对齐
- `YLIB_CACHE_LINE_SIZE`: 缓存行大小
- `YLIB_BRANCH_PREDICTION`: 分支预测优化

### 统计配置
- `YLIB_RUNTIME_STATS`: 运行时统计
- `YLIB_MEMORY_STATS`: 内存使用统计
- `YLIB_PERFORMANCE_COUNTERS`: 性能计数器

详细配置说明请参考 `yLib_config.h` 文件中的注释。

## 内存管理

YLib提供了两种内存管理方案：

### 1. 动态内存管理器（yLib_heap）
- 适用于不规则大小的内存分配
- 支持内存碎片整理
- 提供详细的内存使用统计

### 2. 内存池管理器（yLib_mempool）
- 适用于固定大小的内存分配
- 分配速度更快
- 无内存碎片问题

## 错误处理

YLib使用多层错误处理机制：

1. **返回值检查**: 大多数函数返回状态码
2. **断言检查**: 在调试模式下进行参数验证
3. **日志记录**: 详细的错误日志输出
4. **魔术数字**: 检测内存损坏

## 性能优化建议

1. **编译优化**: 使用 `-O2` 或 `-O3` 优化级别
2. **内存对齐**: 根据目标平台调整对齐设置
3. **模块裁剪**: 只包含需要的模块
4. **配置调优**: 根据应用场景调整配置参数

## 平台支持

YLib设计为跨平台库，支持：

- **桌面平台**: Linux, Windows, macOS
- **嵌入式平台**: ARM Cortex-M, ARM Cortex-A
- **编译器**: GCC, Clang, MSVC

## 已知限制

1. 某些功能需要C99标准支持
2. 线程安全功能需要额外的同步机制
3. 嵌入式环境下部分调试功能可能受限

## 常见问题

### Q: 如何启用线程安全？
A: 在配置文件中设置相应的线程安全选项，并确保链接适当的线程库。

### Q: 内存使用过高怎么办？
A: 可以调整堆大小、禁用不必要的统计功能、使用内存池代替动态分配。

### Q: 如何调试内存泄漏？
A: 启用 `configHEAP_CHECK_ENABLE` 和 `YLIB_MEMORY_STATS`，使用提供的内存统计API。

## 贡献指南

欢迎提交bug报告、功能请求和代码贡献。请确保：

1. 遵循现有的代码风格
2. 添加适当的测试用例
3. 更新相关文档
4. 确保向后兼容性

## 版本历史

### v2.0.0 (2025/06/16)
- 重构配置系统
- 添加统一配置文件
- 改进错误处理机制
- 优化性能
- 完善文档

### v1.x.x
- 初始版本功能

## 联系方式

- 作者: Ryan
- 邮箱: [请填写实际邮箱]
- 项目地址: [请填写实际地址]

## 许可证

[请根据实际情况填写许可证信息]

---

> 💡 **提示**: 使用前请仔细阅读各模块的头文件注释，了解详细的API使用方法。
