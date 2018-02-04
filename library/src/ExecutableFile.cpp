#include "tiny_linker/ExecutableFile.h"

#include "ExecutableFileImpl.cpp"

namespace tiny_linker {
    ExecutableFile::ExecutableFile() : m_pImpl(new ExecutableFileImpl()) {

    }

    void ExecutableFile::Write(std::fstream &stream, llvm::ELF::Elf32_Word text_size) {
        Pimpl()->Write(stream, text_size);
    }

}