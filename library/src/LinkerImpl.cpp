#include <tiny_linker/ObjectFile.h>
#include <tiny_linker/ExecutableFile.h>
#include <optional>
#include <iostream>
#include <numeric>

namespace tiny_linker {
    class LinkerImpl {
    public:
        std::unique_ptr<tiny_linker::ExecutableFile> Link(std::vector<std::shared_ptr<ObjectFile>> objectFiles);

        std::vector<int> GetOffsets(const std::vector<std::shared_ptr<ObjectFile>> &objectFiles) const;

        std::shared_ptr<TextSection>
        CreateResultTextSection(const std::vector<std::shared_ptr<ObjectFile>> &objectFiles) const;
    };

    std::unique_ptr<tiny_linker::ExecutableFile>
    LinkerImpl::Link(std::vector<std::shared_ptr<ObjectFile>> objectFiles) {

        // Создаём результирующую .text секцию - это .text секции всех объектныхх файлов, склееные в одну.
        auto resultTextSection = CreateResultTextSection(objectFiles);
        // Смещения .text секций конкретных объектных файлов относительно начала результирующей .text секции.
        auto offsets = GetOffsets(objectFiles);

        for (size_t currentObjectFileIndex = 0; currentObjectFileIndex < objectFiles.size(); ++currentObjectFileIndex) {
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
                    const auto &symbolName = currentObjectFile->GetStringTableEntry(destinationSymbol->st_name);
                    std::cout << "Symbol name is: " << symbolName << std::endl;

                    // Проходимся по всем объектным файлам...
                    for (size_t objectFileIndex = 0; objectFileIndex < objectFiles.size(); ++objectFileIndex) {
                        const auto &objectFile = objectFiles[objectFileIndex];
                        // И проверяем, есть ли экпортированный символ с нужным нам именем.
                        for (const std::shared_ptr<llvm::ELF::Elf32_Sym> &symbol : objectFile->GetSymbols()) {
                            const auto &sourceSymbolName = objectFile->GetStringTableEntry(symbol->st_name);
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
                    const int textSectionDestinationOffset = offsets[currentObjectFileIndex];
                    const int textSectionSourceOffset = offsets[sourceObjectFileIndex.value()];

                    const auto prevAddress = resultTextSection->ReadAddressAt(
                            textSectionDestinationOffset + relocation->r_offset);

                    const int sourceSymbolAddress = textSectionSourceOffset + sourceSymbol.value();
                    const int destinationAddress = textSectionDestinationOffset + relocation->r_offset;
                    const int newAddress = prevAddress + sourceSymbolAddress - destinationAddress;

                    std::cout << "--Previous address is " << prevAddress << std::endl;
                    std::cout << "--Text section destination offset is " << textSectionDestinationOffset << std::endl;
                    std::cout << "--Text section source offset is " << textSectionSourceOffset << std::endl;
                    std::cout << "--Source symbol address is " << sourceSymbolAddress << std::endl;
                    std::cout << "--Destination address is " << destinationAddress << std::endl;
                    std::cout << "--New address is " << newAddress << std::endl;

                    resultTextSection->WriteAddressAt(textSectionDestinationOffset + relocation->r_offset, newAddress);
                }
            }
        }

        std::optional<size_t> entryPointOffset;
        for (size_t currentObjectFileIndex = 0; currentObjectFileIndex < objectFiles.size(); ++currentObjectFileIndex) {
            const auto &currentObjectFile = objectFiles[currentObjectFileIndex];
            for (const std::shared_ptr<llvm::ELF::Elf32_Sym> &symbol : currentObjectFile->GetSymbols()) {
                const auto sourceSymbolName = currentObjectFile->GetStringTableEntry(symbol->st_name);
                if (sourceSymbolName == "main" && symbol->getBinding() == llvm::ELF::STB_GLOBAL) {
                    entryPointOffset = offsets[currentObjectFileIndex] + symbol->st_value;
                }
            }
        }

        if (!entryPointOffset) {
            std::cout << "Cannot find entry point!";
            return nullptr;
        }

        return std::make_unique<tiny_linker::ExecutableFile>(resultTextSection, entryPointOffset.value());
    }

    std::shared_ptr<TextSection>
    LinkerImpl::CreateResultTextSection(const std::vector<std::shared_ptr<ObjectFile>> &objectFiles) const {
        size_t totalTextSectionSize = std::accumulate(objectFiles.begin(), objectFiles.end(), (size_t) 0,
                                                      [](size_t prev, auto objectFile) {
                                                          return prev + objectFile->GetTextSection()->GetSize();
                                                      }
        );

        auto resultTextSectionBytes = std::make_unique<char[]>(totalTextSectionSize);

        { // Копируем содержимое всех .text секций в result
            int offset = 0;
            for (const std::shared_ptr<ObjectFile> &currentObjectFile : objectFiles) {
                auto currentTextSection = currentObjectFile->GetTextSection();
                auto bytesBegin = currentTextSection->GetBytes().get();
                auto bytesSize = currentTextSection->GetSize();

                std::copy(bytesBegin, bytesBegin + bytesSize, resultTextSectionBytes.get() + offset);
                offset += bytesSize;
            }
        }

        return std::make_shared<TextSection>(move(resultTextSectionBytes), totalTextSectionSize);
    }

    std::vector<int> LinkerImpl::GetOffsets(const std::vector<std::shared_ptr<ObjectFile>> &objectFiles) const {
        std::vector<int> offsets;
        int offset = 0;
        for (const std::shared_ptr<ObjectFile> &currentObjectFile : objectFiles) {
            offsets.push_back(offset);
            offset += currentObjectFile->GetTextSection()->GetSize();
        }
        return offsets;
    }
}