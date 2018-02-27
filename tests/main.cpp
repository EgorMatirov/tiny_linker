#include <tiny_linker/ObjectFile.h>
#include <tiny_linker/Linker.h>
#include <tiny_linker/ExecutableFile.h>

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(object_file, read_symbols_correctly) {
    auto dataStream = std::make_shared<std::ifstream>("../../tests/data/helloworld.o", std::ifstream::binary);
    tiny_linker::ObjectFile objectFile(dataStream);
    EXPECT_EQ(objectFile.GetSymbols().size(), 6);

    EXPECT_EQ(objectFile.GetSymbols()[0]->st_name, 0);
    EXPECT_EQ(objectFile.GetSymbols()[1]->st_name, 1);
    EXPECT_EQ(objectFile.GetSymbols()[2]->st_name, 0);
    EXPECT_EQ(objectFile.GetSymbols()[3]->st_name, 16);
    EXPECT_EQ(objectFile.GetSymbols()[4]->st_name, 20);
    EXPECT_EQ(objectFile.GetSymbols()[5]->st_name, 24);

    EXPECT_EQ(objectFile.GetSymbols()[0]->st_value, 0);
    EXPECT_EQ(objectFile.GetSymbols()[1]->st_value, 0);
    EXPECT_EQ(objectFile.GetSymbols()[2]->st_value, 0);
    EXPECT_EQ(objectFile.GetSymbols()[3]->st_value, 0);
    EXPECT_EQ(objectFile.GetSymbols()[4]->st_value, 0);
    EXPECT_EQ(objectFile.GetSymbols()[5]->st_value, 0);

    EXPECT_EQ(objectFile.GetSymbols()[0]->st_shndx, llvm::ELF::SHN_UNDEF);
    EXPECT_EQ(objectFile.GetSymbols()[1]->st_shndx, llvm::ELF::SHN_ABS);
    EXPECT_EQ(objectFile.GetSymbols()[2]->st_shndx, 1);
    EXPECT_EQ(objectFile.GetSymbols()[3]->st_shndx, llvm::ELF::SHN_UNDEF);
    EXPECT_EQ(objectFile.GetSymbols()[4]->st_shndx, llvm::ELF::SHN_UNDEF);
    EXPECT_EQ(objectFile.GetSymbols()[5]->st_shndx, 1);
}

TEST(object_file, read_symbols_names_correctly) {
    auto dataStream = std::make_shared<std::ifstream>("../../tests/data/helloworld.o", std::ifstream::binary);
    tiny_linker::ObjectFile objectFile(dataStream);

    EXPECT_EQ(objectFile.GetSymbols().size(), 6);
    EXPECT_TRUE(objectFile.GetStringTableHeader() != nullptr);

    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[0]->st_name), "");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[1]->st_name), "helloworld.asm");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[2]->st_name), "");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[3]->st_name), "pri");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[4]->st_name), "pro");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[5]->st_name), "main");
}

TEST(object_file, read_relocations_correctly) {
    auto dataStream = std::make_shared<std::ifstream>("../../tests/data/helloworld.o", std::ifstream::binary);
    tiny_linker::ObjectFile objectFile(dataStream);

    EXPECT_EQ(objectFile.GetRelocations().size(), 2);

    EXPECT_EQ(objectFile.GetRelocations()[0]->r_offset, 1);
    EXPECT_EQ(objectFile.GetRelocations()[1]->r_offset, 6);

    EXPECT_EQ(objectFile.GetRelocations()[0]->getSymbol(), 3);
    EXPECT_EQ(objectFile.GetRelocations()[1]->getSymbol(), 4);
}

TEST(linker, link_correctly_when_main_is_first) {
    const std::vector<std::string> objectFilePaths = {
            "../../tests/data/1/input/helloworld.o",
            "../../tests/data/1/input/pri.o"
    };

    const auto expectedFilePath = "../../tests/data/1/expected_output";
    std::shared_ptr<std::ifstream> expected = std::make_shared<std::ifstream>(expectedFilePath, std::ifstream::binary);

    std::vector<std::shared_ptr<tiny_linker::ObjectFile>> objectFiles;

    // Загружаем объектные файлы и парсим их
    for (const auto &objectFilePath : objectFilePaths) {
        std::shared_ptr<std::ifstream> dataStream = std::make_shared<std::ifstream>(objectFilePath,
                                                                                    std::ifstream::binary);
        std::shared_ptr<tiny_linker::ObjectFile> objectFile = std::make_shared<tiny_linker::ObjectFile>(dataStream);
        objectFiles.push_back(objectFile);
    }

    // Линкуем.
    tiny_linker::Linker linker;
    auto executableFile = linker.Link(objectFiles);

    std::stringstream result(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    executableFile->Write(result);

    result.seekg(0);

    while (!result.eof() && !expected->eof()) {
        auto resultInt = result.get();
        auto expectedInt = expected->get();

        EXPECT_EQ(resultInt, expectedInt);
    }

    EXPECT_EQ(result.eof(), expected->eof());
}
