# HC-SR04 超声波模块驱动使用说明

本目录提供基于 STM32 HAL 的 HC-SR04 超声波测距模块驱动。

## 1. 文件位置

- 头文件：`app/drivers/HC-SR04/HC-SR04.h`
- 源文件：`app/drivers/HC-SR04/HC-SR04.c`

当前版本支持：

- 初始化 Trig / Echo 引脚
- 发送 10 us 触发脉冲
- 读取 Echo 高电平持续时间
- 读取距离，单位支持 cm 和 mm
- 返回状态码，便于判断超时或参数错误

## 2. 当前硬件配置

当前驱动默认使用：

- Trig：`PA0`
- Echo：`PA1`

对应宏定义在 `HC-SR04.h` 中：

```c
#define HCSR04_TRIG_GPIO_Port      GPIOA
#define HCSR04_TRIG_Pin            GPIO_PIN_0
#define HCSR04_ECHO_GPIO_Port      GPIOA
#define HCSR04_ECHO_Pin            GPIO_PIN_1
```

如果后续更换引脚，只需要修改这四个宏，并同步修改 CubeMX 中的 GPIO 配置。

## 3. CubeMX 需要配置什么

你已经给 `PA0` 和 `PA1` 加了用户标签，这很好。还需要确认：

1. `PA0` 配置为 GPIO Output
2. `PA1` 配置为 GPIO Input
3. GPIOA 时钟已开启
4. 工程已生成 `MX_GPIO_Init()`

当前驱动内部也会在 `HCSR04_Init()` 中重新初始化 `PA0` 和 `PA1`：

- `PA0`：推挽输出，无上下拉
- `PA1`：输入，无上下拉

因此 CubeMX 中只要保证引脚没有被其他外设占用即可。

本驱动使用 Cortex-M4 的 DWT 周期计数器实现微秒延时和 Echo 计时，不需要额外配置 TIM 定时器。

## 4. 硬件连接注意事项

HC-SR04 常见供电为 5 V：

- VCC 接 5 V
- GND 接 STM32 GND
- Trig 接 `PA0`
- Echo 接 `PA1`

注意：HC-SR04 的 Echo 输出通常是 5 V，而 STM32F407 的 GPIO 输入不能直接长期承受 5 V。建议 Echo 到 PA1 之间加分压，例如：

- Echo -- 1 kΩ -- PA1
- PA1 -- 2 kΩ -- GND

也可以使用电平转换模块。

## 5. 初始化流程

在主程序初始化阶段调用：

```c
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();

HCSR04_Init();
```

如果你在 `main.c` 中使用，需要包含头文件：

```c
#include "HC-SR04.h"
```

如果编译器找不到头文件，可以在 CMake 中把 `app/drivers/HC-SR04` 加到 include path，或者使用相对路径包含。

## 6. 接口说明

### `void HCSR04_Init(void);`

初始化 HC-SR04 驱动。

功能：

- 开启 GPIOA 时钟
- 初始化 DWT 微秒计时
- 配置 Trig 为输出
- 配置 Echo 为输入
- 将 Trig 拉低

调用示例：

```c
HCSR04_Init();
```

### `HCSR04_StatusTypeDef HCSR04_ReadPulseUs(uint32_t *pulse_us);`

读取 Echo 高电平持续时间，单位为 us。

调用示例：

```c
uint32_t pulse;

if (HCSR04_ReadPulseUs(&pulse) == HCSR04_OK) {
    /* pulse 是 Echo 高电平宽度，单位 us */
}
```

返回值：

- `HCSR04_OK`：读取成功
- `HCSR04_TIMEOUT`：等待 Echo 超时
- `HCSR04_ERROR`：传入参数为空

### `HCSR04_StatusTypeDef HCSR04_ReadDistanceCm(float *distance_cm);`

读取距离，单位为 cm。

调用示例：

```c
float distance;

if (HCSR04_ReadDistanceCm(&distance) == HCSR04_OK) {
    /* distance 单位为 cm */
}
```

### `HCSR04_StatusTypeDef HCSR04_ReadDistanceMm(float *distance_mm);`

读取距离，单位为 mm。

调用示例：

```c
float distance_mm;

if (HCSR04_ReadDistanceMm(&distance_mm) == HCSR04_OK) {
    /* distance_mm 单位为 mm */
}
```

### `HCSR04_StatusTypeDef HCSR04_ReadData(HCSR04_DataTypeDef *data);`

一次读取完整数据。

结构体定义：

```c
typedef struct {
    uint32_t pulse_us;
    float distance_cm;
    float distance_mm;
} HCSR04_DataTypeDef;
```

调用示例：

```c
HCSR04_DataTypeDef data;

if (HCSR04_ReadData(&data) == HCSR04_OK) {
    /* data.pulse_us */
    /* data.distance_cm */
    /* data.distance_mm */
}
```

### `float HCSR04_ReadDistanceCm_Blocking(void);`

阻塞式读取距离，单位 cm。

调用示例：

```c
float distance;

distance = HCSR04_ReadDistanceCm_Blocking();
if (distance >= 0.0f) {
    /* 读取成功 */
}
```

如果失败，会返回 `HCSR04_INVALID_DISTANCE`，默认值为 `-1.0f`。

### `float HCSR04_ReadDistanceMm_Blocking(void);`

阻塞式读取距离，单位 mm。

调用示例：

```c
float distance_mm;

distance_mm = HCSR04_ReadDistanceMm_Blocking();
```

## 7. 典型使用示例

```c
#include "HC-SR04.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    HCSR04_Init();

    while (1)
    {
        float distance_cm;

        if (HCSR04_ReadDistanceCm(&distance_cm) == HCSR04_OK) {
            /* 在这里使用 distance_cm */
        }

        HAL_Delay(100);
    }
}
```

## 8. 测距计算方式

驱动中使用声速近似值：

```c
#define HCSR04_SOUND_SPEED_CM_US   0.0343f
```

距离计算：

```c
distance_cm = pulse_us * 0.0343f / 2.0f;
```

也就是 Echo 高电平时间乘以声速，再除以 2。

## 9. 注意事项

- 两次测距之间建议间隔至少 60 ms，避免上一次超声波回波影响下一次测量。
- Echo 建议做 5 V 到 3.3 V 电平转换。
- 如果一直返回 `HCSR04_TIMEOUT`，优先检查接线、供电、Echo 电平和 Trig 是否输出脉冲。
- 当前驱动是阻塞式测量，测量期间会等待 Echo 信号，最长等待时间由 `HCSR04_TIMEOUT_US` 决定。
- 如果需要放在高实时性任务中，建议后续改成外部中断或输入捕获版本。
