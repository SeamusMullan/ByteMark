# ByteMark
**ByteMark** is an audio plugin that emulates the nostalgic sounds of vintage electronics and vocoders.
It features two core effects: Linear Prediction Coding (LPC) Processing and a Bitcrusher / Redux effect, blending retro aesthetics.


## Current Status: Unfinished

This project is currently being developed. As of 28/12/2024 the LPC Algorithm is not implemented fully and the plugin isnt functional on a DSP level. Spectrum Analyzers and UI Components are implemented and functioning correctly.


## Features

- **Authentic LPC Vocoder**: I've implemented a Linear Prediction Coding to emulate the vintage vocal effects and robotic tones from old talkboxes and voicebox toys.
- **Precision Bit Reduction**: Control bit depth and sampling rate to really dial the years back.
- **Multi-Platform Compatibility**: Available as a VST3 and AU plugin, ensuring seamless integration with popular DAWs.
- **Simple-to-Use Interface**: A sleek, user-friendly design that puts powerful sound shaping tools at your fingertips.
- **Efficient DSP**: ByteMark is designed to maximize performance, minimizing CPU usage so you can run it no matter how old the PC (with a few limits of course).

## How It Works

- **LPC Vocoder Emulation**: At its core, the LPC algorithm analyzes the input signal to predict and synthesize its spectral content. By modeling this behavior, ByteMark delivers realistic vocoder effects reminiscent of early digital audio technologies. It also gets used in a lot of telecommunications stuff too.
- **Bitcrusher / Redux**: By reducing bit depth and sample rate, ByteMark introduces digital artifacts that range from a little crunch to outright ruining your audio, but we all know that's fun sometimes.

## Installation

1. Download the latest release from the [Releases](#) section (Or build it yourself if you dare).
2. Run the pre-built installer for your platform and rescan your plugins in your DAW:
3. **If Building from source** place the .vst3 or .component files in the following locations
   - macOS: `~/Library/Audio/Plug-Ins/Components/` (AU) or `~/Library/Audio/Plug-Ins/VST3/`
   - Windows: `C:\Program Files\Common Files\VST3`
   - Linux: You probably already know where to put the files....

## Roadmap

Future updates will include:

- Expanded preset library
- Enhanced LPC customization options
- Quality of Life Improvements

## Feedback

I’d love to hear your thoughts! Feel free to open issues or share feature requests on the [GitHub Issues](#) page.

If you spot a feature you would like included, make a PR and let me know, I'd love to make this plugin better!
