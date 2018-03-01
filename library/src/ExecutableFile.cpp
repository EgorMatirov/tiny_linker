#include <tiny_linker/ExecutableFile.h>

#include "ExecutableFileImpl.cpp"

namespace tiny_linker {
    void ExecutableFile::Write(std::ostream &stream) {
        Pimpl()->Write(stream);
    }

    ExecutableFile::ExecutableFile(std::shared_ptr<tiny_linker::TextSection> textSection,
                                   std::vector<char> otherSections, size_t entryPointOffset)
            : m_pImpl(new ExecutableFileImpl(std::move(textSection), otherSections, entryPointOffset)) {}

    int ExecutableFile::SizeOfHeaders() {
        return ExecutableFileImpl::SizeOfHeaders();
    }

}