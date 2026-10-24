# FPGA-Based Multi-Processor Image Processing System

**ECE3073 – Computer Systems | Monash University**

## Project Overview

This project involves the development of a real-time image processing system implemented on a DE10-Nano FPGA platform using Intel Quartus, Nios II soft-core processors, and Verilog HDL.

The system captures image data from an ESP32-CAM, processes the images through multiple Nios II processors, and displays the output on a VGA monitor.

The project evolved from a single-processor implementation into a multi-processor architecture to support image acquisition, processing, edge detection, and display operations.

## Technologies Used

- **Hardware:** DE10-Nano FPGA, ESP32-CAM, ADXL345 accelerometer
- **Programming Languages:** C, Verilog HDL
- **Development Tools:** Intel Quartus Prime, Platform Designer (Qsys), Nios II Software Build Tools
- **Communication:** SPI
- **Display:** VGA
- **Image Processing:** Image flipping, box blur, Sobel edge detection
- **Embedded Systems:** Multi-processor architecture, shared memory, custom Nios II instructions

## System Features

### 1. Multi-Processor Architecture

The final implementation uses four Nios II processor roles:

- **Communication Processor:** Manages camera and accelerometer communication through SPI.
- **Processing Processor:** Coordinates image processing, buffering, and user input.
- **Edge Detection Processor:** Performs Sobel edge detection using a custom Nios II instruction.
- **Data Transfer Processor:** Handles processed image data and VGA rendering.

The processors coordinate through shared memory buffers to support image acquisition and display operations.

### 2. Real-Time Image Processing

The system supports four image display modes:

- Original camera image
- Horizontally flipped image
- Box blur filtering
- Sobel edge detection

A custom Verilog-based Nios II instruction was implemented to support Sobel edge detection.

### 3. VGA Display

The image-processing system supports:

- Single-image display mode
- Quad-image display mode
- Multiple image effects displayed simultaneously
- Display-mode selection through FPGA switches and buttons

### 4. Accelerometer-Based Controls

The ADXL345 accelerometer provides user interaction:

- Double-tap detection for switching single-image display effects
- Orientation-based rearrangement of images in quad-display mode

### 5. Camera Communication

The communication processor interfaces with an ESP32-CAM through SPI.

It coordinates image acquisition and transfers image data through shared buffers for subsequent processing and display.

## My Contributions

My contributions to this group project included:

**Communication Processor**
- Implemented SPI communication with the ESP32-CAM and ADXL345 accelerometer.
- Coordinated image acquisition and inter-processor communication using shared memory buffers.
- Supported single-image and quad-image display operations.

**Data Transfer and VGA Output**
- Developed the data transfer and display processor functionality.
- Implemented image rendering operations for VGA output.

**Custom Nios II Instruction**
- Developed a Verilog-based custom instruction for Sobel edge detection.
- Integrated the custom instruction into the FPGA processing architecture.

**System Integration and Debugging**
- Resolved processor, JTAG, and PIO connectivity issues in Platform Designer.
- Addressed accelerometer functionality problems during multi-processor integration.

## Repository Structure

- `M2/` – Final multi-processor FPGA implementation
- `M2/software/` – Nios II processor application source code
- `Screenshots/` – Project photographs, VGA output, and system diagrams

## Project Demonstration

The system demonstrates real-time image acquisition and processing with multiple display modes.

Suggested demonstration images:
- FPGA hardware and ESP32-CAM setup
- Original VGA camera output
- Image flipping and box blur
- Sobel edge detection
- Quad-image display
- Multi-processor architecture in Platform Designer

## How to Run

1. Open the Quartus project (`M2/ECE3073_Project.qpf`) using a compatible Intel Quartus Prime version.
2. Open the Platform Designer configuration and regenerate the required system files if necessary.
3. Generate the Nios II board support packages and build the four software applications.
4. Compile the FPGA design and program the DE10-Nano board.
5. Connect the ESP32-CAM, ADXL345 accelerometer, and VGA display using the project's hardware configuration.
6. Launch the Nios II applications with the correct processor assignments.
7. Use the FPGA buttons, switches, and accelerometer to interact with the image-processing system.

**Note:** This repository provides the project source code. Generated FPGA and software build files are excluded and must be recreated using a compatible development environment.

## Skills Demonstrated

- FPGA system development
- Verilog HDL programming
- Embedded C programming
- Nios II soft-core processor development
- Multi-processor system integration
- SPI communication
- Image processing algorithms
- Custom hardware instruction development
- Hardware and software debugging

## Acknowledgements

Developed as a group project for ECE3073 – Computer Systems at Monash University.

The overall system was developed collaboratively, with individual contributions covering hardware integration, processor development, image-processing algorithms, display functionality, and system verification.
