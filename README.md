# SDN Controller Simulator

An NS-3 module that integrates an SDN (Software Defined Networking) controller for simulating programmable network environments.

## Overview

This project adds SDN support to NS-3 by integrating a controller module and leveraging OpenFlow. Ideal for researchers and students exploring SDN-enabled network simulation.

## Prerequisites

- Linux or macOS environment (WSL or native)
- Git
- CMake
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
```

### 2. Clone the SDN Controller Module
```bash
cd ..
git clone https://github.com/Sanj24Dev/sdn-controller-simulator.git
cp -r sdn-controller-simulator/sdn ns-3-dev/src/
```

### 3. Clone OpenFlow Module
```bash
git clone https://gitlab.com/nsnam/openflow.git
cd openflow
./waf configure
./waf build
```

### 4. Configure NS-3 with OpenFlow Support
```bash
cd ../ns-3-dev
./ns3 configure --enable-examples --enable-tests --with-openflow=../openflow
```
You should see in the summary: NS-3 OpenFlow Integration : enabled

### 5. Build NS-3
```bash
./ns3 build
```

## Running an example
To run the basic SDN simulation run the following cmd in ns-3-dev folder once the project is successfully built
```bash
./ns3 run sdn-one
```
This simulates:
<ul>
  <li>1 SDN switch</li>
  <li>2 End nodes with 2 packets being transmitted</li>
  <Li>Central SDN controller managing flows</Li>
</ul>
The simulation should generate the following output:


## Project Structure
sdn/ <br>
├── examples/ <br>
│ ├── sdn-one-switch.cc <br>
│ └── sdn-two-switch.cc <br>
├── model/ <br>
│ ├── control-packet.h <br>
│ ├── sdn-controller.cc <br>
│ ├── sdn-controller.h <br>
│ ├── sdn-switch.cc <br>
│ ├── sdn-swtich.h <br>
│ ├── sdn-flow-table.cc <br>
│ └── sdn-flow-table.h <br>
├── CMakeLists.txt # Build script for the module <br>

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request. For major changes, open an issue first to discuss your ideas.

## References

- [NS-3 Official Website](https://www.nsnam.org/)
- [OpenFlow Switch Support](https://www.nsnam.org/docs/models/html/openflow-switch.html)

























