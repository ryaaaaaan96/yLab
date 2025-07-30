# YLab 嵌入式开发框架

## 项目概述

YLab是一个基于cortex系列微控制器的现代化嵌入式开发框架，采用分层架构设计，提供统一的设备抽象层和丰富的中间件支持。该框架借鉴了Linux设备驱动模型的设计理念，为嵌入式开发提供了高效、可扩展的解决方案。

### 核心特性

- 🚀 **分层架构设计** - 清晰的三层架构：应用层、中间件层、平台层
- 🔧 **设备抽象层** - 类Linux设备驱动模型，统一的init/read/write/ioctl接口
- 🛠️ **丰富的中间件** - FreeRTOS、Shell、Flash存储等完整中间件支持
- 📦 **模块化设计** - 松耦合的模块设计，易于扩展和维护
- 🔄 **CMake构建系统** - 现代化的构建配置，支持交叉编译

## 项目架构

```
yLab2.0/
├── 1-app/                    # 应用层
│   ├── main.c               # 主程序入口
│   ├── device/              # 设备相关代码
│   │   ├── inc/            # 设备头文件
│   │   └── src/            # 设备实现
│   └── task/               # 任务相关代码
│       ├── inc/            # 任务头文件 
│       └── src/            # 任务实现
├── 2-Midware/               # 中间件层
│   ├── yDev/               # yDev设备抽象层
│   │   ├── inc/           # 设备抽象接口
│   │   └── src/           # 设备抽象实现
│   ├── freeRTOS/          # FreeRTOS实时操作系统
│   │   ├── inc/           # FreeRTOS头文件
│   │   └── src/           # FreeRTOS源码
│   └── shell/             # 命令行Shell系统
│       ├── inc/           # Shell头文件
│       └── src/           # Shell实现
└── 3-yPlatform/     # 平台层
    ├── Core/              # STM32核心文件
    │   ├── startup_xxxxxx.s  # 启动文件
    │   └── system_xxxxx.c   # 系统初始化
    ├── fwlib/             # STM32 HAL/LL库
    │   ├── Inc/           # HAL/LL头文件
    │   └── Src/           # HAL/LL源码
    ├── yDrv/              # yDrv底层驱动
    │   ├── inc/           # 驱动头文件
    │   └── src/           # 驱动实现
    └── yLib/              # yLib通用库
        ├── inc/           # 库头文件
        └── src/           # 库实现
```

## 设计理念

### 三层架构设计

#### 1. 应用层 (1-app)
- **职责**: 业务逻辑实现和用户应用
- **特点**: 
  - 主程序入口和FreeRTOS任务管理
  - 设备功能封装和应用接口
  - 业务逻辑和用户交互
- **主要模块**:
  - `main.c`: 系统启动和初始化
  - `device/`: 具体设备功能实现(通信、开关等)
  - `task/`: FreeRTOS任务(闪烁、串口Shell等)

#### 2. 中间件层 (2-Midware)
- **职责**: 提供系统服务和抽象接口
- **特点**:
  - 跨平台的抽象接口设计
  - 丰富的系统服务组件
  - 模块化和可配置设计
- **主要组件**:
  - **yDev**: 设备抽象层，提供类Linux的设备驱动接口
  - **FreeRTOS**: 实时操作系统，支持多任务调度
  - **Shell**: 命令行系统，支持调试和交互

#### 3. 平台层 (3-ySTM32F4Platform)
- **职责**: 硬件抽象和底层驱动
- **特点**:
  - 硬件相关的底层实现
  - 标准化的驱动接口
  - 高效的资源管理
- **主要组件**:
  - **Core**: 核心文件和系统配置
  - **fwlib**: HAL/LL官方库
  - **yDrv**: 统一的底层驱动框架
  - **yLib**: 通用数据结构和算法库

### yDev设备抽象层

yDev是本框架的核心创新，借鉴Linux设备驱动模型：

```c
// 统一的设备操作接口
typedef struct {
    yDevStatus_t (*init)(yDevHandle_t *handle, void *config);
    yDevStatus_t (*deinit)(yDevHandle_t *handle);
    int32_t (*read)(yDevHandle_t *handle, void *buffer, size_t size);
    int32_t (*write)(yDevHandle_t *handle, const void *data, size_t size);
    yDevStatus_t (*ioctl)(yDevHandle_t *handle, uint32_t cmd, void *arg);
} yDevOps_t;
```

**主要特性:**
- 🔌 **统一接口**: 所有设备使用相同的操作接口
- 🎯 **类型安全**: 强类型的设备句柄和参数
- 🔄 **热插拔**: 支持设备的动态注册和注销
- 📊 **状态管理**: 完整的设备状态跟踪

### yDrv驱动框架

yDrv提供标准化的底层驱动接口：

```c
// GPIO驱动示例
yDrvStatus_t yDrvGpioInit(yDrvGpioConfig_t *config, yDrvGpioHandle_t *handle);
yDrvStatus_t yDrvGpioWrite(yDrvGpioHandle_t *handle, yDrvPinState_t state);
yDrvPinState_t yDrvGpioRead(yDrvGpioHandle_t *handle);
```

**支持的外设:**
- 🔌 **GPIO**: 通用输入输出，支持中断
- 📡 **USART**: 串口通信，支持DMA
- 🌐 **SPI**: SPI通信，支持多设备
- 💾 **DMA**: 直接内存访问，高效数据传输

## 快速开始

### 环境要求

- **工具链**: ARM GCC (arm-none-eabi-gcc)
- **构建系统**: CMake 3.22+
- **调试器**: ST-Link V2/V3 或 J-Link
- **IDE**: VS Code (推荐) 或 STM32CubeIDE

### 编译构建

1. **克隆项目**
```bash
git clone <repository-url>
cd yLab2.0
```

2. **配置构建**
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

3. **编译项目**
```bash
cmake --build . --target all
```

4. **烧录程序**
```bash
# 使用ST-Link
st-flash write YLab_STM32F4_Template.bin 0x8000000

# 或使用OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program YLab_STM32F4_Template.elf verify reset exit"
```

### 硬件配置

#### 支持的开发板
- STM32F407 Discovery
- STM32F429 Discovery  
- 自定义STM32F4xx板卡

#### 引脚配置
```c
// LED闪烁 (默认配置)
#define LED_GPIO_PORT    GPIOD
#define LED_GPIO_PIN     LL_GPIO_PIN_12

// 串口Shell (默认配置)
#define USART_TX_PIN     YDRV_PINA9   // PA9
#define USART_RX_PIN     YDRV_PINA10  // PA10

// SPI Flash (可选)
#define SPI_CS_PIN       YDRV_PINA4   // PA4
#define SPI_SCK_PIN      YDRV_PINA5   // PA5
#define SPI_MISO_PIN     YDRV_PINA6   // PA6
#define SPI_MOSI_PIN     YDRV_PINA7   // PA7
```

## 功能特性

### 实时操作系统支持

基于FreeRTOS v10.x，提供完整的多任务支持：

```c
// 任务创建示例
void StartDefaultTask(void *argument) {
    // LED闪烁任务
    TaskHandle_t blinkTaskHandle = NULL;
    xTaskCreate(BlinkTask, "BlinkTask", 256, NULL, 1, &blinkTaskHandle);
    
    // Shell任务
    TaskHandle_t shellTaskHandle = NULL;
    xTaskCreate(SerialShellTask, "ShellTask", 512, NULL, 2, &shellTaskHandle);
    
    vTaskDelete(NULL);
}
```

**特性:**
- ⏰ **抢占式调度**: 支持优先级的任务调度
- 🔒 **同步机制**: 信号量、互斥锁、消息队列
- 📊 **内存管理**: 动态内存分配(heap_4)
- ⚡ **低功耗**: 支持Tickless模式

### 命令行Shell系统

集成Letter Shell 3.2.4，提供强大的调试和交互功能：

```bash
# 系统信息查询
ylab> info
YLab STM32F4 Framework v2.0
MCU: STM32F407VGT6
Flash: 1024KB, RAM: 192KB
FreeRTOS: Running, Tasks: 3

# GPIO控制
ylab> gpio write D12 1    # 点亮LED
ylab> gpio read D12       # 读取GPIO状态

# 内存查看
ylab> mem dump 0x20000000 256   # 查看内存内容
```

**特性:**
- 📝 **命令补全**: Tab键自动补全
- 📜 **历史记录**: 上下箭头浏览历史
- 🎨 **彩色输出**: 支持ANSI颜色码
- 🔧 **扩展性**: 易于添加自定义命令



## 开发指南

### 添加新设备

1. **定义设备配置**
```c
// 在 yDev/inc/yDev_xxx.h 中定义
typedef struct {
    yDrvGpioPin_t pin;
    uint32_t frequency;
    // 其他配置参数
} yDevXxxConfig_t;
```

2. **实现设备操作**
```c
// 在 yDev/src/yDev_xxx.c 中实现
static yDevStatus_t xxx_init(yDevHandle_t *handle, void *config);
static int32_t xxx_read(yDevHandle_t *handle, void *buffer, size_t size);
static int32_t xxx_write(yDevHandle_t *handle, const void *data, size_t size);

static const yDevOps_t xxx_ops = {
    .init = xxx_init,
    .read = xxx_read,
    .write = xxx_write,
    .ioctl = xxx_ioctl
};
```

3. **注册设备类型**
```c
// 注册新设备类型
yDevRegisterDeviceType("xxx", &xxx_ops);
```

### 添加新任务

1. **任务实现**
```c
// 在 1-app/task/src/ 中实现
void MyTask(void *argument) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(1) {
        // 任务逻辑
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}
```

2. **任务启动**
```c
// 在启动任务中创建
TaskHandle_t myTaskHandle = NULL;
xTaskCreate(MyTask, "MyTask", 512, NULL, 1, &myTaskHandle);
```

### 性能优化建议

1. **内存优化**
   - 合理设置任务栈大小
   - 使用静态内存分配避免碎片
   - 及时释放不需要的资源

2. **实时性优化**
   - 合理设置任务优先级
   - 使用DMA减少CPU占用
   - 避免在中断中进行复杂操作

3. **功耗优化**
   - 使用WFI指令进入低功耗模式
   - 关闭不需要的外设时钟
   - 使用定时器唤醒机制

## 配置说明

### CMake配置

主要配置选项：

```cmake
# 目标芯片配置
set(MCU_FAMILY STM32F4xx)
set(MCU_MODEL STM32F407xx)

# 编译优化选项
set(CMAKE_BUILD_TYPE Debug)        # Debug/Release
set(ENABLE_DMA ON)                 # 启用DMA支持
set(ENABLE_FLASH_STORAGE ON)       # 启用Flash存储
```

### FreeRTOS配置

关键配置项 (FreeRTOSConfig.h)：

```c
#define configUSE_PREEMPTION                1     // 抢占式调度
#define configUSE_IDLE_HOOK                 0     // 空闲钩子
#define configUSE_TICK_HOOK                 0     // 时钟钩子
#define configCPU_CLOCK_HZ                  180000000  // CPU频率
#define configTICK_RATE_HZ                  1000  // 时钟节拍频率
#define configMAX_PRIORITIES                5     // 最大优先级数
#define configMINIMAL_STACK_SIZE            128   // 最小栈大小
#define configTOTAL_HEAP_SIZE               32768 // 堆大小
```

### Shell配置

Shell配置项 (shell_cfg.h)：

```c
#define SHELL_USING_TASK                1    // 使用任务模式
#define SHELL_TASK_STACK_SIZE           2048 // 任务栈大小
#define SHELL_MAX_NUMBER                8    // 最大Shell数量
#define SHELL_COMMAND_MAX_LENGTH        50   // 最大命令长度
#define SHELL_PARAMETER_MAX_NUMBER      8    // 最大参数数量
```

## 故障排除

### 常见问题

1. **编译错误**
   ```
   问题: undefined reference to '__aeabi_d2iz'
   解决: 添加 -lm 链接选项，或检查浮点数使用
   ```

2. **烧录失败**
   ```
   问题: Error: Failed to connect to target
   解决: 检查ST-Link连接，确认芯片型号配置正确
   ```

3. **任务栈溢出**
   ```
   问题: Task stack overflow
   解决: 增加任务栈大小，检查局部变量使用
   ```

4. **Shell无响应**
   ```
   问题: Shell命令无法执行
   解决: 检查串口配置，确认波特率设置正确
   ```

### 调试技巧

1. **使用Shell调试**
   - 添加调试命令查看系统状态
   - 使用`ps`命令查看任务运行情况
   - 使用`free`命令查看内存使用

2. **使用JTAG调试**
   - 设置断点调试关键路径
   - 查看变量和寄存器状态
   - 使用实时跟踪功能

3. **日志输出**
   - 通过串口输出调试信息
   - 使用不同日志级别分类信息
   - 记录关键事件和错误

## 版本历史

### v2.0 (2025-07-30)
- ✨ 支持STM32F4平台和G0平台，切换至相应分支即可
- 🔄 重构yDrv驱动框架
- 📦 优化三层架构设计
- 🚀 性能优化和稳定性提升

### v1.0 (2025-01)
- 🎉 初始版本，基于STM32G0
- 🔧 基础的yDev设备抽象层
- 📱 集成FreeRTOS和Shell
- 💾 基础Flash存储支持

## 贡献指南

我们欢迎社区贡献！请遵循以下准则：

1. **代码风格**
   - 使用统一的代码格式
   - 添加适当的注释和文档
   - 遵循现有的命名规范

2. **提交规范**
   - 使用清晰的提交信息
   - 一个提交解决一个问题
   - 测试通过后再提交

3. **问题报告**
   - 使用Issue模板报告问题
   - 提供详细的复现步骤
   - 包含必要的日志信息

## 许可证

本项目基于 MIT 许可证开源，详见 [LICENSE](LICENSE) 文件。

## 联系我们

- **项目主页**: https://github.com/ryaaaaaan96/yLab
- **技术支持**: 通过 GitHub Issues
- **开发团队**: YLab Development Team

---

**YLab Framework - 让嵌入式开发更简单、更高效！** 🚀
