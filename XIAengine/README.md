# XIA Engine

This subproject provides the low-level interface and configuration tools for XIA Pixie-16 modules.

## Features

- **Hardware Interface**: Implements the `XIAInterface` for interacting with XIA hardware, including getting/setting module and channel parameters.
- **Pixie-16 Support**: Includes specific drivers and utilities for Pixie-16 modules.
- **PLX Integration**: Low-level PCI/PLX communication drivers for hardware access.
- **XIA Configurator**: A GUI-based tool (using Qt) for configuring XIA modules, including baseline filters, CFD filters, and energy filters.

## Structure

- `include/`: Header files.
  - `xiainterface.h`: Abstract interface for XIA hardware interaction.
  - `Pixie16/`: Pixie-16 specific definitions.
  - `plx/`: PLX PCI communication headers.
- `src/`: Implementation.
  - `engine/`: Core engine logic and control.
  - `Pixie16/`: Pixie-16 driver implementation.
  - `plx/`: PLX driver implementation.
  - `XIAConfigurator/`: Source code for the configuration GUI.
