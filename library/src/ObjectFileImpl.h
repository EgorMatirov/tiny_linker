#ifndef TINY_LINKER_OBJECTFILEIMPL_H
#define TINY_LINKER_OBJECTFILEIMPL_H

#include <llvm/BinaryFormat/ELF.h>
#include <memory>
#include <fstream>
#include <vector>


namespace tiny_linker {
    class ObjectFileImpl {
        std::shared_ptr<llvm::ELF::Elf32_Shdr> TextHeader = nullptr;
        std::shared_ptr<llvm::ELF::Elf32_Shdr> StringTableHeader = nullptr;
        std::shared_ptr<std::ifstream> DataStream = nullptr;
        std::shared_ptr<char[]> Text = nullptr;

        std::shared_ptr<llvm::ELF::Elf32_Shdr>
        ReadSectionHeaderAt(llvm::ELF::Elf32_Off sectionHeaderTableOffset, int index);

        template<typename TEntry>
        void SetEntries(const std::shared_ptr<llvm::ELF::Elf32_Shdr> &header,
                        std::vector<std::shared_ptr<TEntry>> &container);

        // Читает строчку, начиная с данной позиции и до первого символа \0.
        std::string ReadStringAt(int position);

        llvm::ELF::Elf32_Off getSectionHeaderNamesTableOffset(std::unique_ptr<llvm::ELF::Elf32_Ehdr> elfHeader);

    public:

        std::vector<std::shared_ptr<llvm::ELF::Elf32_Rel>> Relocations;
        std::vector<std::shared_ptr<llvm::ELF::Elf32_Sym>> Symbols;

        ~ObjectFileImpl() = default;

        explicit ObjectFileImpl(const std::shared_ptr<std::ifstream> &DataStream);

        std::shared_ptr<llvm::ELF::Elf32_Shdr> GetTextHeader() const;

        std::shared_ptr<llvm::ELF::Elf32_Shdr> GetStringTableHeader() const;

        // Возращает строчку по смещению в таблице строк
        std::string GetStringTableEntry(int offset);

        // Читает адрес, начинающийся в позиции position в секции .text
        int ReadAddressFromTextSectionAt(int position);

        // Записывает адрес в позицию position в секции .text
        void WriteAddressToTextSectionAt(int position, int address);

        void WriteText(std::fstream &stream);

        std::unique_ptr<llvm::ELF::Elf32_Ehdr> getElfHeader(const std::shared_ptr<std::ifstream> &DataStream) const;
    };
}

#endif //TINY_LINKER_OBJECTFILEIMPL_H
