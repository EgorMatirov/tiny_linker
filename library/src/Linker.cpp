#include <tiny_linker/Linker.h>
#include <tiny_linker/ExecutableFile.h>

#include "LinkerImpl.cpp"

namespace tiny_linker {
    std::unique_ptr<tiny_linker::ExecutableFile> Linker::Link(std::vector<std::shared_ptr<ObjectFile>> objectFiles) {
        return std::move(Pimpl()->Link(std::move(objectFiles)));
    }

    Linker::Linker() : m_pImpl(new LinkerImpl()) {}
}