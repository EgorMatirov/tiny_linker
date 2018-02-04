#include <tiny_linker/ObjectFile.h>
#include <gtest/gtest.h>

#include <fstream>

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
    EXPECT_TRUE(objectFile.GetStringTableHeader());

    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[0]->st_name), "");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[1]->st_name), "helloworld.asm");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[2]->st_name), "");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[3]->st_name), "pri");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[4]->st_name), "pro");
    EXPECT_EQ(objectFile.GetStringTableEntry(objectFile.GetSymbols()[5]->st_name), "main");
}
