# dts-parser

A lightweight C program that parses Device Tree Source (DTS) files to extract GPIO configuration information for LEDs and buttons. This tool is particularly useful for embedded Linux developers working with device tree bindings.

## Description

`dts-parser` reads device tree source files, analyzes GPIO assignments for buttons (`gpio-keys`) and LEDs (`gpio-leds`), and presents the extracted information in a clean, readable format. It automatically resolves GPIO controller numbers, offsets, and active states.

## Features

- Parse .dts and .dtsi files
- Extract GPIO information for buttons and LEDs
- Handle GPIO_ACTIVE_LOW and GPIO_ACTIVE_HIGH flags
- Support for gpio-ranges from DTSI includes
- Export results to file or stdout
- Clean and structured output format

## Requirements

- Linux operating system
- GCC compiler
- Make (for building)
- Standard C library

## Building

```bash
# Clone the repository
git clone https://github.com/xmenbro/dts-parser.git
cd dts-parser

# Build the program
make
```

## Usage

```bash
# Basic usage
./dts-parser board.dts

# Advanced usage
./dts-parser [-i dtsi_path] [-o output_file] input_file
```
### Options
- -i: Path to DTSI include file (for gpio-ranges extraction)
- -o: Output file path (default: stdout)
- input_file: Path to the DTS file to parse (required)

## Example

Given this dts file:
```
// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
/dts-v1/;

#include "mt7621.dtsi"

#include <dt-bindings/gpio/gpio.h>
#include <dt-bindings/input/input.h>
#include <dt-bindings/leds/common.h>

/ {
	compatible = "tplink,hc220-g5-v1", "mediatek,mt7621-soc";
	model = "TP-Link HC220 G5 v1";

	memory@0 {
		device_type = "memory";
		reg = <0x00000000 0x08000000>;
	};

	chosen {
		bootargs = "earlycon console=ttyS0,115200";
	};

	gpio-keys {
		compatible = "gpio-keys";

		key-reset {
			label = "reset";
			gpios = <&gpio1 8 GPIO_ACTIVE_LOW>;
			linux,code = <KEY_RESTART>;
		};

		key-wps {
			label = "wps";
			gpios = <&gpio2 16 GPIO_ACTIVE_LOW>;
			linux,code = <KEY_WPS_BUTTON>;
		};
	};

	leds {
		compatible = "gpio-leds";

		led-fault {
			color = <LED_COLOR_ID_RED>;
			function = LED_FUNCTION_FAULT;
			gpios = <&gpio3 13 GPIO_ACTIVE_HIGH>;
		};

		led-power {
			color = <LED_COLOR_ID_GREEN>;
			function = LED_FUNCTION_POWER;
			gpios = <&gpio4 14 GPIO_ACTIVE_HIGH>;
			linux,default-trigger = "default-on";
		};

		led-wps {
			color = <LED_COLOR_ID_BLUE>;
			function = LED_FUNCTION_WPS;
			gpios = <&gpio1 15 GPIO_ACTIVE_HIGH>;
		};
	};
};

&pcie {
	status = "okay";
};

&switch0 {
	ports {
		port@0 {
			status = "okay";
			label = "lan2";
		};

		port@1 {
			status = "okay";
			label = "lan1";
		};

		port@2 {
			status = "okay";
			label = "wan";
		};
	};
};
```
The result will be:
```
-----------------------------------------------------------------------
button:
40 reset low
80 wps low

led:
109 led-fault high
142 led-power high
47 led-wps high
-----------------------------------------------------------------------
```