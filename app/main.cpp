#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include <cstring>
#include <optional>

#include <tiny_linker/ObjectFile.h>
#include <tiny_linker/ExecutableFile.h>

int main(int argc, char **argv) {
    std::fstream output;
    output.open("output", std::fstream::out | std::fstream::binary);

    std::vector<std::shared_ptr<tiny_linker::ObjectFile>> objectFiles;

    for (int i = 1; i < argc; ++i) {
        std::shared_ptr<std::ifstream> dataStream = std::make_shared<std::ifstream>(argv[i]);
        std::shared_ptr<tiny_linker::ObjectFile> objectFile = std::make_shared<tiny_linker::ObjectFile>(dataStream);
        objectFiles.push_back(objectFile);
    }

    for (const std::shared_ptr<tiny_linker::ObjectFile> &currentObjectFile : objectFiles) {
        for (const std::shared_ptr<llvm::ELF::Elf32_Rel> &relocation : currentObjectFile->GetRelocations()) {
            const auto symbolIndex = relocation->getSymbol(); // Позиция символа в таблице символов.
            const auto &symbol = currentObjectFile->GetSymbols()[symbolIndex];

            const auto symbolBinding = symbol->getBinding(); // глобальный символ или локальный
            const auto symbolType = symbol->getType(); // тип - файл, секция, функция

            std::optional<llvm::ELF::Elf32_Addr> symbolOffset; // Смещение импортированного символа

            // Объектный файл, из которого импортирован символ
            std::optional<std::shared_ptr<tiny_linker::ObjectFile>> importedObjectFile;

            if (symbolBinding == llvm::ELF::STB_LOCAL && symbolType == llvm::ELF::STT_SECTION) {
                // Нужно сделать relocation для секции. Например, перемещение секции с данными.
                continue;
            } else if (symbolBinding == llvm::ELF::STB_GLOBAL)// Импортирование символа, например. функции.
            {
                const auto symbolName = currentObjectFile->GetStringTableEntry(
                        symbol->st_name);// Получаем имя нужного символа
                std::cout << "Symbol name is: " << symbolName << std::endl;

                for (const auto &objectFile : objectFiles) {
                    // Проверяем, есть ли экпортированный символ с нужным нам именем.
                    for (const std::shared_ptr<llvm::ELF::Elf32_Sym> &exportedSymbol : objectFile->GetSymbols()) {
                        const auto importedSymbolName = objectFile->GetStringTableEntry(exportedSymbol->st_name);
                        if (importedSymbolName == symbolName && exportedSymbol->st_shndx != llvm::ELF::SHN_UNDEF) {
                            importedObjectFile = objectFile;
                            symbolOffset = exportedSymbol->st_value;
                        }
                    }
                }
            }

            if (!symbolOffset || !importedObjectFile) {
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
                const auto prevAddress = currentObjectFile->ReadAddressFromTextSectionAt(relocation->r_offset);

                int textSectionDestinationOffset = 0;
                for (int i = 0; objectFiles[i] != currentObjectFile; ++i)
                    textSectionDestinationOffset += objectFiles[i]->GetTextHeader()->sh_size;

                int textSectionSourceOffset = 0;
                for (int i = 0; objectFiles[i] != importedObjectFile; ++i)
                    textSectionSourceOffset += objectFiles[i]->GetTextHeader()->sh_size;

                const auto sourceSymbolAddress = textSectionSourceOffset + symbolOffset.value();
                const auto destinationAddress = textSectionDestinationOffset + relocation->r_offset;
                const auto newAddress = prevAddress + sourceSymbolAddress - destinationAddress;

                std::cout << "--Previous address is " << prevAddress << std::endl;
                std::cout << "--New address is " << newAddress << std::endl;

                currentObjectFile->WriteAddressToTextSectionAt(relocation->r_offset, newAddress);
            }
        }
    }

    llvm::ELF::Elf32_Word textSize = 0;
    for (const auto &objectFile : objectFiles) {
        textSize += objectFile->GetTextHeader()->sh_size;
    }

    tiny_linker::ExecutableFile executableFile;
    executableFile.Write(output, textSize);

    for (const auto &objectFile : objectFiles) {
        objectFile->WriteText(output);
    }

    output.close();
    return 0;
}