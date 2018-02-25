#include <tiny_linker/ObjectFile.h>
#include <tiny_linker/ExecutableFile.h>
#include <optional>
#include <iostream>

namespace tiny_linker {
    class LinkerImpl {
    public:
        std::unique_ptr<tiny_linker::ExecutableFile> Link(std::vector<std::shared_ptr<ObjectFile>> objectFiles);
    };

    std::unique_ptr<tiny_linker::ExecutableFile> LinkerImpl::Link(std::vector<std::shared_ptr<ObjectFile>> objectFiles) {
        size_t totalTextSectionSize = 0;
        for (const std::shared_ptr<tiny_linker::ObjectFile> &currentObjectFile : objectFiles) {
            totalTextSectionSize += currentObjectFile->GetTextSection()->GetSize();
        }

        auto resultTextSectionBytes = std::make_unique<char[]>(totalTextSectionSize);
        size_t offset = 0;
        std::vector<size_t> offsets;

        for (const std::shared_ptr<tiny_linker::ObjectFile> &currentObjectFile : objectFiles) {
            offsets.push_back(offset);
            auto currentTextSection = currentObjectFile->GetTextSection();
            auto bytesBegin = currentTextSection->GetBytes().get();
            auto bytesSize = currentTextSection->GetSize();

            std::copy(bytesBegin, bytesBegin + bytesSize, resultTextSectionBytes.get() + offset);
            offset += bytesSize;
        }

        auto resultTextSection = std::make_shared<tiny_linker::TextSection>(std::move(resultTextSectionBytes), totalTextSectionSize);

        for(size_t currentObjectFileIndex = 0; currentObjectFileIndex < objectFiles.size(); ++currentObjectFileIndex) {
            const auto &currentObjectFile = objectFiles[currentObjectFileIndex];
            for (const std::shared_ptr<llvm::ELF::Elf32_Rel> &relocation : currentObjectFile->GetRelocations()) {
                // Relocation - это запись о том, адрес какого символа и куда нужно подставить.

                const auto symbolIndex = relocation->getSymbol(); // Позиция символа в таблице символов.
                const auto &destinationSymbol = currentObjectFile->GetSymbols()[symbolIndex];

                const auto symbolBinding = destinationSymbol->getBinding(); // глобальный символ или локальный
                const auto symbolType = destinationSymbol->getType(); // тип - файл, секция, функция

                std::optional<llvm::ELF::Elf32_Addr> sourceSymbol; // Смещение импортированного символа

                // Объектный файл, из которого импортирован символ
                std::optional<std::shared_ptr<tiny_linker::ObjectFile>> sourceObjectFile;
                std::optional<size_t> sourceObjectFileIndex;

                if (symbolBinding == llvm::ELF::STB_LOCAL && symbolType == llvm::ELF::STT_SECTION) {
                    // Нужно сделать relocation для секции. Например, перемещение секции с данными.
                    continue;
                } else if (symbolBinding == llvm::ELF::STB_GLOBAL) {
                    // Импортирование символа, например. функции.
                    // Получаем имя нужного символа
                    const auto symbolName = currentObjectFile->GetStringTableEntry(destinationSymbol->st_name);
                    std::cout << "Symbol name is: " << symbolName << std::endl;

                    // Проходимся по всем объектным файлам...
                    for(size_t objectFileIndex = 0; objectFileIndex < objectFiles.size(); ++objectFileIndex) {
                        const auto &objectFile = objectFiles[objectFileIndex];
                        // И проверяем, есть ли экпортированный символ с нужным нам именем.
                        for (const std::shared_ptr<llvm::ELF::Elf32_Sym> &symbol : objectFile->GetSymbols()) {
                            const auto sourceSymbolName = objectFile->GetStringTableEntry(symbol->st_name);
                            if (sourceSymbolName == symbolName && symbol->st_shndx != llvm::ELF::SHN_UNDEF &&
                                symbol->getBinding() == llvm::ELF::STB_GLOBAL) {
                                sourceObjectFileIndex = objectFileIndex;
                                sourceSymbol = symbol->st_value;
                            }
                        }
                    }
                }

                if (!sourceSymbol || !sourceObjectFileIndex) {
                    std::cout << "--Symbol was not found!" << std::endl;
                    continue;
                }

                const auto type = relocation->getType();
                // От типа зависит, как будет вычисляться адрес после relocation
                if (type == llvm::ELF::R_386_PC32) { // Записываем новый ОТНОСИТЕЛЬНЫЙ адрес.
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //  |x                       x               |                   |x                         x       | //
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //   |                       |                                    |                         |         //
                    // start of source section   source symbol             start of destination section      destination  //
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //
                    // destination и source symbol известны в виде адресов относительно destination section и source section
                    // destination section и source section известны в виде абсолютных адресов.
                    // относительный адрес source symbol по отношению к destination:
                    // source section + source symbol - destination section - destination;
                    //
                    // в качестве нового адреса используется предыдущий адрес + относительный адрес т.к. для вызова call
                    // необходим адрес со смещением относительно предыдущей операции, а не destination.
                    size_t textSectionDestinationOffset = offsets[currentObjectFileIndex];
                    size_t textSectionSourceOffset = offsets[sourceObjectFileIndex.value()];
                    const auto prevAddress = resultTextSection->ReadAddressAt(textSectionDestinationOffset+ relocation->r_offset);

                    const auto sourceSymbolAddress = textSectionSourceOffset + sourceSymbol.value();
                    const auto destinationAddress = textSectionDestinationOffset + relocation->r_offset;
                    const auto newAddress = prevAddress + sourceSymbolAddress - destinationAddress;

                    std::cout << "--Previous address is " << prevAddress << std::endl;
                    std::cout << "--New address is " << newAddress << std::endl;

                    resultTextSection->WriteAddressAt(textSectionDestinationOffset + relocation->r_offset, (int)newAddress);
                }
            }
        }
        return std::make_unique<tiny_linker::ExecutableFile>(resultTextSection);
    }
}