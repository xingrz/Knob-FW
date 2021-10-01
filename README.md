Knob-FW [![Auto build](https://github.com/xingrz/Knob-FW/actions/workflows/ci.yml/badge.svg)](https://github.com/xingrz/Knob-FW/actions/workflows/ci.yml)
==========

[![license][license-img]][license-url] [![release][release-img]][release-url] [![issues][issues-img]][issues-url] [![commits][commits-img]][commits-url]

这是 [Knob](https://github.com/xingrz/Knob-PCB) 软件部分的源码。基于 [ESP-IDF](https://github.com/espressif/esp-idf) 开发。

## 编译

配置 ESP-IDF，详见[相关官方文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c3/get-started/index.html)。

```sh
# 获取源码
git clone https://github.com/xingrz/Knob-FW.git

# 初始化环境，假设你的 ESP-IDF 位于 ~/esp 目录下
. ~/esp/esp-idf/export.sh

# 执行编译
idf.py build
```

## 烧录

Knob 自带了一个 USB 口，插到电脑即可直接烧录，无需额外的串口转接板。

### 从源码烧录

首次烧录需要全量烧录：

```sh
idf.py flash
```

后续开发调试通常只需要烧录 app 分区：

```sh
idf.py app-flash
```

### 在线烧录

除了自己从源码构建外，你还可以下载[最新的固件][release-url]，在浏览器里通过 [Web ESPTool](https://esp.xingrz.me) 烧录。

## 开源许可

Knob 的软件部分采用 [GNU v3](LICENSE) 协议开源。

[release-img]: https://img.shields.io/github/v/release/xingrz/Knob-FW?style=flat-square
[release-url]: https://github.com/xingrz/Knob-FW/releases/latest
[license-img]: https://img.shields.io/github/license/xingrz/Knob-FW?style=flat-square
[license-url]: LICENSE
[issues-img]: https://img.shields.io/github/issues/xingrz/Knob-FW?style=flat-square
[issues-url]: https://github.com/xingrz/Knob-FW/issues
[commits-img]: https://img.shields.io/github/last-commit/xingrz/Knob-FW?style=flat-square
[commits-url]: https://github.com/xingrz/Knob-FW/commits/master
