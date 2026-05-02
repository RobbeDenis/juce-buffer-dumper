# juce-buffer-dumper
This wrapper provides an easy to use interface for writing audio data to files, using a `juce::ThreadedWriter`. The main reason for making this was to speed up debugging audio in JUCE, by writing audio to WAV files with automated name versioning that persists through different builds.

## Construction and Initialization
Include BufferDump.hpp where you want to write the audio. Create the `jbd::BufferDumper` by providing it the desired dump directory and filename. By default, sanitizePath is already true. If sanitizePath is enabled, BufferDumper will internally check if the given path and filename are valid through the use of JUCE's provided functionality. If false, then there will be no checks.

Before writing audio data, you must call Initialize. This method starts the background writer thread and configures the format writer using `juce::AudioFormatWriterOptions`. This object provides the necessary info, such as sample rate, bit depth, and channel count.
```cpp
// usually as member variable
const bool sanitizePath{ true };
jbd::BufferDumper bufferDumper{ juce::File::createFileWithoutCheckingPath("C:\\JuceProject\\dump"), "buffer_dump", sanitizePath };

// usually in prepareToPlay
juce::AudioFormatWriterOptions options{ };
options = options.withSampleRate(getSampleRate());
options = options.withNumChannels(getTotalNumOutputChannels());
bufferDumper.Initialize(options);
```

## Write and Dump
After initializing, Write can be used. `jbd::BufferDumper::Write` works just like `juce::ThreadedWriter::write`. You just need to provide it with an array of write pointers to the channels and the size of the buffer.

Dump can be used whenever you want to write the audio to the next file, Dump will push the remaining data to the current file and setup the writer for the next one. Using Dump does not currently wait for the write to be finished, so loss of data from the last write is possible. Dump does not have to be called before ending the application, the remaining data will always be pushed before quitting.
```cpp
// write audio to the current file
bufferDumper.Write(buffer.getArrayOfWritePointers(), buffer.getNumSamples());

// called whenever you want to write to the next file
bufferDumper.Dump();
```

## Shutdown
Shutdown can be called to stop the background writing thread. Once called, the BufferDumper must be re-initialized using Initialize before any further audio data can be written.
```cpp
// stops the thread and closes the current file
bufferDumper.Shutdown();
```
