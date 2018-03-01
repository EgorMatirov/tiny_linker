#include "tiny_linker/ObjectFile.h"

#include "ObjectFileImpl.h"

namespace tiny_linker {
    ObjectFile::ObjectFile(const std::shared_ptr<std::ifstream> &DataStream)
            : m_pImpl(new ObjectFileImpl(DataStream)) {}

    std::string ObjectFile::GetStringTableEntry(int offset) {
        return Pimpl()->GetStringTableEntry(offset);
    }

    std::vector<std::shared_ptr<llvm::ELF::Elf32_Rel>> ObjectFile::GetRelocations() const {
        return Pimpl()->Relocations;
    }

    std::vector<std::shared_ptr<llvm::ELF::Elf32_Sym>> ObjectFile::GetSymbols() const {
        return Pimpl()->Symbols;
    }

    std::shared_ptr<llvm::ELF::Elf32_Shdr> ObjectFile::GetStringTableHeader() const {
        return Pimpl()->GetStringTableHeader();
    }

    std::shared_ptr<TextSection> ObjectFile::GetTextSection() const {
        return Pimpl()->GetTextSection();
    }

    std::vector<char> ObjectFile::GetSectionByIndex(const int index) {
        return Pimpl()->GetSectionByIndex(index);
    }
}