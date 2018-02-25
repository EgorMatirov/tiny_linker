#include "ObjectFileImpl.h"

#include <cassert>

namespace tiny_linker {

    ObjectFileImpl::ObjectFileImpl(const std::shared_ptr<std::ifstream> &dataStream) : DataStream(dataStream) {
        auto elfHeader = GetElfHeader(dataStream);

        auto sectionHeaderTableOffset = elfHeader->e_shoff;
        auto sectionHeadersCount = elfHeader->e_shnum;
        auto sectionHeaderStringTableOffset = GetSectionHeaderNamesTableOffset(std::move(elfHeader));

        for (int i = 0; i < sectionHeadersCount; ++i) {
            const auto sectionHeader = ReadSectionHeaderAt(sectionHeaderTableOffset, i);
            const auto sectionNameOffset = sectionHeaderStringTableOffset + sectionHeader->sh_name;
            const auto sectionName = ReadStringAt(sectionNameOffset);

            if (sectionName == ".text") {
                ReadTextSection(dataStream, sectionHeader);
            } else if (sectionName == ".rel.text") {
                SetEntries(sectionHeader, Relocations);
            } else if (sectionName == ".symtab") {
                SetEntries(sectionHeader, Symbols);
            } else if (sectionName == ".strtab") {
                StringTableHeader = sectionHeader;
            }
        }
    }

    void ObjectFileImpl::ReadTextSection(const std::shared_ptr<std::ifstream> &dataStream,
                                         const std::shared_ptr<llvm::ELF::Elf32_Shdr> &textSectionHeader) {
        auto text = std::unique_ptr<char[]>(new char[textSectionHeader->sh_size]);
        dataStream->seekg(textSectionHeader->sh_offset);
        dataStream->read(text.get(), textSectionHeader->sh_size);

        TextSection = std::make_shared<tiny_linker::TextSection>(move(text), textSectionHeader->sh_size);
    }

    std::unique_ptr<llvm::ELF::Elf32_Ehdr>
    ObjectFileImpl::GetElfHeader(const std::shared_ptr<std::ifstream> &dataStream) const {
        auto elfHeader = std::make_unique<llvm::ELF::Elf32_Ehdr>();
        dataStream->read((char *) elfHeader.get(), sizeof(*elfHeader));
        return elfHeader;
    }

    llvm::ELF::Elf32_Off
    ObjectFileImpl::GetSectionHeaderNamesTableOffset(std::unique_ptr<llvm::ELF::Elf32_Ehdr> elfHeader) {
        auto sectionHeaderStringTable = ReadSectionHeaderAt(elfHeader->e_shoff, elfHeader->e_shstrndx);
        return sectionHeaderStringTable->sh_offset;
    }

    std::string ObjectFileImpl::GetStringTableEntry(int offset) {
        return ReadStringAt(StringTableHeader->sh_offset + offset);
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

    std::shared_ptr<TextSection> ObjectFileImpl::GetTextSection() const {
        return TextSection;
    }
}