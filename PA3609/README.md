# Paragon PA-3609 (Windows VST3)

JUCE 8 / CMake port of the PA-3609 web prototype DSP: limiter + diode/opto compressor per channel,
stereo link, mid/side, HF-emphasis detector, sidechain HPF, drive, parallel mix, in/out trim, meters.

## Get the installer (easiest): GitHub Actions
1. Create a new GitHub repo and push this folder to it.
2. Open the repo's Actions tab -> "Build Windows installer" -> latest run.
3. Download the artifact "Paragon-PA3609-VST3-Windows" -> run Paragon-PA3609-VST3-Setup.exe.

## Build locally
Requirements: Windows 10/11, Visual Studio 2022 (Desktop C++), CMake 3.22+, Git, Inno Setup 6.
    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\PA3609.iss
The installer appears in installer\Output\. Or run installer\Install-VST3-Manual.bat as Administrator next to the .vst3 folder, or copy the "Paragon PA-3609.vst3" folder from
build\PA3609_artefacts\Release\VST3\ into C:\Program Files\Common Files\VST3\ by hand.

## Notes
- JUCE is AGPL-3 or commercial: a closed-source release needs a JUCE commercial license.
- Unsigned installers trigger Windows SmartScreen; sign with a code-signing certificate before shipping.
- The UI is a functional parameter panel; the skinned faceplate from the web version is the next step.
- 64-bit VST3 only. No oversampling yet (the saturation can alias at high drive).
