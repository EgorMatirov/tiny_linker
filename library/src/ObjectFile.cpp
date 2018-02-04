#include "tiny_linker/ObjectFile.h"

#include "ObjectFileImpl.h"

namespace tiny_linker {
    ObjectFile::ObjectFile(const std::shared_ptr<std::ifstream> &DataStream)
            : m_pImpl(new ObjectFileImpl(DataStream)) {

    }

    std::shared_ptr<llvm::ELF::Elf32_Shdr> ObjectFile::GetTextHeader() const {
        return Pimpl()->GetTextHeader();
    }

    std::string ObjectFile::GetStringTableEntry(int offset) {
        return Pimpl()->GetStringTableEntry(offset);
    }

    int ObjectFile::ReadAddressFromTextSectionAt(int position) {
        return Pimpl()->ReadAddressFromTextSectionAt(position);
    }

    void ObjectFile::WriteAddressToTextSectionAt(int position, int address) {
        Pimpl()->WriteAddressToTextSectionAt(position, address);
    }

    void ObjectFile::WriteText(std::fstream &stream) {
        Pimpl()->WriteText(stream);
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
}