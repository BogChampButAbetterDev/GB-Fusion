#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#include <SDL.h>

#define BIOS_START_ADDR 0x0000
#define ROM_START_ADDR 0x0100

/*

REGISTER INDEXES:

    0 = a
    1 = b
    2 = c
    3 = d
    4 = e
    5 = f
    6 = h
    7 = l

*/

#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define F 5
#define H 6
#define L 7

class CPU
{
public:
    bool running = true;

    uint8_t registers[8]{};
    uint16_t pc{};
    uint16_t sp{};
    uint8_t* memory = new uint8_t[65536]{};
    uint16_t opcode;
    uint8_t addressingMode;
    uint8_t flags;

    uint32_t cycles;

    uint8_t* io_port = new uint8_t[0x100];

    std::vector<uint8_t> romData;

    int numBanks;
    int current_bank = 1;

    const uint32_t CYCLES_PER_FRAME = 4194304 / 60;

    CPU();

    ~CPU() { delete[] memory; delete[] io_port; }

public:
    bool LoadROM(const std::string& filename);
    void LoadBIOS(const char* path);
    void switch_bank(int bank);
    void Cycle();
    void Update();
    void check_test();

public:
    static uint8_t IE; // Interrupt Enable Register
    static uint8_t IF; // Interrupt Flag Register
    static bool IME;

    const uint16_t INTERRUPT_VECTORS[5] = { 0x40, 0x48, 0x50, 0x58, 0x60 };

    static const int VBLANK = 0x40;
    static const int LCDSTAT = 0x48;
    static const int TIMER = 0x50;
    static const int SERIAL = 0x58;
    static const int JOYPAD = 0x60;

private:
    typedef void (CPU::* CPUFunc)();
    typedef uint16_t(CPU::* CPUAddrModeFuncWithParam)(uint8_t);
    typedef uint16_t(CPU::* CPUAddrModeFunc)(); // No parameters

    bool stopped = false;
    bool halted = false;

    CPUFunc* mainTable = new CPUFunc[256];
    CPUFunc* cbTable = new CPUFunc[256];
    CPUAddrModeFuncWithParam addressModeTableWithParam[4];
    CPUAddrModeFunc addressModeTable[4];

    const int opcodeCycles[256] = {
    4, 8, 8, 8, 4, 4, 8, 4, 20, 8, 8, 8, 4, 4, 8, 4,
    12, 12, 8, 12, 8, 8, 12, 8, 12, 8, 8, 8, 8, 4, 8, 4,
    12, 12, 12, 12, 8, 4, 12, 4, 12, 8, 12, 12, 8, 4, 12, 4,
    12, 12, 12, 12, 12, 4, 12, 4, 12, 8, 12, 12, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    8, 12, 12, 12, 12, 12, 8, 12, 8, 12, 12, 8, 12, 12, 8, 12,
    8, 12, 12, 12, 12, 12, 8, 12, 8, 12, 12, 12, 12, 12, 8, 12,
    12, 12, 12, 12, 12, 12, 8, 12, 12, 12, 12, 12, 12, 12, 8, 12,
    12, 12, 12, 12, 12, 12, 8, 12, 12, 12, 12, 12, 12, 12, 8, 12,
    };

    std::default_random_engine randGen;
    std::uniform_int_distribution<unsigned int> randByte;

    void InitTables();

private:
    // Main opcodes
    void OP_00(); // NOP
    void OP_01(); // LD BC,nn
    void OP_02(); // LD (BC),A
    void OP_03(); // INC BC
    void OP_04(); // INC B
    void OP_05(); // DEC B
    void OP_06(); // LD B,n
    void OP_07(); // RLCA
    void OP_08(); // LD (nn),SP
    void OP_09(); // ADD HL,BC
    void OP_0A(); // LD A,(BC)
    void OP_0B(); // DEC BC
    void OP_0C(); // INC C
    void OP_0D(); // DEC C
    void OP_0E(); // LD C,n
    void OP_0F(); // RRCA
    void OP_10(); // STOP 0
    void OP_11(); // LD DE, d16
    void OP_12(); // LD (DE), A
    void OP_13(); // INC DE
    void OP_14(); // INC D
    void OP_15(); // DEC D
    void OP_16(); // LD D, d8
    void OP_17(); // RLA
    void OP_18(); // JR r8 
    void OP_19(); // ADD HL, DE
    void OP_1A(); // LD A, (DE)
    void OP_1B(); // DEC DE
    void OP_1C(); // INC E
    void OP_1D(); // DEC E
    void OP_1E(); // LD E, d8 
    void OP_1F(); // RRA
    void OP_20(); // JR NZ, r8
    void OP_21(); // LD HL, d16
    void OP_22(); // LD (HL+), A
    void OP_23(); // INC HL
    void OP_24(); // INC H
    void OP_25(); // DEC H
    void OP_26(); // LD H, d8
    void OP_27(); // DAA
    void OP_28(); // JR Z, r8
    void OP_29(); // ADD HL, HL
    void OP_2A(); // LD A, (HL+)
    void OP_2B(); // DEC HL
    void OP_2C(); // INC L
    void OP_2D(); // DEC L
    void OP_2E(); // LD L, d8
    void OP_2F(); // CPL
    void OP_30(); // JR NC, r8
    void OP_31(); // LD SP, d16
    void OP_32(); // LD (HL-), A
    void OP_33(); // INC SP
    void OP_34(); // INC (HL)
    void OP_35(); // DEC (DEC)
    void OP_36(); // LD HL, d8
    void OP_37(); // SCF
    void OP_38(); // JR C, r8
    void OP_39(); // ADD HL, SP
    void OP_3A(); // LD A, (HL-)
    void OP_3B(); // DEC SP
    void OP_3C(); // INC A 
    void OP_3D(); // DEC A
    void OP_3E(); // LD A, d8
    void OP_3F(); // CCF
    void OP_40(); // LD B, B
    void OP_41(); // LD B, C
    void OP_42(); // LD B, D
    void OP_43(); // LD B, E
    void OP_44(); // LD B, H
    void OP_45(); // LD B, L
    void OP_46(); // LD B, (HL)
    void OP_47(); // LD B, A
    void OP_48(); // LD C, B
    void OP_49(); // LD C, C
    void OP_4A(); // LD C, D
    void OP_4B(); // LD C, E
    void OP_4C(); // LD C, H
    void OP_4D(); // LD C, L
    void OP_4E(); // LD C, (HL)
    void OP_4F(); // LD C, A
    void OP_50(); // LD D, B
    void OP_51(); // LD D, C
    void OP_52(); // LD D, D
    void OP_53(); // LD D, E
    void OP_54(); // LD D, H
    void OP_55(); // LD D, L
    void OP_56(); // LD D, (HL)
    void OP_57(); // LD D, A
    void OP_58(); // LD E, B
    void OP_59(); // LD E, C
    void OP_5A(); // LD E, D
    void OP_5B(); // LD E, E
    void OP_5C(); // LD E, H
    void OP_5D(); // LD E, L
    void OP_5E(); // LD E, (HL)
    void OP_5F(); // LD E, A
    void OP_60(); // LD H, B
    void OP_61(); // LD H, C
    void OP_62(); // LD H, D
    void OP_63(); // LD H, E
    void OP_64(); // LD H, H
    void OP_65(); // LD H, L
    void OP_66(); // LD H, (HL)
    void OP_67(); // LD H, A
    void OP_68(); // LD L, B
    void OP_69(); // LD L, C
    void OP_6A(); // LD L, D
    void OP_6B(); // LD L, E
    void OP_6C(); // LD L, H
    void OP_6D(); // LD L, L
    void OP_6E(); // LD L, (HL)
    void OP_6F(); // LD L, A
    void OP_70(); // LD (HL), B
    void OP_71(); // LD (HL), C
    void OP_72(); // LD (HL), D
    void OP_73(); // LD (HL), E
    void OP_74(); // LD (HL), H
    void OP_75(); // LD (HL), L
    void OP_76(); // HALT
    void OP_77(); // LD (HL), A
    void OP_78(); // LD A, B
    void OP_79(); // LD A, C
    void OP_7A(); // LD A, D
    void OP_7B(); // LD A, E
    void OP_7C(); // LD A, H
    void OP_7D(); // LD A, L
    void OP_7E(); // LD A, (HL)
    void OP_7F(); // LD A, A
    void OP_80(); // ADD A, B
    void OP_81(); // ADD A, C
    void OP_82(); // ADD A, D
    void OP_83(); // ADD A, E
    void OP_84(); // ADD A, H
    void OP_85(); // ADD A, L
    void OP_86(); // ADD A, (HL)
    void OP_87(); // ADD A, A
    void OP_88(); // ADC A, B
    void OP_89(); // ADC A, C
    void OP_8A(); // ADC A, D
    void OP_8B(); // ADC A, E
    void OP_8C(); // ADC A, H
    void OP_8D(); // ADC A, L
    void OP_8E(); // ADC A, (HL)
    void OP_8F(); // ADC A, A
    void OP_90(); // SUB B
    void OP_91(); // SUB C
    void OP_92(); // SUB D
    void OP_93(); // SUB E
    void OP_94(); // SUB H
    void OP_95(); // SUB L
    void OP_96(); // SUB (HL)
    void OP_97(); // SUB A
    void OP_98(); // SBC A, B
    void OP_99(); // SBC A, C
    void OP_9A(); // SBC A, D
    void OP_9B(); // SBC A, E
    void OP_9C(); // SBC A, H
    void OP_9D(); // SBC A, L
    void OP_9E(); // SBC A, (HL)
    void OP_9F(); // SBC A, A
    void OP_A0(); // AND B
    void OP_A1(); // AND C
    void OP_A2(); // AND D
    void OP_A3(); // AND E
    void OP_A4(); // AND H
    void OP_A5(); // AND L
    void OP_A6(); // AND (HL)
    void OP_A7(); // AND A
    void OP_A8(); // XOR B
    void OP_A9(); // XOR C
    void OP_AA(); // XOR D
    void OP_AB(); // XOR E
    void OP_AC(); // XOR H
    void OP_AD(); // XOR L
    void OP_AE(); // XOR (HL)
    void OP_AF(); // XOR A
    void OP_B0(); // OR B
    void OP_B1(); // OR C
    void OP_B2(); // OR D
    void OP_B3(); // OR E
    void OP_B4(); // OR H
    void OP_B5(); // OR L
    void OP_B6(); // OR (HL)
    void OP_B7(); // OR A
    void OP_B8(); // CP B
    void OP_B9(); // CP C
    void OP_BA(); // CP D
    void OP_BB(); // CP E
    void OP_BC(); // CP H
    void OP_BD(); // CP L
    void OP_BE(); // CP (HL)
    void OP_BF(); // CP A
    void OP_C0(); // RET NZ
    void OP_C1(); // POP BC
    void OP_C2(); // JP NZ, a16
    void OP_C3(); // JP a16
    void OP_C4(); // CALL NZ, a16
    void OP_C5(); // PUSH BC
    void OP_C6(); // ADD A, d8
    void OP_C7(); // RST 00H
    void OP_C8(); // RET Z
    void OP_C9(); // RET
    void OP_CA(); // JP Z, a16
  //void OP_CB(); // PREFIX CB not needed
    void OP_CC(); // CALL Z, a16
    void OP_CD(); // CALL a16
    void OP_CE(); // ADC A, d8
    void OP_CF(); // RST 08H
    void OP_D0(); // RET NC
    void OP_D1(); // POP DE
    void OP_D2(); // JP NC, a16
    void OP_D3(); // OUT (C), A
    void OP_D4(); // CALL NC, a16
    void OP_D5(); // PUSH DE
    void OP_D6(); // SUB d8
    void OP_D7(); // RST 10H
    void OP_D8(); // RET C
    void OP_D9(); // RETI
    void OP_DA(); // JP C, a16
    void OP_DB(); // IN A, (C)
    void OP_DC(); // CALL C, a16
    void OP_DD(); // NOP
    void OP_DE(); // SBC A, d8
    void OP_DF(); // RST 18H
    void OP_E0(); // LD (a8), A
    void OP_E1(); // POP HL
    void OP_E2(); // LD (C), A
    void OP_E3(); // NOP
    void OP_E4(); // NOP
    void OP_E5(); // PUSH HL
    void OP_E6(); // AND d8
    void OP_E7(); // RST 20H
    void OP_E8(); // ADD SP, r8
    void OP_E9(); // JP (HL)
    void OP_EA(); // LD (a16), A
    void OP_EB(); // NOP
    void OP_EC(); // NOP
    void OP_ED(); // NOP
    void OP_EE(); // XOR d8
    void OP_EF(); // RST 28H
    void OP_F0(); // LD A, (a16)
    void OP_F1(); // POP AF
    void OP_F2(); // LD A, (C)
    void OP_F3(); // DI
    void OP_F4(); // NOP
    void OP_F5(); // PUSH AF
    void OP_F6(); // OR d8
    void OP_F7(); // RST 30H
    void OP_F8(); // LD HL, SP+r8
    void OP_F9(); // LD SP, HL
    void OP_FA(); // LD A, (a16)
    void OP_FB(); // EI
    void OP_FC(); // NOP
    void OP_FD(); // NOP
    void OP_FE(); // CP d8
    void OP_FF(); // RST 38H

    // CB-prefixed opcodes
    void CB_00(); // RLC B
    void CB_01(); // RLC C
    void CB_02(); // RLC D
    void CB_03(); // RLC E
    void CB_04(); // RLC H
    void CB_05(); // RLC L
    void CB_06(); // RLC (HL)
    void CB_07(); // RLC A
    void CB_08(); // RRC B
    void CB_09(); // RRC C
    void CB_0A(); // RRC D
    void CB_0B(); // RRC E
    void CB_0C(); // RRC H
    void CB_0D(); // RRC L
    void CB_0E(); // RRC (HL)
    void CB_0F(); // RRC A

    void OP_NULL(); // Handler for unimplemented opcodes

private:
    // Addressing modes
    uint16_t AddrMode_Immediate();
    uint16_t AddrMode_Immediate16();
    uint16_t AddrMode_Direct();
    uint16_t AddrMode_Register(uint8_t reg);
    uint16_t AddrMode_Indirect(uint8_t addr);

    void SetFlag(uint8_t flag, bool value);
    bool GetFlag(uint8_t flag) const;
    void UpdateFlags(); // update the 'flags variable'
    void UpdateFlagsAfterArithmetic(uint32_t result, uint16_t operand1, uint16_t operand2, bool isSubtraction);
    void UpdateFlagsAfterIncrement(int reg_index, int result);
    void UpdateFlagsAfterDecrement(int reg_index, int result);
    void UFAI16(uint16_t reg, int result); // update flags after increment but with 16 bit registers
    void UFAD16(uint16_t reg, int result); // same as above but with decremement flags
    void UFARA(uint16_t result, uint8_t op1, uint8_t op2, bool is_subtraction); // update flags after register arithmetic
    void UFAAO(uint8_t result); // update flags after and operation
    void UFAOXO(uint8_t result); // update flags after or / xor operation

    void PushToStack(uint16_t val);
    uint16_t PopFromStack();

    void CheckInterrupts();
    void HandleInterrupt();
    void ExecuteInterruptRoutine(int interruptIndex);
    //void UpdateInterrupts();

    void VBlankInterrupt();
    void LCDSTATInterrupt();
    void TimerInterrupt();
    void SerialInterrupt();
    void JoypadInterrupt();

    static const uint8_t FLAG_Z = 0x80; // Zero flag
    static const uint8_t FLAG_N = 0x40; // Subtract flag
    static const uint8_t FLAG_H = 0x20; // Half carry flag
    static const uint8_t FLAG_C = 0x10; // Carry flag

    uint16_t GetAF() const;
    void SetAF(uint16_t value);

    uint16_t GetBC() const;
    void SetBC(uint16_t value);

    uint16_t GetDE() const;
    void SetDE(uint16_t value);

    uint16_t GetHL() const;
    void SetHL(uint16_t value);

public:
    uint8_t val;
    static void RequestInterrupt(int index);

private:
    void register_out(); // displays the values of all the registers
};
