#include <tiny_linker/TextSection.h>

#include "TextSectionImpl.cpp"

namespace tiny_linker {
    TextSection::TextSection(std::unique_ptr<char[]> bytes, size_t size) : m_pImpl(
            new TextSectionImpl(std::move(bytes), size)) {

    }

    void TextSection::WriteAddressAt(size_t position, int address) {
        Pimpl()->WriteAddressAt(position, address);
    }

    int TextSection::ReadAddressAt(size_t position) {
        return Pimpl()->ReadAddressAt(position);
    }

    std::shared_ptr<char[]> TextSection::GetBytes() const {
        return Pimpl()->GetBytes();
    }

    std::size_t TextSection::GetSize() const {
        return Pimpl()->GetSize();
    }
}