/*

This script is only for:
            -CPU memory access
            -Utility functions for opcodes such as flag modification or register access
            -Any kind of CPU functionality like interupts or the boot sequence

Opcodes should be handled in the opcodes.cpp script.
Opcode declarations are in CPU.h and the opcode lookup table is in the init tables function here.
Any other peice of hardware should have its own script and class.

*/

#include "CPU.h"
#include "Display.h"

uint8_t CPU::IF = 0;
uint8_t CPU::IE = 0;
bool CPU::IME = false;

CPU::CPU()
    :cycles(0), randGen(std::chrono::system_clock::now().time_since_epoch().count()),
    randByte(0, 255U)
{
    running = true;

    for (int i = 0; i < 8; i++) { // zero out all the registers 
        registers[i] = 0x00;
    }

    SetAF(0x0000);
    SetBC(0x0000);
    SetDE(0x0000);
    SetHL(0x0000);

    IE = 0x00; // Interrupt Enable Register (disable all interrupts by default)
    IF = 0x00; // Interrupt Flag Register (no interrupts pending by default)
    IME = false;

    //pc = 0x0100; // Start address for the Game Boy program counter when running a game

    InitTables();
}

bool CPU::LoadROM(const std::string& filename) {
    std::ifstream romFile(filename, std::ios::binary | std::ios::ate);

    if (!romFile.is_open()) {
        std::cerr << "Error: Could not open ROM file!" << std::endl;
        return false;
    }

    std::streamsize romSize = romFile.tellg();
    if (romSize > 0x10000) { // Check if ROM size exceeds available memory
        std::cerr << "Error: ROM size exceeds available memory!" << std::endl;
        return false;
    }

    romFile.seekg(0, std::ios::beg);

    romData.resize(static_cast<size_t>(romSize));
    if (!romFile.read(reinterpret_cast<char*>(romData.data()), romSize)) {
        std::cerr << "Error: Could not read ROM data!" << std::endl;
        return false;
    }

    numBanks = (romSize - 1) / 0x4000 + 1;

    // Copy the ROM data into memory starting at ROM_START_ADDR
    memcpy(&memory[ROM_START_ADDR], romData.data(), romSize);

    // Load the first switchable bank (0x4000 - 0x7FFF)
    //switch_bank(1);

    pc = 0x0100;

    romFile.close();

    std::cout << "ROM loaded at address " << std::hex << ROM_START_ADDR << std::endl;
    return true;
}

void CPU::LoadBIOS(const char* path) {
    // Open the BIOS file
    std::ifstream biosFile(path, std::ios::binary);

    if (!biosFile) {
        std::cerr << "Error: Unable to open BIOS file at " << path << std::endl;
        return;
    }

    // Read the BIOS file into memory starting at address 0x0000
    biosFile.read(reinterpret_cast<char*>(&memory[0x0000]), 0x100);

    if (biosFile.gcount() != 0x100) {
        std::cerr << "Error: BIOS file size is not 256 bytes (0x100 bytes)" << std::endl;
        return;
    }

    // Close the file
    biosFile.close();

    std::cout << "BIOS loaded successfully" << std::endl;
}

void CPU::switch_bank(int bank) {
    if (bank >= numBanks) {
        std::cerr << "Error: Invalid bank switch attempt!" << std::endl;
        return;
    }

    // Calculate the memory offset for the given bank
    size_t offset = bank * 0x4000;

    // Copy the bank's data into the 0x4000-0x7FFF range
    memcpy(&memory[0x4000], &romData[offset], 0x4000);
}

void CPU::InitTables()
{
    // Initialize all entries to OP_NULL to handle unimplemented opcodes
    for (int i = 0; i < 256; i++) {
        mainTable[i] = &CPU::OP_NULL;
        cbTable[i] = &CPU::OP_NULL;
    }

    // Initialize addressing mode table
    addressModeTable[0] = &CPU::AddrMode_Immediate;
    addressModeTable[1] = &CPU::AddrMode_Direct;
    addressModeTable[2] = &CPU::AddrMode_Immediate16;

    addressModeTableWithParam[2] = &CPU::AddrMode_Register;
    addressModeTableWithParam[3] = &CPU::AddrMode_Indirect;

    // Set up function pointer table for main opcodes
    mainTable[0x00] = &CPU::OP_00; // NOP
    mainTable[0x01] = &CPU::OP_01; // LD BC,nn
    mainTable[0x02] = &CPU::OP_02; // LD (BC),A
    mainTable[0x03] = &CPU::OP_03; // INC BC
    mainTable[0x04] = &CPU::OP_04; // INC B
    mainTable[0x05] = &CPU::OP_05; // DEC B
    mainTable[0x06] = &CPU::OP_06; // LD B,n
    mainTable[0x07] = &CPU::OP_07; // RLCA
    mainTable[0x08] = &CPU::OP_08; // LD (nn),SP
    mainTable[0x09] = &CPU::OP_09; // ADD HL,BC
    mainTable[0x0A] = &CPU::OP_0A; // LD A,(BC)
    mainTable[0x0B] = &CPU::OP_0B; // DEC BC
    mainTable[0x0C] = &CPU::OP_0C; // INC C
    mainTable[0x0D] = &CPU::OP_0D; // DEC C
    mainTable[0x0E] = &CPU::OP_0E; // LD C,n
    mainTable[0x0F] = &CPU::OP_0F; // RRCA
    mainTable[0x10] = &CPU::OP_10; // STOP 0
    mainTable[0x11] = &CPU::OP_11; // LD DE, d16
    mainTable[0x12] = &CPU::OP_12; // LD (DE), A
    mainTable[0x13] = &CPU::OP_13; // INC DE
    mainTable[0x14] = &CPU::OP_14; // INC D
    mainTable[0x15] = &CPU::OP_15; // DEC D
    mainTable[0x16] = &CPU::OP_16; // LD D, d8
    mainTable[0x17] = &CPU::OP_17; // RLA
    mainTable[0x18] = &CPU::OP_18; // JR r8 
    mainTable[0x19] = &CPU::OP_19; // ADD HL, DE
    mainTable[0x1A] = &CPU::OP_1A; // LD A, (DE)
    mainTable[0x1B] = &CPU::OP_1B; // DEC DE
    mainTable[0x1C] = &CPU::OP_1C; // INC E
    mainTable[0x1D] = &CPU::OP_1D; // DEC E
    mainTable[0x1E] = &CPU::OP_1E; // LD E, d8
    mainTable[0x1F] = &CPU::OP_1F; // RRA
    mainTable[0x20] = &CPU::OP_20; // JR NZ, r8
    mainTable[0x21] = &CPU::OP_21; // LD HL, d16
    mainTable[0x22] = &CPU::OP_22; // LD (HL+), A
    mainTable[0x23] = &CPU::OP_23; // INC HL
    mainTable[0x24] = &CPU::OP_24; // INC H 
    mainTable[0x25] = &CPU::OP_25; // DEC H
    mainTable[0x26] = &CPU::OP_26; // LD H, d8
    mainTable[0x27] = &CPU::OP_27; // DAA
    mainTable[0x28] = &CPU::OP_28; // JR Z, r8
    mainTable[0x29] = &CPU::OP_29; // ADD HL, HL
    mainTable[0x2A] = &CPU::OP_2A; // LD A, (HL+)
    mainTable[0x2B] = &CPU::OP_2B; // DEC HL
    mainTable[0x2C] = &CPU::OP_2C; // INC L
    mainTable[0x2D] = &CPU::OP_2D; // DEC L
    mainTable[0x2E] = &CPU::OP_2E; // LD L, d8
    mainTable[0x2F] = &CPU::OP_2F; // CPL
    mainTable[0x30] = &CPU::OP_30; // JR NC, r8
    mainTable[0x31] = &CPU::OP_31; // LD SP, d16
    mainTable[0x32] = &CPU::OP_32; // not even going to bother putting comments here anymore 
    mainTable[0x33] = &CPU::OP_33;
    mainTable[0x34] = &CPU::OP_34;
    mainTable[0x35] = &CPU::OP_35;
    mainTable[0x36] = &CPU::OP_36;
    mainTable[0x37] = &CPU::OP_37;
    mainTable[0x38] = &CPU::OP_38;
    mainTable[0x39] = &CPU::OP_39;
    mainTable[0x3A] = &CPU::OP_3A;
    mainTable[0x3B] = &CPU::OP_3B;
    mainTable[0x3C] = &CPU::OP_3C;
    mainTable[0x3D] = &CPU::OP_3D;
    mainTable[0x3E] = &CPU::OP_3E;
    mainTable[0x3F] = &CPU::OP_3F;
    mainTable[0x40] = &CPU::OP_40; // LD B, B <- first auto implement
    mainTable[0x41] = &CPU::OP_41; // LD B, C
    mainTable[0x42] = &CPU::OP_42; // LD B, D
    mainTable[0x43] = &CPU::OP_43; // LD B, E
    mainTable[0x44] = &CPU::OP_44; // LD B, H
    mainTable[0x45] = &CPU::OP_45; // LD B, L
    mainTable[0x46] = &CPU::OP_46; // LD B, (HL)
    mainTable[0x47] = &CPU::OP_47; // LD B, A
    mainTable[0x48] = &CPU::OP_48; // LD C, B
    mainTable[0x49] = &CPU::OP_49; // LD C, C
    mainTable[0x4A] = &CPU::OP_4A; // LD C, D
    mainTable[0x4B] = &CPU::OP_4B; // LD C, E
    mainTable[0x4C] = &CPU::OP_4C; // LD C, H
    mainTable[0x4D] = &CPU::OP_4D; // LD C, L
    mainTable[0x4E] = &CPU::OP_4E; // LD C, (HL)
    mainTable[0x4F] = &CPU::OP_4F; // LD C, A
    mainTable[0x50] = &CPU::OP_50; // LD D, B
    mainTable[0x51] = &CPU::OP_51; // LD D, C
    mainTable[0x52] = &CPU::OP_52; // LD D, D
    mainTable[0x53] = &CPU::OP_53; // LD D, E
    mainTable[0x54] = &CPU::OP_54; // LD D, H
    mainTable[0x55] = &CPU::OP_55; // LD D, L
    mainTable[0x56] = &CPU::OP_56; // LD D, (HL)
    mainTable[0x57] = &CPU::OP_57; // LD D, A
    mainTable[0x58] = &CPU::OP_58; // LD E, B
    mainTable[0x59] = &CPU::OP_59; // LD E, C
    mainTable[0x5A] = &CPU::OP_5A; // LD E, D
    mainTable[0x5B] = &CPU::OP_5B; // LD E, E
    mainTable[0x5C] = &CPU::OP_5C; // LD E, H
    mainTable[0x5D] = &CPU::OP_5D; // LD E, L
    mainTable[0x5E] = &CPU::OP_5E; // LD E, (HL)
    mainTable[0x5F] = &CPU::OP_5F; // LD E, A
    mainTable[0x60] = &CPU::OP_60; // LD H, B
    mainTable[0x61] = &CPU::OP_61; // LD H, C
    mainTable[0x62] = &CPU::OP_62; // LD H, D
    mainTable[0x63] = &CPU::OP_63; // LD H, E
    mainTable[0x64] = &CPU::OP_64; // LD H, H
    mainTable[0x65] = &CPU::OP_65; // LD H, L
    mainTable[0x66] = &CPU::OP_66; // LD H, (HL)
    mainTable[0x67] = &CPU::OP_67; // LD H, A
    mainTable[0x68] = &CPU::OP_68; // LD L, B
    mainTable[0x69] = &CPU::OP_69; // LD L, C
    mainTable[0x6A] = &CPU::OP_6A; // LD L, D
    mainTable[0x6B] = &CPU::OP_6B; // LD L, E
    mainTable[0x6C] = &CPU::OP_6C; // LD L, H
    mainTable[0x6D] = &CPU::OP_6D; // LD L, L
    mainTable[0x6E] = &CPU::OP_6E; // LD L, (HL)
    mainTable[0x6F] = &CPU::OP_6F; // LD L, A
    mainTable[0x70] = &CPU::OP_70; // LD (HL), B
    mainTable[0x71] = &CPU::OP_71; // LD (HL), C
    mainTable[0x72] = &CPU::OP_72; // LD (HL), D
    mainTable[0x73] = &CPU::OP_73; // LD (HL), E
    mainTable[0x74] = &CPU::OP_74; // LD (HL), H
    mainTable[0x75] = &CPU::OP_75; // LD (HL), L
    mainTable[0x76] = &CPU::OP_76; // HALT
    mainTable[0x77] = &CPU::OP_77; // LD (HL), A
    mainTable[0x78] = &CPU::OP_78; // LD A, B
    mainTable[0x79] = &CPU::OP_79; // LD A, C
    mainTable[0x7A] = &CPU::OP_7A; // LD A, D
    mainTable[0x7B] = &CPU::OP_7B; // LD A, E
    mainTable[0x7C] = &CPU::OP_7C; // LD A, H
    mainTable[0x7D] = &CPU::OP_7D; // LD A, L
    mainTable[0x7E] = &CPU::OP_7E; // LD A, (HL)
    mainTable[0x7F] = &CPU::OP_7F; // LD A, A
    mainTable[0x80] = &CPU::OP_80; // ADD A, B
    mainTable[0x81] = &CPU::OP_81; // ADD A, C
    mainTable[0x82] = &CPU::OP_82; // ADD A, D
    mainTable[0x83] = &CPU::OP_83; // ADD A, E
    mainTable[0x84] = &CPU::OP_84; // ADD A, H
    mainTable[0x85] = &CPU::OP_85; // ADD A, L
    mainTable[0x86] = &CPU::OP_86; // ADD A, (HL)
    mainTable[0x87] = &CPU::OP_87; // ADD A, A
    mainTable[0x88] = &CPU::OP_88; // ADC A, B
    mainTable[0x89] = &CPU::OP_89; // ADC A, C
    mainTable[0x8A] = &CPU::OP_8A; // ADC A, D
    mainTable[0x8B] = &CPU::OP_8B; // ADC A, E
    mainTable[0x8C] = &CPU::OP_8C; // ADC A, H
    mainTable[0x8D] = &CPU::OP_8D; // ADC A, L
    mainTable[0x8E] = &CPU::OP_8E; // ADC A, (HL)
    mainTable[0x8F] = &CPU::OP_8F; // ADC A, A
    mainTable[0x90] = &CPU::OP_90; // SUB B
    mainTable[0x91] = &CPU::OP_91; // SUB C
    mainTable[0x92] = &CPU::OP_92; // SUB D
    mainTable[0x93] = &CPU::OP_93; // SUB E
    mainTable[0x94] = &CPU::OP_94; // SUB H
    mainTable[0x95] = &CPU::OP_95; // SUB L
    mainTable[0x96] = &CPU::OP_96; // SUB (HL)
    mainTable[0x97] = &CPU::OP_97; // SUB A
    mainTable[0x98] = &CPU::OP_98; // SBC A, B
    mainTable[0x99] = &CPU::OP_99; // SBC A, C
    mainTable[0x9A] = &CPU::OP_9A; // SBC A, D
    mainTable[0x9B] = &CPU::OP_9B; // SBC A, E
    mainTable[0x9C] = &CPU::OP_9C; // SBC A, H
    mainTable[0x9D] = &CPU::OP_9D; // SBC A, L
    mainTable[0x9E] = &CPU::OP_9E; // SBC A, (HL)
    mainTable[0x9F] = &CPU::OP_9F; // SBC A, A
    mainTable[0xA0] = &CPU::OP_A0; // AND B
    mainTable[0xA1] = &CPU::OP_A1; // AND C
    mainTable[0xA2] = &CPU::OP_A2; // AND D
    mainTable[0xA3] = &CPU::OP_A3; // AND E
    mainTable[0xA4] = &CPU::OP_A4; // AND H
    mainTable[0xA5] = &CPU::OP_A5; // AND L
    mainTable[0xA6] = &CPU::OP_A6; // AND (HL)
    mainTable[0xA7] = &CPU::OP_A7; // AND A
    mainTable[0xA8] = &CPU::OP_A8; // XOR B
    mainTable[0xA9] = &CPU::OP_A9; // XOR C
    mainTable[0xAA] = &CPU::OP_AA; // XOR D
    mainTable[0xAB] = &CPU::OP_AB; // XOR E
    mainTable[0xAC] = &CPU::OP_AC; // XOR H
    mainTable[0xAD] = &CPU::OP_AD; // XOR L
    mainTable[0xAE] = &CPU::OP_AE; // XOR (HL)
    mainTable[0xAF] = &CPU::OP_AF; // XOR A
    mainTable[0xB0] = &CPU::OP_B0; // OR B
    mainTable[0xB1] = &CPU::OP_B1; // OR C
    mainTable[0xB2] = &CPU::OP_B2; // OR D
    mainTable[0xB3] = &CPU::OP_B3; // OR E
    mainTable[0xB4] = &CPU::OP_B4; // OR H
    mainTable[0xB5] = &CPU::OP_B5; // OR L
    mainTable[0xB6] = &CPU::OP_B6; // OR (HL)
    mainTable[0xB7] = &CPU::OP_B7; // OR A
    mainTable[0xB8] = &CPU::OP_B8; // CP B
    mainTable[0xB9] = &CPU::OP_B9; // CP C
    mainTable[0xBA] = &CPU::OP_BA; // CP D
    mainTable[0xBB] = &CPU::OP_BB; // CP E
    mainTable[0xBC] = &CPU::OP_BC; // CP H
    mainTable[0xBD] = &CPU::OP_BD; // CP L
    mainTable[0xBE] = &CPU::OP_BE; // CP (HL)
    mainTable[0xBF] = &CPU::OP_BF; // CP A
    mainTable[0xC0] = &CPU::OP_C0; // RET NZ
    mainTable[0xC1] = &CPU::OP_C1; // POP BC
    mainTable[0xC2] = &CPU::OP_C2; // JP NZ, a16
    mainTable[0xC3] = &CPU::OP_C3; // JP a16
    mainTable[0xC4] = &CPU::OP_C4; // CALL NZ, a16
    mainTable[0xC5] = &CPU::OP_C5; // PUSH BC
    mainTable[0xC6] = &CPU::OP_C6; // ADD A, d8
    mainTable[0xC7] = &CPU::OP_C7; // RST 00H
    mainTable[0xC8] = &CPU::OP_C8; // RET Z
    mainTable[0xC9] = &CPU::OP_C9; // RET
    mainTable[0xCA] = &CPU::OP_CA; // JP Z, a16
  //mainTable[0xCB] = &CPU::OP_CB; // PREFIX CB not needed
    mainTable[0xCC] = &CPU::OP_CC; // CALL Z, a16
    mainTable[0xCD] = &CPU::OP_CD; // CALL a16
    mainTable[0xCE] = &CPU::OP_CE; // ADC A, d8
    mainTable[0xCF] = &CPU::OP_CF; // RST 08H
    mainTable[0xD0] = &CPU::OP_D0; // RET NC
    mainTable[0xD1] = &CPU::OP_D1; // POP DE
    mainTable[0xD2] = &CPU::OP_D2; // JP NC, a16
    mainTable[0xD3] = &CPU::OP_D3; // OUT (C), A
    mainTable[0xD4] = &CPU::OP_D4; // CALL NC, a16
    mainTable[0xD5] = &CPU::OP_D5; // PUSH DE
    mainTable[0xD6] = &CPU::OP_D6; // SUB d8
    mainTable[0xD7] = &CPU::OP_D7; // RST 10H
    mainTable[0xD8] = &CPU::OP_D8; // RET C
    mainTable[0xD9] = &CPU::OP_D9; // RETI
    mainTable[0xDA] = &CPU::OP_DA; // JP C, a16
    mainTable[0xDB] = &CPU::OP_DB; // IN A, (C)
    mainTable[0xDC] = &CPU::OP_DC; // CALL C, a16
    mainTable[0xDD] = &CPU::OP_DD; // NOP IX/IY opcodes not needed
    mainTable[0xDE] = &CPU::OP_DE; // SBC A, d8
    mainTable[0xDF] = &CPU::OP_DF; // RST 18H
    mainTable[0xE0] = &CPU::OP_E0; // LD (a8), A
    mainTable[0xE1] = &CPU::OP_E1; // POP HL
    mainTable[0xE2] = &CPU::OP_E2; // LD (C), A
    mainTable[0xE3] = &CPU::OP_E3; // NOP
    mainTable[0xE4] = &CPU::OP_E4; // NOP
    mainTable[0xE5] = &CPU::OP_E5; // PUSH HL
    mainTable[0xE6] = &CPU::OP_E6; // AND d8
    mainTable[0xE7] = &CPU::OP_E7; // RST 20H
    mainTable[0xE8] = &CPU::OP_E8; // ADD SP, r8
    mainTable[0xE9] = &CPU::OP_E9; // JP (HL)
    mainTable[0xEA] = &CPU::OP_EA; // LD (a16), A
    mainTable[0xEB] = &CPU::OP_EB; // NOP
    mainTable[0xEC] = &CPU::OP_EC; // NOP
    mainTable[0xED] = &CPU::OP_ED; // NOP
    mainTable[0xEE] = &CPU::OP_EE; // XOR d8
    mainTable[0xEF] = &CPU::OP_EF; // RST 28H
    mainTable[0xF0] = &CPU::OP_F0; // LD A, (a16)
    mainTable[0xF1] = &CPU::OP_F1; // POP AF
    mainTable[0xF2] = &CPU::OP_F2; // LD A, (C)
    mainTable[0xF3] = &CPU::OP_F3; // DI
    mainTable[0xF4] = &CPU::OP_F4; // NOP
    mainTable[0xF5] = &CPU::OP_F5; // PUSH AF
    mainTable[0xF6] = &CPU::OP_F6; // OR d8
    mainTable[0xF7] = &CPU::OP_F7; // RST 30H
    mainTable[0xF8] = &CPU::OP_F8; // LD HL, SP+r8
    mainTable[0xF9] = &CPU::OP_F9; // LD SP, HL
    mainTable[0xFA] = &CPU::OP_FA; // LD A, (a16)
    mainTable[0xFB] = &CPU::OP_FB; // EI
    mainTable[0xFC] = &CPU::OP_FC; // NOP
    mainTable[0xFD] = &CPU::OP_FD; // NOP
    mainTable[0xFE] = &CPU::OP_FE; // CP d8
    mainTable[0xFF] = &CPU::OP_FF; // RST 38H

    // Set up function pointer table for CB-prefixed opcodes
    cbTable[0x00] = &CPU::CB_00; // RLC B
    cbTable[0x01] = &CPU::CB_01; // RLC C
    cbTable[0x02] = &CPU::CB_02; // RLC D
    cbTable[0x03] = &CPU::CB_03; // RLC E
    cbTable[0x04] = &CPU::CB_04; // RLC H
    cbTable[0x05] = &CPU::CB_05; // RLC L
    cbTable[0x06] = &CPU::CB_06; // RLC (HL)
    cbTable[0x07] = &CPU::CB_07; // RLC A
    cbTable[0x08] = &CPU::CB_08; // RRC B
    cbTable[0x09] = &CPU::CB_09; // RRC C
    cbTable[0x0A] = &CPU::CB_0A; // RRC D
    cbTable[0x0B] = &CPU::CB_0B; // RRC E
    cbTable[0x0C] = &CPU::CB_0C; // RRC H
    cbTable[0x0D] = &CPU::CB_0D; // RRC L
    cbTable[0x0E] = &CPU::CB_0E; // RRC (HL)
    cbTable[0x0F] = &CPU::CB_0F; // RRC A
}

void CPU::Cycle() {
    if (!stopped) {
        if (halted) {
            CheckInterrupts();
            if (IE & IF) {
                halted = false;
            }
            return;
        }

        // Fetch the opcode
        opcode = memory[pc];

        // Execute the opcode
        if (opcode == 0xCB) {
            opcode = memory[++pc];
            (this->*cbTable[opcode])();
            pc += 1;  // Move past the CB prefix
        }
        else {
            (this->*mainTable[opcode])();
            register_out();
            pc += 1;  // Move past the opcode
        }

        // Update cycles
        cycles += opcodeCycles[opcode];  // Add the cycle count for the executed opcode
    }
    else {
        // Check interrupts if the CPU is stopped
        CheckInterrupts();
    }
}

void CPU::Update()
{
    static uint32_t cycleAccumulator = 0;

    // Update the CPU cycle count and accumulate cycles
    cycleAccumulator += cycles;

    while (cycleAccumulator >= CYCLES_PER_FRAME) {
        cycleAccumulator -= CYCLES_PER_FRAME;
    }

}

void CPU::check_test() {
    std::cout << "checking test result" << std::endl;

    const uint16_t RESULT_ADDR = 0xC000;
    const uint8_t SUCCESS_CODE = 0x00;

    std::cout << "Memory at 0xC000: " << std::hex << +memory[RESULT_ADDR] << std::endl;

    uint8_t result = memory[RESULT_ADDR];

    if (result == SUCCESS_CODE) {
        std::cout << "Test passed!" << std::endl;
    }
    else {
        std::cout << "Test failed. Code: " << std::hex << +result << std::endl;
    }

    static uint16_t lastPC = 0;
    if (pc == lastPC) {
        std::cout << "Test ROM appears to be in an infinite loop. Test likely completed." << std::endl;
        running = false;
    }
    lastPC = pc;
}

#pragma region Utility_functions_for_accsessing_flags
void CPU::SetFlag(uint8_t flag, bool value)
{
    if (value) {
        registers[F] |= flag; // Set the flag
    }
    else {
        registers[F] &= ~flag; // Clear the flag
    }

    UpdateFlags();
}

bool CPU::GetFlag(uint8_t flag) const
{
    return (flags & flag) != 0;
}

void CPU::UpdateFlags()
{
    flags = registers[F];
}

void CPU::UpdateFlagsAfterArithmetic(uint32_t result, uint16_t operand1, uint16_t operand2, bool isSubtraction) {
    SetFlag(FLAG_N, isSubtraction);

    // H flag: Half Carry flag
    bool halfCarry = isSubtraction ? ((operand1 & 0xFFF) < (operand2 & 0xFFF))
        : ((operand1 & 0xFFF) + (operand2 & 0xFFF) > 0xFFF);
    SetFlag(FLAG_H, halfCarry);

    // C flag: Carry flag
    bool carry = isSubtraction ? (operand1 < operand2) : (result > 0xFFFF);
    SetFlag(FLAG_C, carry);
}

void CPU::UpdateFlagsAfterIncrement(int reg_index, int result)
{

    SetFlag(FLAG_Z, result == 0); // Zero flag
    SetFlag(FLAG_N, false);       // Subtract flag (clear for increment)
    SetFlag(FLAG_H, ((registers[reg_index] & 0x0F) == 0x00)); // Half-Carry flag (set if lower nibble was 0x0F before increment)
} // 16 bit registers don't need this 

void CPU::UpdateFlagsAfterDecrement(int reg_index, int result)
{
    uint8_t original = registers[reg_index]; // The original value of the register

    // Update flags
    SetFlag(FLAG_Z, result == 0); // Zero flag
    SetFlag(FLAG_N, true);        // Subtract flag (set for decrement)

    // Half-Carry flag (set if there was a borrow from the lower nibble)
    SetFlag(FLAG_H, ((original & 0x0F) == 0x00));
}

void CPU::UFAI16(uint16_t reg, int result)
{
    SetFlag(FLAG_Z, result == 0); // Zero flag
    SetFlag(FLAG_N, false);       // Subtract flag (clear for increment)
    SetFlag(FLAG_H, ((reg & 0x0F) == 0x00));
}

void CPU::UFAD16(uint16_t reg, int result)
{
    uint8_t original = reg; // The original value of the register

    // Update flags
    SetFlag(FLAG_Z, result == 0); // Zero flag
    SetFlag(FLAG_N, true);        // Subtract flag (set for decrement)

    // Half-Carry flag (set if there was a borrow from the lower nibble)
    SetFlag(FLAG_H, ((original & 0x0F) == 0x00));
}

void CPU::UFARA(uint16_t result, uint8_t op1, uint8_t op2, bool is_subtraction)
{
    SetFlag(FLAG_Z, (result & 0xFF) == 0);

    SetFlag(FLAG_N, is_subtraction);

    bool halfCarry;
    if (is_subtraction) {
        halfCarry = ((op1 & 0x0F) < (op2 & 0x0F));
    }
    else {
        halfCarry = (((op1 & 0x0F) + (op2 & 0x0F)) > 0x0F);
    }
    SetFlag(FLAG_H, halfCarry);

    bool carry;
    if (is_subtraction) {
        carry = (op1 < op2);
    }
    else {
        carry = (result > 0xFF);
    }
    SetFlag(FLAG_C, carry);
}

void CPU::UFAAO(uint8_t result)
{
    SetFlag(FLAG_Z, result == 0x00);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, true);
    SetFlag(FLAG_C, false);
}

void CPU::UFAOXO(uint8_t result)
{
    SetFlag(FLAG_Z, result == 0x00);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_C, false);
}
#pragma endregion

#pragma region utility_functions_for_accessing_16_bit_registers 
// Access methods for 16-bit registers
uint16_t CPU::GetAF() const
{
    return (registers[0] << 8) | (flags);
}

void CPU::SetAF(uint16_t value)
{
    registers[0] = value >> 8;
    flags = value & 0xF0;  // Lower nibble of F is always 0
}

uint16_t CPU::GetBC() const
{
    return (registers[B] << 8) | registers[C];
}

void CPU::SetBC(uint16_t value)
{
    registers[B] = value >> 8;
    registers[C] = value & 0xFF;
}

uint16_t CPU::GetDE() const
{
    return (registers[3] << 8) | registers[4];
}

void CPU::SetDE(uint16_t value)
{
    registers[3] = value >> 8;
    registers[4] = value & 0xFF;
}

uint16_t CPU::GetHL() const
{
    return (registers[H] << 8) | registers[L];
}

void CPU::SetHL(uint16_t value)
{
    registers[H] = value >> 8;
    registers[L] = value & 0xFF;
}
#pragma endregion

#pragma region functions_for_easily_accsessing_stack
void CPU::PushToStack(uint16_t val)
{
    memory[sp - 1] = (val >> 8) & 0xFF; // Push high byte
    memory[sp - 2] = val & 0xFF;        // Push low byte
    sp -= 2; // Update stack pointer
}

uint16_t CPU::PopFromStack() {
    uint16_t val = (memory[sp] << 8) | memory[sp + 1]; // Pop value
    sp += 2; // Update stack pointer
    return val;
}
#pragma endregion

#pragma region CPU_interupt_functions
void CPU::CheckInterrupts() {
    if (IME && (IF & IE)) {
        if (stopped) {
            stopped = false;
        }
        HandleInterrupt(); // Handle the interrupt
    }
}


void CPU::HandleInterrupt() {
    // Check if interrupts are enabled
    if (!IME) {
        return; // Do not handle interrupts if IME is not set
    }

    // Check each interrupt
    for (int i = 0; i < 5; ++i) {
        // Check if the interrupt is enabled and the flag is set
        if ((IF & (1 << i)) && (IE & (1 << i))) {
            // Execute the interrupt routine
            ExecuteInterruptRoutine(i);
            // Clear the interrupt flag
            IF &= ~(1 << i);
            // Exit after handling the first pending interrupt
            break;
        }
    }
}

void CPU::ExecuteInterruptRoutine(int interruptIndex)
{
    PushToStack(pc);                   // Save current PC
    pc = INTERRUPT_VECTORS[interruptIndex]; // Jump to interrupt vector address

    // Call the appropriate interrupt handler
    switch (interruptIndex) {
    case 0: VBlankInterrupt(); break;
    case 1: LCDSTATInterrupt(); break;
    case 2: TimerInterrupt(); break;
    case 3: SerialInterrupt(); break;
    case 4: JoypadInterrupt(); break;
    default: break; // Safety check, although should never occur
    }

    // Pop the return address from the stack
    pc = PopFromStack();
}

void CPU::RequestInterrupt(int index)
{
    IF |= (1 << index);
}
#pragma endregion

#pragma region interrupt_functionality
void CPU::VBlankInterrupt()
{
}

void CPU::LCDSTATInterrupt()
{
}

void CPU::TimerInterrupt()
{
}

void CPU::SerialInterrupt()
{
}

void CPU::JoypadInterrupt()
{
}
#pragma endregion 

#pragma region addressing_modes
uint16_t CPU::AddrMode_Immediate()
{
    return memory[pc + 1];
}

uint16_t CPU::AddrMode_Direct()
{
    uint8_t low = memory[pc + 1];   // Fetch the lower byte of the address
    uint8_t high = memory[pc + 2];  // Fetch the upper byte of the address
    return (high << 8) | low;     // Combine to form the full 16-bit address
}

uint16_t CPU::AddrMode_Immediate16()
{
    uint8_t lowerByte = memory[pc + 1];
    uint8_t upperByte = memory[pc + 2];
    return (upperByte << 8) | lowerByte;
}

uint16_t CPU::AddrMode_Register(uint8_t reg)
{
    return registers[reg];
}

uint16_t CPU::AddrMode_Indirect(uint8_t addr)
{
    return registers[addr]; // addr should be reg I just don't want to change it
}
#pragma endregion

void CPU::register_out()
{
    std::cout << "----------------------------" << std::endl;

    std::cout << "Current opcode: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode)
        << " at " << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(pc) << std::endl;

    std::cout << "A: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[A]) << std::endl;
    std::cout << "B: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[B]) << std::endl;
    std::cout << "C: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[C]) << std::endl;
    std::cout << "D: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[D]) << std::endl;
    std::cout << "E: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[E]) << std::endl;
    std::cout << "F: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[F]) << std::endl;
    std::cout << "H: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[H]) << std::endl;
    std::cout << "L: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(registers[L]) << std::endl;

    std::cout << "AF: " << std::hex << std::setw(4) << std::setfill('0') << GetAF() << std::endl;
    std::cout << "BC: " << std::hex << std::setw(4) << std::setfill('0') << GetBC() << std::endl;
    std::cout << "DE: " << std::hex << std::setw(4) << std::setfill('0') << GetDE() << std::endl;
    std::cout << "HL: " << std::hex << std::setw(4) << std::setfill('0') << GetHL() << std::endl;
}
