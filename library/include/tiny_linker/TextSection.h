#ifndef TINY_LINKER_TEXTSECTION_H
#define TINY_LINKER_TEXTSECTION_H

#include <llvm/BinaryFormat/ELF.h>

#include <memory>
#include <vector>

namespace tiny_linker {
    class TextSectionImpl;

    class TextSection {
    public:
        explicit TextSection(std::unique_ptr<char[]> bytes, size_t size);

        // Читает адрес, начинающийся в позиции position в секции .text
        int ReadAddressAt(size_t position);

        // Записывает адрес в позицию position в секции .text
        void WriteAddressAt(size_t position, int address);

        std::shared_ptr<char[]> GetBytes() const;

        std::size_t GetSize() const;

    private:
        const TextSectionImpl *Pimpl() const { return m_pImpl.get(); }

        TextSectionImpl *Pimpl() { return m_pImpl.get(); }

        std::shared_ptr<TextSectionImpl> m_pImpl;
    };
}


#endif //TINY_LINKER_TEXTSECTION_H
