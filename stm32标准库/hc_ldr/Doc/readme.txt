


本项目基于stm32f103ve，adc，tim，光敏，串口，蓝牙的混合使用。
光敏模块通过adc将模拟信号转换为数字信号交给dma转运到单片机，然后配置一秒定时中断通过串口将光强信息传给蓝牙手机端，
并且可以通过蓝牙发送数据控制单片机