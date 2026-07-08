#include "executable.hpp"
#include "instruction.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace uil {
    instruction executable_parse_instruction(BytecodeStream& stream) {
        instruction ins;
        ins.opcode = static_cast<instruction_opcode>(stream.read_16());

        for(int i = 0; i < INSTRUCTION_MAX_OPERANDS; i++) {
            uint8_t operand_type_raw  = stream.read_byte();
            instruction_operand_type operand_type = instruction_operand_type::NULLOP;
            if(operand_type_raw == 0b0000) operand_type = instruction_operand_type::NULLOP;
            if(operand_type_raw == 0b0001) operand_type = instruction_operand_type::REGISTER;
            if(operand_type_raw == 0b0010) operand_type = instruction_operand_type::IMMEDIATE;
            if(operand_type_raw == 0b0100) operand_type = instruction_operand_type::ADDRESS;
            if(operand_type_raw == 0b1000) operand_type = instruction_operand_type::CONST;

            uint32_t operand_data = static_cast<uint32_t>(stream.read_32());

            ins.operands[i] = {
                .type = operand_type,
                .data = operand_data
            };
        }

        return ins;
    }

    bool executable_validate_header(const executable_header& hdr, size_t file_size) {
        if(std::memcmp(hdr.magic, "UILUIL", 6) != 0)
            return false;
        if(hdr.version_major != 1)
            return false;
        if(hdr.code_offset + hdr.code_size > file_size)
            return false;
        if(hdr.meta_offset + hdr.meta_size > file_size)
            return false;

        return true;
    }

    bool executable_validate_meta_section(const executable_header& hdr, const executable_header_meta& meta, size_t file_size) {
        if(hdr.meta_offset + sizeof(executable_header_meta) > file_size)
            return false;

        size_t symbols_size = meta.symbol_count * sizeof(executable_meta_symbol);
        size_t types_size = meta.type_count * sizeof(executable_meta_type);

        if(hdr.meta_offset + sizeof(executable_header_meta) + symbols_size + types_size + meta.string_pool_size > file_size)
            return false;

        return true;
    }

    uint32_t executable_string_pool_append(std::string& pool, const std::string& str) {
        uint32_t offset = pool.size();
        pool += str;
        pool += '\0';

        return offset;
    }

    BytecodeStream::BytecodeStream(const uint8_t* buffer, size_t buffer_size)
    : offset(0) {
        if(buffer == nullptr || buffer_size < 1)
            throw std::runtime_error("Invalid buffer");

        this->raw = buffer;
        this->size = buffer_size;
    }

    uint8_t BytecodeStream::read_byte() {
        if(this->offset >= this->size)
            throw std::runtime_error("buffer overflow");

        return this->raw[this->offset++];
    }

    const uint8_t* BytecodeStream::read_n_bytes(size_t n) {
        if(this->offset + n > this->size)
            throw std::runtime_error("buffer overflow");

        const uint8_t* ptr = &this->raw[this->offset];
        this->offset += n;

        return ptr;
    }

    uint8_t BytecodeStream::peek_byte() {
        if(this->offset >= this->size)
            throw std::runtime_error("buffer overflow");

        return this->raw[this->offset];
    }

    const uint8_t* BytecodeStream::peek_n_bytes(size_t n) {
        if(this->offset + n > this->size)
            throw std::runtime_error("buffer overflow");

        const uint8_t* ptr = &this->raw[offset];
        return ptr;
    }

    uint8_t BytecodeStream::peek_pos(size_t pos) {
        if(pos >= this->size)
            throw std::runtime_error("buffer overflow");

        return this->raw[pos];
    }

    void BytecodeStream::seek(size_t pos) {
        if(pos >= this->size)
            throw std::runtime_error("buffer overflow");

        this->offset = pos;
    }

    size_t BytecodeStream::tell() const {
        return this->offset;
    }

    bool BytecodeStream::eof() const {
        return this->offset >= this->size;
    }

    void BytecodeStream::reset() {
        this->offset = 0;
    }

    size_t BytecodeStream::get_size() const {
        return this->size;
    }

    uint16_t BytecodeStream::read_16() {
        uint16_t val = 0;

        val |= static_cast<uint16_t>(read_byte());
        val |= static_cast<uint16_t>(read_byte()) << 8;

        return val;
    }

    uint32_t BytecodeStream::read_32() {
        uint32_t val = 0;

        val |= static_cast<uint32_t>(read_byte());
        val |= static_cast<uint32_t>(read_byte()) << 8;
        val |= static_cast<uint32_t>(read_byte()) << 16;
        val |= static_cast<uint32_t>(read_byte()) << 24;

        return val;
    }

    uint64_t BytecodeStream::read_64() {
        uint64_t val = 0;

        val |= static_cast<uint64_t>(read_byte());
        val |= static_cast<uint64_t>(read_byte()) << 8;
        val |= static_cast<uint64_t>(read_byte()) << 16;
        val |= static_cast<uint64_t>(read_byte()) << 24;
        val |= static_cast<uint64_t>(read_byte()) << 32;
        val |= static_cast<uint64_t>(read_byte()) << 40;
        val |= static_cast<uint64_t>(read_byte()) << 48;
        val |= static_cast<uint64_t>(read_byte()) << 56;

        return val;
    }

    static executable_header build_program_header(uint32_t prog_size, uint32_t meta_size) {
        executable_header hdr {};
        std::memcpy(hdr.magic, "UILUIL", 6);

        hdr.version_major = 1;
        hdr.version_minor = 0;

        hdr.code_offset = sizeof(executable_header);
        hdr.code_size = prog_size;
        hdr.meta_offset = hdr.code_offset + hdr.code_size;
        hdr.meta_size   = sizeof(executable_header_meta);

        hdr.flags = 0;
        hdr.checksum = 0; // TODO: CRC32 checksum

        return hdr;
    }

    void write_executable(const std::string& filename, const std::vector<uint8_t>& code, const executable_meta& meta) {
        uint32_t meta_size =    sizeof(executable_meta)         + meta.types.size()   * 
                                sizeof(executable_meta_type)    + meta.symbols.size() * 
                                sizeof(executable_meta_symbol)  + meta.string_pool.size();

        executable_header hdr = build_program_header(code.size(), meta_size);

        std::ofstream fout(filename, std::ios::binary);
        fout.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));

        // 1. Code
        fout.write(reinterpret_cast<const char*>(code.data()), code.size());

        // 2. Meta
        executable_header_meta meta_hdr{};
        meta_hdr.symbol_count       = meta.symbols.size();
        meta_hdr.type_count         = meta.types.size();
        meta_hdr.string_pool_size   = meta.string_pool.size();
        fout.write(reinterpret_cast<char*>(&meta_hdr), sizeof(meta_hdr));

        // 2a. Types
        fout.write(reinterpret_cast<const char*>(meta.types.data()), meta.types.size() * sizeof(executable_meta_type));

        // 2b. Symbols
        fout.write(reinterpret_cast<const char*>(meta.symbols.data()), meta.symbols.size() * sizeof(executable_meta_symbol));

        // 2c. String pool
        fout.write(meta.string_pool.data(), meta.string_pool.size());

        fout.close();
    }

    executable_header read_header(BytecodeStream& stream) {
        executable_header hdr;

        for(size_t i = 0; i < 6; i++)
            hdr.magic[i] = stream.read_byte();

        hdr.version_major = stream.read_byte();
        hdr.version_minor = stream.read_byte();
        hdr.header_size =   stream.read_16();
        hdr.flags =         stream.read_32();
        hdr.code_offset =   stream.read_32();
        hdr.code_size =     stream.read_32();
        hdr.data_offset =   stream.read_32();
        hdr.data_size =     stream.read_32();
        hdr.meta_offset =   stream.read_32();
        hdr.meta_size =     stream.read_32();
        hdr.checksum =      stream.read_32();
        hdr.checksum_type = stream.read_byte();
        hdr.entry_offset =  stream.read_64();

        for(size_t i = 0; i < 7; i++)
            hdr.reserved[i] = stream.read_byte();

        return hdr;
    }

    executable_image load_executable(const std::string& filename) {
        if(filename.empty())
            throw std::runtime_error("No input file specified");

        std::ifstream input_file(filename, std::ios::binary);
        if(!input_file.is_open())
            throw std::runtime_error("Failed to open input file: " + filename);

        input_file.seekg(0, std::ios::end);
        size_t file_size = input_file.tellg();
        input_file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(file_size);
        input_file.read(reinterpret_cast<char*>(buffer.data()), file_size);
        if(!input_file)
            throw std::runtime_error("Failed to read executable");

        BytecodeStream stream(buffer.data(), buffer.size());
        executable_image image;

        memcpy(&image.header, stream.read_n_bytes(sizeof(executable_header)), sizeof(executable_header));
        if(!executable_validate_header(image.header, file_size))
            throw std::runtime_error("Invalid executable header");

        stream.seek(image.header.code_offset);

        size_t code_end = image.header.code_offset + image.header.code_size;
        while(stream.tell() < code_end)
            image.code.push_back(executable_parse_instruction(stream));

        // TODO: data section (yet not even implemented)

//        if(image.header.data_size > 0) {
//            stream.seek(image.header.data_offset);
//
//            const uint8_t* data = stream.read_n_bytes(image.header.data_size);
//            image.data.assign(data, data + image.header.data_size);
//        }



        if(image.header.meta_size > 0) {
            stream.seek(image.header.meta_offset);
            
            executable_header_meta meta_header;
            meta_header.symbol_count     = stream.read_32();
            meta_header.type_count       = stream.read_32();
            meta_header.string_pool_size = stream.read_32();

            for(uint32_t i = 0; i < meta_header.type_count; i++) {
                executable_meta_type type;
                type.name_offset = stream.read_32();
                type.size        = stream.read_32();
                type.flags       = stream.read_32();

                image.meta.types.push_back(type);
            }

            for(uint32_t i = 0; i < meta_header.symbol_count; i++) {
                executable_meta_symbol symbol;
                symbol.name_offset  = stream.read_32();
                symbol.type_id      = stream.read_32();
                symbol.stack_offset = stream.read_32();
                symbol.flags        = stream.read_32();

                image.meta.symbols.push_back(symbol);
            }

            if(meta_header.string_pool_size > 0) {
                const uint8_t* strings = stream.read_n_bytes(meta_header.string_pool_size);
                image.meta.string_pool.assign(reinterpret_cast<const char*>(strings), meta_header.string_pool_size);
            }
        }

        return image;
    }
};
