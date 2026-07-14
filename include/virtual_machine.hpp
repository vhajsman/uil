#ifndef __VIRTUAL_MACHINE_H
#define __VIRTUAL_MACHINE_H

#include "executable.hpp"
#include "instruction.hpp"
#include "kit_params.hpp"
#include "registers.hpp"
#include <cstdint>
#include <vector>

namespace uil {
    class VirtualMachine;


    class VirtualMachine {
        protected:
        static constexpr uint32_t STACK_TOP = 0xFFFFFFFF;
        static constexpr uint32_t STACK_SIZE = 0x210000;
        bool halt = false;
        
        private:
        executable_image image;

        register_file regs;
        std::vector<uint8_t> memory;
        uint32_t memory_size;

        void push(uint64_t val);
        uint64_t pop();

        uint32_t stack_start = this->STACK_TOP;
        uint32_t stack_end = this->STACK_TOP - this->STACK_SIZE;

        uint64_t read_memory(uint32_t address);
        void write_memory(uint32_t address, uint64_t val);

        uint64_t cast_operand_read(instruction_operand& op);
        void cast_operand_write(instruction_operand& op, uint64_t val);
        // uint64_t* cast_operand(instruction_operand& op);
        // uint64_t read_operand_imm(instruction_operand& op);

        public:
        VirtualMachine();
        ~VirtualMachine();

        void boot(const std::string& filename);
        void run();

        protected:
        void run_instruction(instruction& ins);

        private:
        inline void register_set(register_t& reg, uint64_t val) {
            reg.value = val;
        }

        inline uint64_t register_read(register_t& reg) {
            return reg.value;
        }
    };

    void virtual_machine_main(kit_params* params);
};

#endif
