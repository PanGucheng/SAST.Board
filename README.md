# SAST.Board

基于 **STM32F401xC + STM32 HAL** 的嵌入式实验项目，集成了矩阵键盘、ST7735S 彩屏、蜂鸣器、RGB 灯、ADC 波形显示与 HC-SR04 超声波测距等功能。

## 功能概览

- 4×4 矩阵键盘扫描（定时器中断去抖，事件回调机制）
- ST7735S（128×160）显示驱动与图形/文字绘制
- 蜂鸣器 PWM 控制与提示音
- RGB LED PWM 控制与呼吸效果
- ADC 采样 + 波形显示（频率与时基显示）
- 5 分钟倒计时（可加减时间、开始/暂停、复位）
- HC-SR04 超声波测距（串口输出距离）

## 当前按键行为（`Core/Hardware/key_callbacks.c`）

- Key1：屏幕显示 `Action: Key 1`
- Key2：屏幕显示 `Action: Key 2`
- Key3：清屏并蜂鸣 2 次
- Key4：蜂鸣器扫频测试 + 进度条显示
- Key5：打开 ADC 波形显示
- Key6：关闭 ADC 波形显示并清屏
- Key9~12：控制倒计时（加 1 分钟 / 减 1 分钟 / 开始暂停 / 复位）

> Key7~Key16 其余回调目前保留为空实现，可按需求扩展。

## 目录结构

```text
SAST.Board/
├── Core/
│   ├── Inc/            # CubeMX 生成头文件
│   ├── Src/            # 主程序与外设初始化
│   ├── Hardware/       # 自定义功能模块（键盘、屏幕、ADC、蜂鸣器等）
│   └── Startup/        # 启动文件
├── Drivers/            # CMSIS + STM32 HAL 驱动
├── CMakeLists.txt      # CMake 交叉编译配置
├── STM32F401CCUX_FLASH.ld
├── sast_board_everyting.ioc
└── st_nucleo_f4.cfg    # OpenOCD 调试/下载配置
```

## 构建说明（CMake）

项目使用 ARM GCC 交叉编译工具链：

- `arm-none-eabi-gcc`
- `arm-none-eabi-g++`
- `arm-none-eabi-objcopy`
- `arm-none-eabi-size`

构建命令：

```bash
cmake -S . -B build
cmake --build build
```

构建产物：

- `build/sast_board_everyting.elf`
- `build/sast_board_everyting.hex`
- `build/sast_board_everyting.bin`

> 注意：当前 `CMakeLists.txt` 中要求 `cmake_minimum_required(VERSION 4.0)`，请确保本机 CMake 版本满足要求。

## 烧录（OpenOCD 示例）

可结合 `st_nucleo_f4.cfg` 使用 OpenOCD：

```bash
openocd -f st_nucleo_f4.cfg \
  -c "program build/sast_board_everyting.elf verify reset exit"
```

## 开发说明

- 主入口：`Core/Src/main.c`
- 键盘扫描与事件：`Core/Hardware/keyboard.c`
- 按键功能回调：`Core/Hardware/key_callbacks.c`
- ADC 处理与显示：`Core/Hardware/my_adc.c`
- 蜂鸣器：`Core/Hardware/my_tim.c`
- RGB：`Core/Hardware/rgb_led.c`
- 超声波：`Core/Hardware/HC_SR04.c`

如需新增功能，建议优先在 `Core/Hardware/` 增加模块，并在 `main.c` 中完成初始化与调度。
