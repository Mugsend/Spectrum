# Spectrum
<img width="1202" height="330" alt="image" src="https://github.com/user-attachments/assets/40858446-894e-4517-a522-52e56cbe11b0" />

Spectrum is a real-time audio spectrum analyzer plugin built with C++ and the JUCE framework. It provides highly customizable visual metering for mixing and audio analysis, featuring dynamic FFT sizing, multiple frequency scales (including MIDI pitch), and an interactive UI with a custom dark-mode aesthetic.

## Features

* **High-Resolution Analysis:** Selectable FFT sizes ranging from 2048 up to 16384 for precise low-frequency resolution.
* **Multiple Scaling Modes:**
  * **Logarithmic:** Standard mixing view emphasizing the usable audio spectrum.
  * **Linear:** Even distribution of bins across the frequency range.
  * **Semitone (ST) / MIDI:** Aligns the frequency spectrum to the musical grid, drawing vertical lines at 'C' notes.
* **Channel Routing:** Isolate Left, Right, or sum (L+R) channels.
* **Display Modes:** Toggle between traditional Line drawing and vertical Bins, with an optional Peak hold overlay.
* **Dynamic dB Range:** Auto-scaling dB bounds that adapt to the incoming signal, or manual control via horizontal range sliders.
* **Interactive Tooltips:** Hover over the spectrum to instantly see the precise frequency (Hz/kHz), musical note, and dB level.
* **Custom UI:** Built with a custom `LookAndFeel` class for a sleek, flat, and modern interface.

## Prerequisites

To build this project from source, you will need:
* A C++ compiler supporting at least C++17 (MSVC for Windows, Clang for macOS/Linux).
* [JUCE Framework](https://juce.com/) (v8.x recommended).
* CMake (v3.15+) **OR** the Projucer.

## Getting Started

### Building with CMake
1. Clone the repository:
   ```
   git clone [https://github.com/mugsend/spectrum.git](https://github.com/mugsend/spectrum.git)
   cd spectrum
   ```
2. Configure the build:
   ```
   cmake -B build
   ```
3. Compile the plugin:
   ```
   cmake --build build --config Release
   ```
4. The compiled plugin (VST3, AU, Standalone) will be located in the build/ directory.

### Building with Projucer
1. Clone the repository.

2. Open the .jucer file in the Projucer application.

3. Ensure your global paths to the JUCE modules are set correctly.

4. Click the "Save and Open in IDE" button for your target platform (Visual Studio or Xcode).

5. Build the project in your IDE under the Release configuration.

## Project Structure
* /Source - Contains all C++ source and header files.

* PluginProcessor.cpp / .h - Handles the DSP, audio buffering, and FFT computations.

* PluginEditor.cpp / .h - Manages the plugin window layout and UI parameter attachments.

* MagnitudeMeter.cpp / .h - The core visualization component responsible for drawing the spectrum, grid, and tooltips.

* CustomLookAndFeel.h - UI styling overrides for sliders, buttons, and menus.
