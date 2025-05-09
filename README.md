# SDN Controller Simulator

An NS-3 module that integrates an SDN (Software Defined Networking) controller for simulating programmable network environments.

## Overview

This project adds SDN support to NS-3 by integrating a controller module and leveraging OpenFlow. It includes an example simulation (`sdn-one`) with one switch and two end-host nodes. Ideal for researchers and students exploring SDN-enabled network simulation.

## Prerequisites

- Linux or macOS environment (WSL or native)
- Git
- Python 3
- C++ compiler (GCC or Clang)
- NS-3 version 3.43
- OpenFlow source

## Installation & Setup

Follow the steps below to set up NS-3 with the SDN controller module:

### 1. Clone NS-3

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev
git checkout -b ns-3.43-branch ns-3.43
