#ifndef __META_H
#define __META_H

#include "instruction.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace uil {
    class BytecodeStream;

    #pragma pack(push, 1)

    struct executable_header {
        char    magic[6];   // UILUIL
        uint8_t version_major;
        uint8_t version_minor;
        uint16_t header_size;
        uint32_t flags;
        uint32_t code_offset;
        uint32_t code_size;
        uint32_t data_offset;
        uint32_t data_size;
        uint32_t meta_offset;
        uint32_t meta_size;
        uint32_t checksum;
        uint8_t  checksum_type; // Checksum type: 0 = none, 1 = CRC32
        uint64_t entry_offset;  // Offset of main function (relative to file start)
        uint8_t  reserved[7];   // Reserved for future use
    };

    #pragma pack(pop)

    enum executable_header_flags: uint32_t {
        EX_FLAG_DBG     = 1 << 0,
        EX_FLAG_STRIPPED= 1 << 1,
        EX_FLAG_OPTIMIZE= 1 << 2
    };

    #pragma pack(push, 1)

    struct executable_meta_symbol {
        uint32_t name_offset;
        uint32_t type_id;
        uint32_t stack_offset;
        uint32_t flags;
    };

    struct executable_meta_type {
        uint32_t name_offset;
        uint32_t size;
        uint32_t flags;
    };

    struct executable_header_meta {
        uint32_t symbol_count;
        uint32_t type_count;
        uint32_t string_pool_size;
    };

    #pragma pack(pop)

    enum executable_meta_symbol_flags: uint32_t {
        SYM_FLAG_GLOBAL = 1 << 0,
        SYM_FLAG_CONST  = 1 << 1,
        SYM_FLAG_PARAM  = 1 << 2
    };

    enum executable_meta_type_flags: uint8_t {
        TYPE_FLAG_SIGN      = 1 << 0,
        TYPE_FLAG_USER      = 1 << 1,
        TYPE_FLAG_INTERGRAL = 1 << 2,
        TYPE_FLAG_NUMERIC   = 1 << 2
    };

    struct executable_meta {
        std::vector<executable_meta_type> types;
        std::vector<executable_meta_symbol> symbols;
        std::string string_pool;
    };

    bool executable_validate_header(const executable_header& hdr, size_t file_size);
    bool executable_validate_meta_section(const executable_header& hdr, const executable_header_meta& meta, size_t file_size);

    uint32_t executable_string_pool_append(std::string& pool, const std::string& str);

    instruction executable_parse_instruction(BytecodeStream& stream);

    class BytecodeStream {
        private:
        const uint8_t* raw;
        size_t size;
        size_t offset;

        public:
        BytecodeStream(const uint8_t* buffer, size_t buffer_size);

        uint8_t read_byte();
        const uint8_t* read_n_bytes(size_t n);

        uint8_t peek_byte();
        const uint8_t* peek_n_bytes(size_t n);
        uint8_t peek_pos(size_t pos);

        size_t tell() const;
        bool eof() const;

        void reset();

        size_t get_size() const;

        uint16_t read_16();
        uint32_t read_32();
        uint64_t read_64();
    };

    void write_executable(
        const std::string& filename, 
        const std::vector<uint8_t>& code,
        const executable_meta& meta
    );
};

#endif
