#include "ObjectFileImpl.h"

#include <cassert>

namespace tiny_linker {

    ObjectFileImpl::ObjectFileImpl(const std::shared_ptr<std::ifstream> &DataStream) : DataStream(DataStream) {
        auto elfHeader = getElfHeader(DataStream);

        auto sectionHeaderTableOffset = elfHeader->e_shoff;
        auto sectionHeadersCount = elfHeader->e_shnum;
        auto sectionHeaderStringTableOffset = getSectionHeaderNamesTableOffset(std::move(elfHeader));

        for (int i = 0; i < sectionHeadersCount; ++i) {
            const auto sectionHeader = ReadSectionHeaderAt(sectionHeaderTableOffset, i);
            const auto sectionNameOffset = sectionHeaderStringTableOffset + sectionHeader->sh_name;
            const auto sectionName = ReadStringAt(sectionNameOffset);

            if (sectionName == ".text") {
                TextHeader = sectionHeader;
                Text = std::shared_ptr<char[]>(new char[TextHeader->sh_size]);
                DataStream->seekg(TextHeader->sh_offset);
                DataStream->read(Text.get(), TextHeader->sh_size);
            } else if (sectionName == ".rel.text") {
                SetEntries(sectionHeader, Relocations);
            } else if (sectionName == ".symtab") {
                SetEntries(sectionHeader, Symbols);
            } else if (sectionName == ".strtab") {
                StringTableHeader = sectionHeader;
            }
        }
    }

    std::unique_ptr<llvm::ELF::Elf32_Ehdr>
    ObjectFileImpl::getElfHeader(const std::shared_ptr<std::ifstream> &DataStream) const {
        auto elfHeader = std::make_unique<llvm::ELF::Elf32_Ehdr>();
        DataStream->read((char *) elfHeader.get(), sizeof(*elfHeader));
        return elfHeader;
    }

    llvm::ELF::Elf32_Off
    ObjectFileImpl::getSectionHeaderNamesTableOffset(std::unique_ptr<llvm::ELF::Elf32_Ehdr> elfHeader) {
        auto sectionHeaderStringTable = ReadSectionHeaderAt(elfHeader->e_shoff, elfHeader->e_shstrndx);
        return sectionHeaderStringTable->sh_offset;
    }

    std::shared_ptr<llvm::ELF::Elf32_Shdr> ObjectFileImpl::GetTextHeader() const {
        return TextHeader;
    }

    std::string ObjectFileImpl::GetStringTableEntry(int offset) {
        return ReadStringAt(StringTableHeader->sh_offset + offset);
    }

    int ObjectFileImpl::ReadAddressFromTextSectionAt(int position) {
        int address = 0;
        // Конвертируем из litle endian
        for (int i = 0; i < 4; ++i) {
            address |= (unsigned) (Text.get()[position + i] << (8 * i));
        }
        return address;
    }

    void ObjectFileImpl::WriteAddressToTextSectionAt(int position, int address) {
        // Конвертируем в little endian
        for (int i = 0; i < 4; ++i) {
            Text.get()[position + i] = (char) (address >> (i * 8));
        }
    }

    void ObjectFileImpl::WriteText(std::fstream &stream) {
        stream.write(Text.get(), TextHeader->sh_size);
    }

    std::shared_ptr<llvm::ELF::Elf32_Shdr> ObjectFileImpl::GetStringTableHeader() const {
        return StringTableHeader;
    }

    std::string ObjectFileImpl::ReadStringAt(int position) {
        DataStream->seekg(position);
        std::string result;
        std::getline(*DataStream, result, '\0');
        return result;
    }

    std::shared_ptr<llvm::ELF::Elf32_Shdr>
    ObjectFileImpl::ReadSectionHeaderAt(llvm::ELF::Elf32_Off sectionHeaderTableOffset, int index) {
        auto sectionHeader = std::make_shared<llvm::ELF::Elf32_Shdr>();
        const auto sectionHeaderSize = sizeof(llvm::ELF::Elf32_Shdr);
        const auto pos = sectionHeaderTableOffset + sectionHeaderSize * index;

        assert(sectionHeaderSize == sizeof(llvm::ELF::Elf32_Shdr));

        DataStream->seekg(pos);
        DataStream->read((char *) sectionHeader.get(), sectionHeaderSize);
        return sectionHeader;
    }

    template<typename TEntry>
    void ObjectFileImpl::SetEntries(const std::shared_ptr<llvm::ELF::Elf32_Shdr> &header,
                                    std::vector<std::shared_ptr<TEntry>> &container) {
        container.clear();

        auto from = header->sh_offset;
        auto to = from + header->sh_size;
        auto entrySize = header->sh_entsize;

        assert(entrySize == sizeof(TEntry));

        for (; from != to; from += entrySize) {
            auto entry = std::make_shared<TEntry>();
            DataStream->seekg(from);
            DataStream->read((char *) entry.get(), entrySize);
            container.push_back(entry);
        }
    }
}