#ifndef TINY_LINKER_OBJECTFILEIMPL_H
#define TINY_LINKER_OBJECTFILEIMPL_H

#include <llvm/BinaryFormat/ELF.h>
#include <memory>
#include <fstream>
#include <vector>
#include <tiny_linker/TextSection.h>


namespace tiny_linker {
    class ObjectFileImpl {
        std::shared_ptr<llvm::ELF::Elf32_Shdr> StringTableHeader = nullptr;
        std::shared_ptr<std::ifstream> DataStream = nullptr;
        std::shared_ptr<TextSection> TextSection;

        std::shared_ptr<llvm::ELF::Elf32_Shdr>
        ReadSectionHeaderAt(llvm::ELF::Elf32_Off sectionHeaderTableOffset, int index);

        template<typename TEntry>
        void SetEntries(const std::shared_ptr<llvm::ELF::Elf32_Shdr> &header,
                        std::vector<std::shared_ptr<TEntry>> &container);

        // Читает строчку, начиная с данной позиции и до первого символа \0.
        std::string ReadStringAt(int position);

        llvm::ELF::Elf32_Off GetSectionHeaderNamesTableOffset(std::unique_ptr<llvm::ELF::Elf32_Ehdr> elfHeader);

        void ReadTextSection(const std::shared_ptr<std::ifstream> &dataStream,
                             const std::shared_ptr<llvm::ELF::Elf32_Shdr> &textSectionHeader);

    public:
        std::vector<std::shared_ptr<llvm::ELF::Elf32_Rel>> Relocations;
        std::vector<std::shared_ptr<llvm::ELF::Elf32_Sym>> Symbols;

        ~ObjectFileImpl() = default;

        explicit ObjectFileImpl(const std::shared_ptr<std::ifstream> &dataStream);

        std::unique_ptr<llvm::ELF::Elf32_Ehdr> GetElfHeader(const std::shared_ptr<std::ifstream> &dataStream) const;

        std::shared_ptr<llvm::ELF::Elf32_Shdr> GetStringTableHeader() const;

        std::shared_ptr<tiny_linker::TextSection> GetTextSection() const;

        // Возращает строчку по смещению в таблице строк
        std::string GetStringTableEntry(int offset);
    };
}

#endif //TINY_LINKER_OBJECTFILEIMPL_H
