#ifndef TINY_LINKER_EXECUTABLEFILE_H
#define TINY_LINKER_EXECUTABLEFILE_H

#include <fstream>
#include <memory>

#include <llvm/BinaryFormat/ELF.h>

namespace tiny_linker {
    class TextSection;
    class ExecutableFileImpl;

    class ExecutableFile {
    public:
        explicit ExecutableFile(std::shared_ptr<tiny_linker::TextSection> textSection);

        void Write(std::fstream &stream);

    private:
        const ExecutableFileImpl *Pimpl() const { return m_pImpl.get(); }

        ExecutableFileImpl *Pimpl() { return m_pImpl.get(); }

        std::shared_ptr<ExecutableFileImpl> m_pImpl;
    };
}

#endif //TINY_LINKER_EXECUTABLEFILE_H
