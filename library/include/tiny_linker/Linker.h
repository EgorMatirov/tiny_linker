#ifndef TINY_LINKER_LINKER_H
#define TINY_LINKER_LINKER_H

#include <memory>
#include <vector>

namespace tiny_linker {
    class LinkerImpl;

    class ObjectFile;
    class ExecutableFile;

    class Linker {
    public:
        explicit Linker();
        std::unique_ptr<tiny_linker::ExecutableFile> Link(std::vector<std::shared_ptr<ObjectFile>> objectFiles);

    private:
        const LinkerImpl *Pimpl() const { return m_pImpl.get(); }

        LinkerImpl *Pimpl() { return m_pImpl.get(); }

        std::shared_ptr<LinkerImpl> m_pImpl;
    };
}

#endif //TINY_LINKER_LINKER_H
