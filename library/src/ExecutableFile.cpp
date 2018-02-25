#include <tiny_linker/ExecutableFile.h>

#include "ExecutableFileImpl.cpp"

namespace tiny_linker {
    void ExecutableFile::Write(std::fstream &stream) {
        Pimpl()->Write(stream);
    }

    ExecutableFile::ExecutableFile(std::shared_ptr<tiny_linker::TextSection> textSection) : m_pImpl(
            new ExecutableFileImpl(std::move(textSection))) {}

}