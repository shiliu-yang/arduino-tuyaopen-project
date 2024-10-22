# tuyaDesktopDisplay

参考项目仓库：[chuxin520922/SmallDesktopDisplay (github.com)](https://github.com/chuxin520922/SmallDesktopDisplay/tree/main)
外壳使用的是 SD2 的外壳。

由于 T2 开发板 flash 空间有限，所以天气的图片都存放到了 uf 区域里面。所以在编译烧录主固件前需要先进行一下操作：

+ 擦除应用代码区域，防止当前应用代码有文件系统操作功能。擦除开始地址：`0x11000`，长度: `0x119000`
+ 擦除文件系统区域，***该操作会导致授权信息，激活信息丢失！！！*** 擦除开始地址：`0x1d2000`，长度: `0x2e000`
+ 烧录天气文件写入程序，`2.Tools/weatherImgWrite/weatherImgWrite.ino`

编译烧录桌面小电视固件：

1. 下载 arduino-tuyaopen。在 Arduino IDE 2 上的开发板管理器上搜索下载  `Tuya Open` 
2. 复制 `1.Software/libraries` 到 Arduino 库目录
3. 编译烧录桌面小电视固件 `1.Software/tuyaDesktopDisplay/tuyaDesktopDisplay.ino`

> 注意：需要一个 tuya open sdk 授权码，在 tuyaDesktopDisplay.ino 文件中进行修改。[tuya-open-sdk-for-device/README_zh.md at master · tuya/tuya-open-sdk-for-device (github.com)](https://github.com/tuya/tuya-open-sdk-for-device/blob/master/README_zh.md#涂鸦云应用) 

[嘉立创开源广场项目链接](https://oshwhub.com/yangyangyan/tuyadesktopdisplay)
