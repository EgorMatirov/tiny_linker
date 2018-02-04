#ifndef TINY_LINKER_OBJECTFILE_H
#define TINY_LINKER_OBJECTFILE_H

#include <llvm/BinaryFormat/ELF.h>

#include <memory>
#include <vector>

namespace tiny_linker {
    class ObjectFileImpl;

    class ObjectFile {
    public:
        explicit ObjectFile(const std::shared_ptr<std::ifstream> &DataStream);

        std::vector<std::shared_ptr<llvm::ELF::Elf32_Rel>> GetRelocations() const;

        std::vector<std::shared_ptr<llvm::ELF::Elf32_Sym>> GetSymbols() const;

        std::shared_ptr<llvm::ELF::Elf32_Shdr> GetTextHeader() const;

        std::shared_ptr<llvm::ELF::Elf32_Shdr> GetStringTableHeader() const;

        // Возращает строчку по индексу в таблице строк
        std::string GetStringTableEntry(int offset);

        // Читает адрес, начинающийся в позиции position в секции .text
        int ReadAddressFromTextSectionAt(int position);

        // Записывает адрес в позицию position в секции .text
        void WriteAddressToTextSectionAt(int position, int address);

        void WriteText(std::fstream &stream);

    private:
        const ObjectFileImpl *Pimpl() const { return m_pImpl.get(); }

        ObjectFileImpl *Pimpl() { return m_pImpl.get(); }

        std::shared_ptr<ObjectFileImpl> m_pImpl;
    };
}


#endif //TINY_LINKER_OBJECTFILE_H
