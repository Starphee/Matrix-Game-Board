# Matrix Game Board

*Interactive Multi-Matrix Tic Tac Toe Board*

---

## Project Overview

The **Matrix Game Board** is an original engineering project by Dorian Todd, designing and building an **interactive Tic Tac Toe** game using multiple **LED matrix displays** as both the **visual board** and the **input mechanism**.

Developed as a technical challenge, this project documents the entire process, from initial research and CAD design through manufacturing, complex wiring, assembly, testing, and iteration. It highlights the innovative approach of using the LED matrices themselves as pressure-sensitive buttons via underlying mechanical switches, all within a compact and portable form factor.

This repository contains the technical documentation, design files, firmware, and other resources related to the project.

> [!NOTE]
> This documentation serves to demonstrate the project's creation process and does not intend to be a step-by-step instruction manual for replication. 

## Key Features

*   **Interactive LED Display:** Utilizes nine 8x8 LED matrix modules (HT16K33) to display the Tic Tac Toe game state.
*   **Integrated Input:** Each matrix display doubles as a pressure-sensitive button via a low-profile mechanical keyboard switch mounted underneath.
*   **Tactile Feedback:** Mechanical switches provide satisfying tactile response for gameplay interaction.
*   **Compact Design:** Designed to fit within a 6.5-inch cubed area, adhering to competition constraints.
*   **Custom Enclosure:** Features a 3D-printed base with integrated component mounts and a laser-cut top panel.
*   **Advanced Electronics:** Incorporates an Adafruit Feather ESP32 V2 microcontroller, I2C multiplexing for displays, and analog multiplexing for switch inputs to manage numerous components efficiently.
*   **Audio Feedback:** Includes a Piezo Buzzer with volume control for game sounds.
*   **Battery Powered:** Designed with support for a Lithium Polymer battery for portability.
*   **Iterative Design Process:** Documentation showcases the challenges faced during prototyping, wiring, and assembly, and the design improvements made to overcome them.

## Technical Details

The project is built upon the following core components:

*   **Microcontroller:** Adafruit ESP32 Feather V2
*   **Display:** 9x Ada8x8 LED Matrix Modules (HT16K33)
*   **Display Multiplexer:** TCA9548A I2C Multiplexer (Manages the I2C addresses of the nine matrix displays)
*   **Input Switches:** 9x Kailh Low-Profile Mechanical Keyboard Switches
*   **Input Multiplexer:** CD74HC4067 16-Channel Analog Multiplexer (Condenses the 9 switch inputs down to 4 microcontroller pins)
*   **Audio:** 1x Piezo Buzzer, 1x Volume Potentiometer
*   **Power:** 1x Power Switch, 1x Power LED, 1x Lithium Polymer 1000mAh 3.7v battery (charging via ESP32's USB-C port)
*   **Casing:** 3D-printed PLA base, Laser-cut wood top panel / side panels

A complete Bill of Materials (BoM) can be found [here](#bill-of-materials) or in the technical documentation file.

## Project Files

This repository includes (or will include) the following files:

*   `Technical Documentation.pdf`: The source document detailing the project process.
*   `/CAD/`: Fusion360 files and exported formats (STL, STEP) for the 3D-printed base and laser-cut panels.
*   `/Laser_Cut_Files/`: Files (DXF or similar) used for laser cutting the panel parts.
*   `/Firmware/`: Arduino code for the ESP32 Feather, including game logic, display control, and input handling.
*   `/Wiring_Diagram/`: Image of the project's wiring diagram.
*   `/Bill_of_Materials/`: A more detailed list of components.
*   `/Images/`: Photos and renders from the documentation showcasing the build process and final design.

## Getting Started (Exploring the Project)

While this is not an instruction manual to build your own, you can explore the project by:

*   **Cloning** this repository.
*   **Reviewing** the `Technical Documentation.pdf` to understand the design process, challenges, and solutions.
*   **Examining** the CAD files to see the mechanical design and component layout.
*   **Reviewing** the wiring diagram to understand the electronic connections.
*   **Exploring** the firmware code to see how the game logic, displays, and inputs are handled.

## Credits

*   **Design and Programming:** [Dorian Todd](http://www.doriantodd.com)
*   **Technical Documentation:** Dorian Todd
*   **Image Sources (Acknowledged in Doc):** Adafruit Industries et al., Hackaday et al., Altium et al., Gaqqee Gem DIY et al., Sac Valley Manufacturing et al.
*   **Inspiration:** Adam Savage's "One Day Builds" methodology.

## License

This project is open-source. Please see the [<LICENSE_FILE>](<LICENSE_FILE>) file for details on how you can use, modify, and distribute the files in this repository.

---
