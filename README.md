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
