
#include <string>

#include "BufferDump.hpp"


jbd::BufferDumper::BufferDumper(juce::File dumpDirectory, juce::String wavFileName, bool sanitizePath)
	: m_WriterOptions{ }
	, m_DumpDirectory{ dumpDirectory }
	, m_FileName{ wavFileName }
	, m_Counter{ 0 }
	, m_SanitizePath{ sanitizePath }
	, m_DumpOnWrite{ false }
	, m_ThreadedWriter{ nullptr }
	, m_Thread{ "BufferDumpThread" }
{
    if (m_SanitizePath) { 
        SanitizePath(); 
    }
}

void jbd::BufferDumper::Initialize(const juce::AudioFormatWriterOptions& options)
{
    m_WriterOptions = options;

    if (!m_Thread.isThreadRunning()) {
        m_Thread.startThread();
    }

    CreateNextWriter();
}

void jbd::BufferDumper::SanitizePath()
{
    using namespace juce;

    // Check and create legal dump directory
    m_DumpDirectory = File::createLegalPathName(m_DumpDirectory.getFullPathName());

    if (!m_DumpDirectory.exists())
    {
        Result result = m_DumpDirectory.createDirectory();
        if (result.failed())
        {
            Logger::outputDebugString("Failed to create directory: " + result.getErrorMessage());
        }
    }

    // Make legal filename and remove any extension if present
    m_FileName = File::createLegalFileName(m_FileName);
    if (int last{ m_FileName.indexOfChar('.') }; last > -1)
    {
        m_FileName = m_FileName.substring(0, last);
    }
}

void jbd::BufferDumper::CreateNextWriter()
{
    using namespace juce;

    File nextFile = m_DumpDirectory.getChildFile(m_FileName + "_" + std::to_string(m_Counter) + ".wav");

    while (nextFile.exists())
    {
        ++m_Counter;
        nextFile = m_DumpDirectory.getChildFile(m_FileName + "_" + std::to_string(m_Counter) + ".wav");
    }


    WavAudioFormat wavFormat;
    std::unique_ptr<OutputStream> streamToWriteTo{ std::make_unique<FileOutputStream>(nextFile) };
    std::unique_ptr<AudioFormatWriter> writer{ wavFormat.createWriterFor(streamToWriteTo, m_WriterOptions) };

    if (writer != nullptr)
    {
        constexpr int bufferSize{ 44100 };
        m_ThreadedWriter = std::make_unique<AudioFormatWriter::ThreadedWriter>(writer.release(), m_Thread, bufferSize);
    }

    ++m_Counter;
}

void jbd::BufferDumper::Write(const float* const* data, int numSamples)
{
    m_ThreadedWriter->write(data, numSamples);

}

void jbd::BufferDumper::Dump()
{
    m_ThreadedWriter.reset(nullptr);
    CreateNextWriter();
}

void jbd::BufferDumper::Shutdown()
{
    if (m_Thread.isThreadRunning()) 
    {
        // Dump the remaining buffer info
        m_ThreadedWriter.reset(nullptr);

        // Stop thread
        m_Thread.stopThread(1000);
    }
}