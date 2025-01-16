# EasyMatrix_Tuya

该项目是在 哔哩哔哩 UP 主 大聪明的二手脑袋 的 [EasyMatrix 像素时钟](https://www.bilibili.com/video/BV1FC411s7Yf/?spm_id_from=333.1387.homepage.video_card.click&vd_source=e8559f5a49990ff137c2bae949ba3a9c)项目的基础上加上 tuya 相关的特色，可以涂鸦智能 app 对颜色和开关状态进行控制改变。

![EasyMatrix](https://images.tuyacn.com/fe-static/docs/img/b344151f-1adc-4b75-b163-028417f5ab92.png)

## 1. 硬件

### 1.1. 清单

|         名称         | 数量 |                             备注                             |
| :------------------: | :--: | :----------------------------------------------------------: |
|         T3-U         |  1   | [T3-U 模组规格书](https://developer.tuya.com/cn/docs/iot/T3-U-Module-Datasheet?id=Kdd4pzscwf0il) |
| ws2812B灯珠 3528规格 | 256  |              可以多买几颗，防止残次品或者焊坏了              |
|     12mm金属按键     |  3   |                         高头款 3-6V                          |
|   16mm金属电源开关   |  1   |                       平头自锁款 3-9V                        |
|  MAX9814拾音器模块   |  1   |                                                              |
|      无源蜂鸣器      |  1   |                         6.5mm高矮体                          |
|  黑色半透明亚克力板  |  1   | 193.4 * 49.4 * 1.5mm，某宝上切割都会有误差，有点看运气，大了就砂纸磨掉点，小了就多涂点透明胶水 ^ ^ |
|    立式Type-C母座    |  1   |                       6P立贴，高6.8mm                        |
|     0805贴片电阻     |  2   |       5.1K，精度5%的就行，配合Type-C母座使用的下拉电阻       |
|  0805贴片电容0.1uF   | 248  | 标准ws2812电路是每颗灯珠配一个0.1uF电容，但是实测没有电容一点问题没有，所以电容可不买 |
|       绝缘胶水       |  1   |  透明的最好、半透明的也可以，如果是黑色的外壳，那黑色的也行  |
|         螺丝         |      |             滚花铜螺母M1.6*3*2.5，平头螺丝M1.6*6             |

### 1.2. 硬件连线

| T3 引脚 |                             功能                             |
| :-----: | :----------------------------------------------------------: |
|   24    |               按键 1，短按：当前页面上一种样式               |
|   32    |               按键2，短按：当前页面下一种样式                |
|   34    | 按键3，短按：切换页面（时间->拾音器->动画->闹钟->亮度调节）；长按：移除设备 |
|   36    |                         蜂鸣器输出脚                         |
|   25    |                           MIC 输入                           |
|   16    |                       像素灯输出控制脚                       |



## 2. 软件

处于闹铃状态下，按下任意按键关闭闹铃

编译像素时钟固件：

1. 在 Arduino IDE 2 上的开发板管理器上搜索 `Tuya Open` 进行下载；
2. 复制 `1.Software/libraries` 到 Arduino 库目录；
3. 编译烧录像素时钟固件 `1.Software/EasyMatrix_Tuya/EasyMatrix_Tuya.ino`

> 注意：需要一个 tuya open sdk 授权码，在 EasyMatrix_Tuya.ino 文件中进行修改。[tuya-open-sdk-for-device/README_zh.md at master · tuya/tuya-open-sdk-for-device (github.com)](https://github.com/tuya/tuya-open-sdk-for-device/blob/master/README_zh.md#涂鸦云应用)