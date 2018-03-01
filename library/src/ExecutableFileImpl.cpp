#include <llvm/BinaryFormat/ELF.h>

#include <fstream>
#include <tiny_linker/TextSection.h>

namespace tiny_linker {

    class ExecutableFileImpl {
    public:
        explicit ExecutableFileImpl(std::shared_ptr<tiny_linker::TextSection> textSection,
                                                        std::vector<char> otherSections,
                                                        size_t entryPointOffset);

        void Write(std::ostream &stream);

        static int SizeOfHeaders();

    private:
        std::shared_ptr<tiny_linker::TextSection> TextSection;
        std::vector<char> OtherSections;
        size_t EntryPointOffset;
    };

    void ExecutableFileImpl::Write(std::ostream &stream) {
        // Заголовок ELF + специфичная информация, которую менять не понадобится.
        char magic[] = {0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        llvm::ELF::Elf32_Addr entry = 0x8048000; // Смещение в виртуальной памяти, по которому будет расположен бинарник.

        llvm::ELF::Elf32_Ehdr elfHeader{};
        memcpy(elfHeader.e_ident, magic, llvm::ELF::EI_NIDENT);
        elfHeader.e_type = llvm::ELF::ET_EXEC; // Исполняемый файл
        elfHeader.e_machine = (llvm::ELF::Elf32_Half) 0x40000003; // i386
        elfHeader.e_version = llvm::ELF::EV_CURRENT; // Версия ELF
        elfHeader.e_phoff = sizeof(llvm::ELF::Elf32_Ehdr); // Программные заголовки - сразу после заголовка ELF
        elfHeader.e_shoff = 0; // Не используем заголовки секций.
        elfHeader.e_flags = 0; // В i686 нет флагов.
        elfHeader.e_ehsize = sizeof(llvm::ELF::Elf32_Ehdr); // размер заголовка ELF.
        elfHeader.e_phentsize = sizeof(llvm::ELF::Elf32_Phdr); // размер каждого программного заголовка
        elfHeader.e_phnum = 1; // количество программных заголовков
        elfHeader.e_shentsize = sizeof(llvm::ELF::Elf32_Shdr); // размер каждого заголовка секции
        elfHeader.e_shnum = 0; // количество заголовков секций
        elfHeader.e_shstrndx = 0; // нет таблицы названий секций

        // Точка входа - мы располагаем код сразу после всех заголовков.
        const auto headersSize = sizeof(llvm::ELF::Elf32_Ehdr) +
                                 sizeof(llvm::ELF::Elf32_Shdr) * elfHeader.e_shnum +
                                 sizeof(llvm::ELF::Elf32_Phdr) * elfHeader.e_phnum;
        elfHeader.e_entry = entry + headersSize + EntryPointOffset;

        stream.write((char *) &elfHeader, sizeof(elfHeader));

        const llvm::ELF::Elf32_Word size = (llvm::ELF::Elf32_Word) TextSection->GetSize() + headersSize;

        llvm::ELF::Elf32_Phdr programHeader{};
        // Данный сегмент - загружаемый - его необходимо загрузить в память
        programHeader.p_type = llvm::ELF::PT_LOAD;
        // Помечаем сегмент как доступный для чтения и исполнения
        programHeader.p_flags = llvm::ELF::PF_R | llvm::ELF::PF_X;
        // Смещение сегмента в файле
        programHeader.p_offset = 0;
        // Виртуальный адрес, по которому распологается сегмент.
        programHeader.p_vaddr = entry;
        // Физический адрес, по которому распологается сегмент. Не актуально, но проще сделать равным vaddr.
        programHeader.p_paddr = programHeader.p_vaddr;
        // Размер (в байтах), занимаемый сегментом _в файле_
        programHeader.p_filesz = size;
        // Размер (в байтах). занимаемый сегментом _в памяти_. Может быть больше, чем размер в файле,
        // если планируется писать что-то в эту память. Лишняя память обнуляется.
        programHeader.p_memsz = programHeader.p_filesz;
        // Выравнивание. TODO: Понятия не имею, почему использовал здесь "size", но и так работает.
        programHeader.p_align = size;

        stream.write((char *) &programHeader, sizeof(programHeader));

        stream.write(TextSection->GetBytes().get(), TextSection->GetSize());
        stream.write(&OtherSections[0], OtherSections.size());
    }

    ExecutableFileImpl::ExecutableFileImpl(std::shared_ptr<tiny_linker::TextSection> textSection,
                                           std::vector<char> otherSections,
                                           size_t entryPointOffset)
            : TextSection(std::move(textSection)),
              OtherSections(std::move(otherSections)),
              EntryPointOffset(entryPointOffset) {}

    int ExecutableFileImpl::SizeOfHeaders() {
        return sizeof(llvm::ELF::Elf32_Ehdr) + sizeof(llvm::ELF::Elf32_Phdr);
    }
}