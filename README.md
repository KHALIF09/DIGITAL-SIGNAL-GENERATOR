# DIGITAL-SIGNAL-GENERATOR

⸻

📘 Digital Signal Generator (C++ • ImGui • ImPlot • CMake)

A full-featured Digital Signal Generator written in modern C++17, using:
	•	Dear ImGui → GUI
	•	ImPlot → Real-time plotting
	•	GLFW + OpenGL → Window + rendering
	•	CMake → Build system


⸻

🚀 Features

Digital Signal Processing
	•	PCM (8-bit) encoding
	•	Delta Modulation
	•	Manacher’s Algorithm for longest palindrome detection
	•	Line encoding schemes:
	•	NRZ-L
	•	NRZ-I
	•	Manchester
	•	Differential Manchester
	•	AMI (with real violation handling)

Scrambling

Supports:
	•	B8ZS
	•	HDB3

Violation pulses (B/V) correctly mapped to real bipolar AMI voltages.

Decoding

Decoders for all implemented schemes:
	•	NRZ-L
	•	NRZ-I
	•	Manchester
	•	Dif-Manchester
	•	AMI

Visualization
	•	Real-time graph plotting using ImPlot
	•	Clean GUI with:
	•	Input controls
	•	Encoding options
	•	Scrambling controls
	•	Analysis window (mean, std, palindrome, decoding accuracy)

⸻

📁 Project Structure

/DigitalSignalGeneratorCpp
│
├── CMakeLists.txt
│
└── src/
    ├── main.cpp
    ├── DigitalSignalGenerator.hpp
    └── DigitalSignalGenerator.cpp


⸻

🔧 Requirements

Linux / macOS / Windows (WSL supported)

You need:
	•	C++17 compiler (g++, clang++, MSVC)
	•	CMake ≥ 3.16
	•	OpenGL development libraries
	•	git (for pulling external deps)

On Ubuntu:

sudo apt update
sudo apt install g++ cmake git xorg-dev libglu1-mesa-dev

On macOS (Homebrew):

brew install cmake glfw


⸻

🛠 Build Instructions

Step 1 — Clone project

git clone <your repo>
cd DigitalSignalGeneratorCpp

(If using the ZIP, just unzip & cd into folder.)

⸻

Step 2 — Generate build files

mkdir build
cd build
cmake ..

This will automatically download:
	•	Dear ImGui
	•	ImPlot
	•	GLFW

⸻

Step 3 — Build

cmake --build . --config Release


⸻

Step 4 — Run

Linux/macOS:

./DigitalSignalGeneratorCpp

Windows (MSVC):

.\Release\DigitalSignalGeneratorCpp.exe


⸻

💡 Usage Guide

1. Choose Input Type
	•	Digital Input → Enter bit string
	•	Analog Input
	•	PCM (8-bit)
	•	Delta Modulation
	•	Auto-generates a sine wave and converts it to bits

2. Select Line Encoding Scheme
	•	NRZ-L / NRZ-I
	•	Manchester / Differential Manchester
	•	AMI

3. Scrambling (AMI only)
	•	Enable checkbox
	•	Choose:
	•	B8ZS
	•	HDB3

4. Generate Signal

Plots voltage vs time and prints:
	•	Palindrome analysis
	•	Mean & standard deviation
	•	Scrambling details

5. Decode Signal
	•	Decodes currently plotted waveform
	•	Shows accuracy (% match)

6. Clear

Resets everything.

⸻

📦 Dependencies

All dependencies are automatically downloaded by CMake:
	•	Dear ImGui
	•	ImPlot
	•	GLFW
	•	OpenGL

You do not need to install these manually.

⸻

🧠 Algorithms Implemented

Manacher’s algorithm
	•	O(n) longest palindrome search

Encoding rules (NRZ/Manchester/AMI)
	•	Fully implemented per IEEE specs

Scrambling rules
	•	B8ZS replaces long zero sequences with:
000VB0VB
	•	HDB3 replaces 4 zeros with either:
000V or B00V

Violation Logic (AMI)
	•	B = same polarity as last mark
	•	V = opposite polarity

⸻

📝 License

This project is completely free to use, modify, or integrate into your personal work.

⸻

✨ Author

Created by Khalif

