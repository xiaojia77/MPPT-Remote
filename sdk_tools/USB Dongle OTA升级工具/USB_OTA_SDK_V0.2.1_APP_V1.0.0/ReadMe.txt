===========================================
||*********** 开发前必读 ****************||
===========================================

# 开发资料

	apk --- 测试 APK 文件夹
	 ├── 测试 APK
	code --- 参考源码工程文件夹
	 ├── 参考 Demo 源码工程
	doc --- 开发文档文件夹
	 ├── 杰理 OTA 库(Android 版)开发说明外发文档.pdf --- 讲解 OTA 库的开发使用
	libs --- 核心库文件夹
	 ├── jl_usb_dongle_Vxxx.aar --- 杰理 USB 通讯相关
	 ├── jl_rcsp_main_Vxxx.aar --- 杰理 RCSP 协议相关
	 └── jl_ota_Vxxx.aar --- 杰理 OTA 相关
	 
# 实现参考

1. Dongle管理类实现参考 ``com.jieli.ota.tool.dongle.DongleManager``

2. OTA管理类实现参考 ``com.jieli.ota.tool.ota.OTAManager``


# 测试说明

## 测试文件存放说明
1. Dongle升级文件: /Android/data/com.jieli.ota/files/upgrade/dongle/
2. 键盘设备升级文件: /Android/data/com.jieli.ota/files/upgrade/kb/
3. 鼠标设备升级文件: /Android/data/com.jieli.ota/files/upgrade/mouse/


## 手机连接Dongle注意事项
1. 部分手机需要打开OTG选项，才能连上Dongle


## 测试步骤
1. 连上Dongle 
2. 发现可以升级的设备
3. 选择升级文件
4. 开始升级测试