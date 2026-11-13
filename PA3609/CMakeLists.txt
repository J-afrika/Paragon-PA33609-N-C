cmake_minimum_required(VERSION 3.22)
project(PA3609 VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(PA3609
  COMPANY_NAME "Paragon Audio"
  PLUGIN_MANUFACTURER_CODE Prgn
  PLUGIN_CODE Pa36
  FORMATS VST3
  PRODUCT_NAME "Paragon PA-3609"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  VST3_CATEGORIES Fx Dynamics
  COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(PA3609 PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)

target_compile_definitions(PA3609 PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(PA3609
  PRIVATE juce::juce_audio_utils
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
