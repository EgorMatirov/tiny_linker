#ifndef PROJECT_EXECUTABLEFILE_H
#define PROJECT_EXECUTABLEFILE_H

#include <fstream>
#include <memory>

#include <llvm/BinaryFormat/ELF.h>

namespace tiny_linker {
    class ExecutableFileImpl;

    class ExecutableFile {
    public:
        explicit ExecutableFile();

        void Write(std::fstream &stream, llvm::ELF::Elf32_Word text_size);

    private:
        const ExecutableFileImpl *Pimpl() const { return m_pImpl.get(); }

        ExecutableFileImpl *Pimpl() { return m_pImpl.get(); }

        std::shared_ptr<ExecutableFileImpl> m_pImpl;
    };
}

#endif //PROJECT_EXECUTABLEFILE_H
