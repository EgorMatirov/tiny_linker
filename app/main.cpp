#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include <cstring>
#include <optional>

#include <tiny_linker/ObjectFile.h>
#include <tiny_linker/ExecutableFile.h>
#include <tiny_linker/Linker.h>

int main(int argc, char **argv) {
    std::fstream output;
    output.open("output", std::fstream::out | std::fstream::binary);

    std::vector<std::shared_ptr<tiny_linker::ObjectFile>> objectFiles;

    // Загружаем объектные файлы и парсим их
    for (int i = 1; i < argc; ++i) {
        std::shared_ptr<std::ifstream> dataStream = std::make_shared<std::ifstream>(argv[i]);
        std::shared_ptr<tiny_linker::ObjectFile> objectFile = std::make_shared<tiny_linker::ObjectFile>(dataStream);
        objectFiles.push_back(objectFile);
    }

    // Линкуем.
    tiny_linker::Linker linker;
    auto executableFile = linker.Link(objectFiles);
    executableFile->Write(output);
    output.close();
    return 0;
}