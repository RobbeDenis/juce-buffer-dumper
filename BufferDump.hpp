#pragma once

#include <memory>

#include "JuceHeader.h"

namespace jbd
{
	class BufferDumper
	{
	public:
		/*
		@param dumpDirectory Absolute path to the directory where files will be written to.
		@param wavFileName Name (prefix) of the wave files, no file extension needed.
		@param sanitizePath If true the dumpDirectory and wavFileName will both be purged of illegal characters,
		dumpDirectory will be created if it does not yet exist and wavFileName will have every extension removed. */
		explicit BufferDumper(juce::File dumpDirectory, juce::String fileName, bool sanitizePath = true);

		BufferDumper() = delete;

		// Has to be called to start the write thread
		void Initialize(const juce::AudioFormatWriterOptions& options);
		// Has to be called to shut down the thread and dump the last buffer info into the file
		void Shutdown();

		// Write to the current file
		void Write(const float* const* data, int numSamples);
		/*
		Dump the remaining writer info into the current file, and create the next one.
		Using does not wait on write to be finished, so the last write might not fully finish */
		void Dump();

	private:
		void SanitizePath();
		void CreateNextWriter();

		juce::AudioFormatWriterOptions m_WriterOptions;
		juce::File m_DumpDirectory;
		juce::String m_FileName;
		size_t m_Counter;
		bool m_SanitizePath;

		std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> m_ThreadedWriter;
		juce::TimeSliceThread m_Thread;
	};
}
