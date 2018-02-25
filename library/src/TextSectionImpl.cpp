#include <memory>

namespace tiny_linker {
    class TextSectionImpl {
    private:
        std::shared_ptr<char[]> bytes;
        size_t size;
    public:
        TextSectionImpl(std::unique_ptr<char[]> bytes, size_t size) : bytes(std::move(bytes)), size(size) {

        }

        void WriteAddressAt(size_t position, int address) {
            // Конвертируем в little endian
            for (int i = 0; i < 4; ++i) {
                bytes.get()[position + i] = (char) (address >> (i * 8));
            }
        }

        int ReadAddressAt(size_t position) {
            int address = 0;
            // Конвертируем из little endian
            for (int i = 0; i < 4; ++i) {
                address |= (unsigned) (bytes.get()[position + i] << (8 * i));
            }
            return address;
        }

        std::shared_ptr<char[]> GetBytes() const {
            return bytes;
        }

        std::size_t GetSize() const {
            return size;
        }
    };
}