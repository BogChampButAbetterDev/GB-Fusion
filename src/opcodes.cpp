#include "CPU.h"

void CPU::OP_NULL()
{
    // Handle unimplemented opcode
    //std::cout << "Unimplemented opcode: 0x" << std::hex << opcode << std::endl;
    pc += 1;
}

void CPU::OP_00() {
    //no operation
    pc += 1; // next instruction
}

void CPU::OP_01() {
    uint16_t address = (this->*addressModeTable[2])();
    SetBC(address);
    pc += 3;
}

void CPU::OP_02() {
    uint16_t bc = GetBC();
    memory[bc] = registers[A];
    pc += 1;
}

void CPU::OP_03() {
    uint16_t bc = GetBC();
    uint16_t result = bc += 1;
    SetBC(result);
    pc += 1;
}

void CPU::OP_04() {
    signed int result = registers[B] += 1;
    registers[B] = result;
    UpdateFlagsAfterIncrement(B, result);
    pc += 1;
}

void CPU::OP_05() {
    signed int result = registers[B] -= 1;
    registers[B] = result;
    UpdateFlagsAfterDecrement(B, result);
    pc += 1;
}

void CPU::OP_06() {
    uint8_t address = (this->*addressModeTable[0])();
    registers[B] = address;
    pc += 2;
}

void CPU::OP_07() {
    // Rotate A left through the carry flag
    bool carryOut = (registers[A] & 0x80) != 0; // The bit that will be shifted out of A (bit 7)

    //              shift A left        shift the 7th bit out
    registers[A] = (registers[A] << 1) | (carryOut ? 0x01 : 0x00);

    // Set the flags
    SetFlag(FLAG_Z, false); // Zero flag is unaffected by RLCA
    SetFlag(FLAG_N, false); // Subtract flag
    SetFlag(FLAG_H, false); // Half-Carry flag
    SetFlag(FLAG_C, carryOut); // Carry flag

    pc += 1;
}

void CPU::OP_08() {
    uint16_t address = (this->*addressModeTable[1])();
    memory[address] = sp & 0xFF;       // Store low byte
    memory[address + 1] = (sp >> 8);   // Store high byte
    pc += 3; // advance by 3 to skip the operands
}

void CPU::OP_09() {
    int bc = GetBC();
    int hl = GetHL();
    int result = hl + bc;
    SetHL(result & 0xFFFF);

    UpdateFlagsAfterArithmetic(result, bc, hl, false);
    pc += 1;
}

void CPU::OP_0A() {
    uint16_t address = GetBC();
    registers[A] = memory[address];
    pc += 1;
}

void CPU::OP_0B() {
    uint16_t bc = GetBC();
    SetBC(bc += 1);
    pc += 1;
}

void CPU::OP_0C() {
    signed int result = registers[C] += 1;
    registers[C] = result;
    UpdateFlagsAfterIncrement(C, result);
    pc += 1;
}

void CPU::OP_0D() {
    signed int result = registers[C] -= 1;
    registers[C] = result;
    UpdateFlagsAfterDecrement(C, result);
    pc += 1;
}

void CPU::OP_0E() {
    uint8_t val = (this->*addressModeTable[0])();
    registers[C] = val;
    pc += 2;
}

void CPU::OP_0F() {
    bool oldcarry = (flags & FLAG_C) != 0;

    uint8_t lsb = (registers[A] & 0x01);
    registers[A] = (registers[A] >> 1) | (oldcarry ? 0x80 : 0x00);

    SetFlag(FLAG_Z, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);

    pc += 1;
}

void CPU::OP_10()
{
    stopped = true;
}

void CPU::OP_11()
{
    uint16_t address = (this->*addressModeTable[2])();
    SetDE(address);
    pc += 3;
}

void CPU::OP_12()
{
    uint16_t DE = GetBC();
    memory[DE] = registers[A];
    pc += 1;
}

void CPU::OP_13()
{
    uint16_t de = GetDE();
    uint16_t result = de += 1;
    SetDE(result);
    pc += 1;
}

void CPU::OP_14()
{
    signed int result = registers[D] += 1;
    registers[D] = result;
    UpdateFlagsAfterIncrement(D, result);
    pc += 1;
}

void CPU::OP_15()
{
    signed int result = registers[D] -= 1;
    registers[D] = result;
    UpdateFlagsAfterDecrement(D, result);
    pc += 1;
}

void CPU::OP_16()
{
    uint8_t address = (this->*addressModeTable[0])();
    registers[D] = address;
    pc += 2;
}

void CPU::OP_17()
{
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;

    SetFlag(FLAG_C, registers[A] & 0x80);

    registers[A] = (registers[A] << 1) | carry;

    SetFlag(FLAG_N, false);
    SetFlag(FLAG_H, false);

    pc += 1;
}

void CPU::OP_18()
{
    int8_t offset = (this->*addressModeTable[0])();
    pc += (offset - 2);
}

void CPU::OP_19()
{
    int de = GetDE();
    int hl = GetHL();
    int result = hl + de;

    SetHL(result & 0xFFFF);

    UpdateFlagsAfterArithmetic(result, hl, de, false);
    pc += 1;
}

void CPU::OP_1A()
{
    uint16_t address = GetDE();
    registers[A] = memory[address];
    pc += 1;
}

void CPU::OP_1B()
{
    uint16_t de = GetDE();
    SetDE(de -= 1);
    pc += 1;
}

void CPU::OP_1C()
{
    signed int result = registers[E] += 1;
    registers[E] = result;
    UpdateFlagsAfterIncrement(E, result);
    pc += 1;
}

void CPU::OP_1D()
{
    signed int result = registers[E] -= 1;
    registers[E] = result;
    UpdateFlagsAfterDecrement(E, result);
    pc += 1;
}

void CPU::OP_1E()
{
    val = (this->*addressModeTable[0])(); // Assuming this correctly fetches the immediate value
    registers[E] = val;
    pc += 2;
}

void CPU::OP_1F()
{
    // Get the current carry flag
    uint8_t carry = (flags & FLAG_C) ? 1 : 0;

    // Update the carry flag to the old bit 0 of register A
    SetFlag(FLAG_C, registers[A] & 0x01);

    // Rotate register A right through carry
    registers[A] = (registers[A] >> 1) | (carry << 7);

    // Update flags
    SetFlag(FLAG_Z, 0); // Z flag is always reset
    SetFlag(FLAG_N, 0); // N flag is always reset
    SetFlag(FLAG_H, 0); // H flag is always reset

    pc += 1;
}

void CPU::OP_20()
{
    int8_t offset = (this->*addressModeTable[0])();

    if (!GetFlag(FLAG_Z)) {
        pc += offset;
    }
    else {
        pc += 2;
    }
}

void CPU::OP_21()
{
    uint16_t address = (this->*addressModeTable[2])();
    SetHL(address);
    pc += 3;
}

void CPU::OP_22()
{
    uint16_t address = GetHL();
    memory[address] = registers[A];
    SetHL(address + 1);
    pc += 1;
}

void CPU::OP_23()
{
    uint16_t hl = GetHL();
    uint16_t result = hl += 1;
    SetHL(result);
    pc += 1;
}

void CPU::OP_24()
{
    signed int result = registers[H] += 1;
    registers[H] = result;
    UpdateFlagsAfterIncrement(H, result);
    pc += 1;
}

void CPU::OP_25()
{
    signed int result = registers[H] -= 1;
    registers[H] = result;
    UpdateFlagsAfterDecrement(H, result);
    pc += 1;
}

void CPU::OP_26()
{
    uint8_t address = (this->*addressModeTable[0])();
    registers[D] = address;
    pc += 1;
}

void CPU::OP_27()
{
    uint8_t correction = 0;

    if (!GetFlag(FLAG_N)) { // Addition
        if (GetFlag(H) || (registers[A] & 0x0F) > 9) {
            correction = 0x06;
        }
        if (GetFlag(C) || (registers[A] > 0x99)) {
            correction |= 0x60;
            SetFlag(C, true);
        }
    }
    else { // Subtraction
        if (GetFlag(H)) {
            correction = 0x06;
        }
        if (GetFlag(C)) {
            correction |= 0x60;
        }
    }

    registers[A] += GetFlag(FLAG_N) ? -correction : correction;

    SetFlag(H, false);
    SetFlag(FLAG_Z, registers[A] == 0);

    pc += 1;
}

void CPU::OP_28()
{
    int8_t offset = (this->*addressModeTable[0])();

    if (GetFlag(FLAG_Z)) {
        pc += offset;
    }
    else {
        pc += 2;
    }
}

void CPU::OP_29()
{
    int hl = GetHL();
    int result = hl + hl;

    SetHL(result & 0xFFFF);

    UpdateFlagsAfterArithmetic(result, hl, hl, false);

    pc += 1;
}

void CPU::OP_2A()
{
    uint16_t address = GetHL();
    registers[A] = memory[address];
    SetHL(address + 1);
    pc += 1;
}

void CPU::OP_2B()
{
    uint16_t hl = GetHL();
    SetHL(hl += 1);
    pc += 1;
}

void CPU::OP_2C()
{
    signed int result = registers[L] += 1;
    registers[L] = result;
    UpdateFlagsAfterIncrement(L, result);
    pc += 1;
}

void CPU::OP_2D()
{
    signed int result = registers[L] -= 1;
    registers[L] = result;
    UpdateFlagsAfterDecrement(L, result);
    pc += 1;
}

void CPU::OP_2E()
{
    uint8_t val = (this->*addressModeTable[0])();
    registers[L] = val;
    pc += 2;
}

void CPU::OP_2F()
{
    registers[A] = ~registers[A];
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_H, true);
    pc += 1;
}

void CPU::OP_30()
{
    int8_t offset = (this->*addressModeTable[0])();

    if (!GetFlag(FLAG_C)) {
        pc += offset;
    }
    else {
        pc += 2;
    }
}

void CPU::OP_31() {
    uint16_t address = (memory[pc + 1] | (memory[pc + 2] << 8));

    sp = address;

    pc += 3;
}

void CPU::OP_32()
{
    uint16_t address = GetHL();
    memory[address] = registers[A];
    SetHL(address - 1);
    pc += 1;
}

void CPU::OP_33()
{
    uint16_t value = sp + 1;
    memory[sp + 1] = value & 0xFF;
    memory[sp + 2] = (value >> 8) & 0xFF;
    pc += 1;
}

void CPU::OP_34()
{
    uint16_t hl = GetHL();
    uint8_t value = memory[hl];
    value += 1;
    memory[hl] = value;

    UFAI16(hl, value);
    pc += 1;
}

void CPU::OP_35()
{
    uint16_t hl = GetHL();
    uint8_t value = memory[hl];
    value -= 1;
    memory[hl] = value;

    UFAD16(hl, value);

    pc += 1;
}

void CPU::OP_36()
{
    uint8_t val = (this->*addressModeTable[0])(); // use immediate addressing to get d8
    uint16_t hl = GetHL();
    memory[hl] = val;
    pc += 2;
}

void CPU::OP_37()
{
    SetFlag(FLAG_C, true);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    pc += 1;
}

void CPU::OP_38()
{
    int8_t offset = (this->*addressModeTable[0])();

    if (GetFlag(FLAG_C)) {
        pc += offset;
    }
    else {
        pc += 2;
    }
}

void CPU::OP_39()
{
    int hl = GetHL();
    int result = hl + sp;

    SetHL(result & 0xFFFF);

    UpdateFlagsAfterArithmetic(result, hl, sp, false);

    pc += 1;
}

void CPU::OP_3A()
{
    uint16_t address = GetHL();
    registers[A] = memory[address];
    SetHL(address - 1);
    pc += 1;
}

void CPU::OP_3B()
{
    sp -= 1;
    pc += 1;
}

void CPU::OP_3C()
{
    int result = registers[A] += 1;
    UpdateFlagsAfterIncrement(A, result);
    pc += 1;
}

void CPU::OP_3D()
{
    int result = registers[A] -= 1;
    UpdateFlagsAfterDecrement(A, result);
    pc += 1;
}

void CPU::OP_3E()
{
    uint8_t val = (this->*addressModeTable[0])();
    registers[A] = val;
    pc += 2;
}

void CPU::OP_3F()
{
    bool carry = GetFlag(FLAG_C);
    SetFlag(FLAG_C, !carry);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    pc += 1;
}

void CPU::OP_40() {
    // just loads b into b doesn't have to do anything
    pc += 1;
}

void CPU::OP_41() {
    registers[B] = registers[C];
    pc += 1;
}

void CPU::OP_42() {
    registers[B] = registers[D];
    pc += 1;
}

void CPU::OP_43() {
    registers[B] = registers[E];
    pc += 1;
}

void CPU::OP_44() {
    registers[B] = registers[H];
    pc += 1;
}

void CPU::OP_45() {
    registers[B] = registers[L];
    pc += 1;
}

void CPU::OP_46() 
{
    registers[B] = memory[GetHL()];
    pc += 1;
}

void CPU::OP_47() {
    registers[B] = registers[A];
    pc += 1;
}

void CPU::OP_48() 
{
    registers[C] = registers[B];
    pc += 1;
}

void CPU::OP_49() {
    //load c into c doesn't have to do anything
    pc += 1;
}

void CPU::OP_4A() {
    registers[C] = registers[D];
    pc += 1;
}

void CPU::OP_4B() {
    registers[C] = registers[E];
    pc += 1;
}

void CPU::OP_4C() {
    registers[C] = registers[H];
    pc += 1;
}

void CPU::OP_4D() {
    registers[C] = registers[L];
    pc += 1;
}

void CPU::OP_4E() 
{
    registers[C] = memory[GetHL()];
    pc += 1;
}

void CPU::OP_4F() {
    registers[C] = registers[A];
    pc += 1;
}

void CPU::OP_50() {
    registers[D] = registers[B];
    pc += 1;
}

void CPU::OP_51() {
    registers[D] = registers[C];
    pc += 1;
}

void CPU::OP_52() {
    // no load
    pc += 1;
}

void CPU::OP_53() {
    registers[D] = registers[E];
    pc += 1;
}

void CPU::OP_54() {
    registers[D] = registers[H];
    pc += 1;
}

void CPU::OP_55() {
    registers[D] = registers[L];
    pc += 1;
}

void CPU::OP_56() {
    registers[D] = memory[GetHL()];
    pc += 1;
}

void CPU::OP_57() {
    registers[D] = registers[A];
    pc += 1;
}

void CPU::OP_58() {
    registers[E] = registers[B];
    pc += 1;
}

void CPU::OP_59() {
    registers[E] = registers[C];
    pc += 1;
}

void CPU::OP_5A() {
    registers[E] = registers[D];
    pc += 1;
}

void CPU::OP_5B() {
    // no load
    pc += 1;
}

void CPU::OP_5C() {
    registers[E] = registers[H];
    pc += 1;
}

void CPU::OP_5D() {
    registers[E] = registers[L];
    pc += 1;
}

void CPU::OP_5E() {
    registers[E] = memory[GetHL()];
    pc += 1;
}

void CPU::OP_5F() {
    registers[E] = registers[A];
    pc += 1;
}

void CPU::OP_60() {
    registers[H] = registers[B];
    pc += 1;
}

void CPU::OP_61() {
    registers[H] = registers[C];
    pc += 1;
}

void CPU::OP_62() {
    registers[H] = registers[D];
    pc += 1;
}

void CPU::OP_63() {
    registers[H] = registers[E];
    pc += 1;
}

void CPU::OP_64() {
    // no load
    pc += 1;
}

void CPU::OP_65() {
    registers[H] = registers[L];
    pc += 1;
}

void CPU::OP_66() {
    registers[H] = memory[GetHL()];
    pc += 1;
}

void CPU::OP_67() {
    registers[H] = registers[A];
    pc += 1;
}

void CPU::OP_68() {
    registers[L] = registers[B];
    pc += 1;
}

void CPU::OP_69() {
    registers[L] = registers[C];
    pc += 1;
}

void CPU::OP_6A() {
    registers[L] = registers[D];
    pc += 1;
}

void CPU::OP_6B() {
    registers[L] = registers[E];
    pc += 1;
}

void CPU::OP_6C() {
    registers[L] = registers[H];
    pc += 1;
}

void CPU::OP_6D() {
    // no load 
    pc += 1;
}

void CPU::OP_6E() {
    registers[L] = memory[GetHL()];
    pc += 1;
}

void CPU::OP_6F() {
    registers[L] = registers[A];
    pc += 1;
}

void CPU::OP_70() {
    memory[GetHL()] = registers[B];
    pc += 1;
}

void CPU::OP_71() {
    memory[GetHL()] = registers[C];
    pc += 1;
}

void CPU::OP_72() {
    memory[GetHL()] = registers[D];
    pc += 1;
}

void CPU::OP_73() {
    memory[GetHL()] = registers[E];
    pc += 1;
}

void CPU::OP_74() {
    memory[GetHL()] = registers[H];
    pc += 1;
}

void CPU::OP_75() {
    memory[GetHL()] = registers[C];
    pc += 1;
}

void CPU::OP_76() {
    halted = true;
    pc += 1;
}

void CPU::OP_77() {
    memory[GetHL()] = registers[A];
    pc += 1;
}

void CPU::OP_78() {
    registers[A] = registers[B];
    pc += 1;
}

void CPU::OP_79() {
    registers[A] = registers[C];
    pc += 1;
}

void CPU::OP_7A() {
    registers[A] = registers[D];
    pc += 1;
}

void CPU::OP_7B() {
    registers[A] = registers[E];
    pc += 1;
}

void CPU::OP_7C() {
    registers[A] = registers[H];
    pc += 1;
}

void CPU::OP_7D() {
    registers[A] = registers[L];
    pc += 1;
}

void CPU::OP_7E() {
    registers[A] = memory[GetHL()];
    pc += 1;
}

void CPU::OP_7F() {
    // no load
    pc += 1;
}

void CPU::OP_80() {
    uint16_t result = registers[A] + registers[B];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[B], false);
    pc += 1;
}

void CPU::OP_81() {
    uint16_t result = registers[A] + registers[C];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[C], false);
    pc += 1;
}

void CPU::OP_82() {
    uint16_t result = registers[A] + registers[D];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[D], false);
    pc += 1;
}

void CPU::OP_83() {
    uint16_t result = registers[A] + registers[E];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[E], false);
    
    pc += 1;
}

void CPU::OP_84() {
    uint16_t result = registers[A] + registers[H];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[H], false);
    
    pc += 1;
}

void CPU::OP_85() {
    uint16_t result = registers[A] + registers[L];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[L], false);
    
    pc += 1;
}

void CPU::OP_86() {
    uint8_t value = memory[GetHL()];
    uint16_t result = registers[A] + value; 
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], memory[GetHL()], false);

    pc += 1;
}

void CPU::OP_87() {
    uint16_t result = registers[A] + registers[A];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[A], false);

    pc += 1;
}

void CPU::OP_88() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + registers[B] + carry;
    
    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);
    
    bool halfcarry = ((registers[A] & 0x0F) + (registers[B] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_89() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + registers[C] + carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[C] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_8A() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + registers[D] + carry;
    
    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);
    
    bool halfcarry = ((registers[A] & 0x0F) + (registers[D] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_8B() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + registers[E] + carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[E] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_8C() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + registers[H] + carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[H] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_8D() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + registers[L] + carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[L] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_8E() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + memory[GetHL()] + carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (memory[GetHL()] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_8F() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] + registers[A] + carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[A] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_90() {
    uint16_t result = registers[A] - registers[B];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[B], true);
    pc += 1;
}

void CPU::OP_91() {
    uint16_t result = registers[A] - registers[C];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[C], true);
    pc += 1;
}

void CPU::OP_92() {
    uint16_t result = registers[A] - registers[D];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[D], true);
    pc += 1;
}

void CPU::OP_93() {
    uint16_t result = registers[A] - registers[E];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[E], true);
    pc += 1;
}

void CPU::OP_94() {
    uint16_t result = registers[A] - registers[H];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[H], true);
    pc += 1;
}

void CPU::OP_95() {
    uint16_t result = registers[A] - registers[L];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[L], true);
    pc += 1;
}

void CPU::OP_96() {
    uint16_t result = registers[A] - memory[GetHL()];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], memory[GetHL()], true);
    pc += 1;
}

void CPU::OP_97() {
    uint16_t result = registers[A] - registers[A];
    registers[A] = result & 0xFF;
    UFARA(result, registers[A], registers[A], true);
    pc += 1;
}

void CPU::OP_98() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - registers[B] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[B] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_99() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - registers[C] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[C] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_9A() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - registers[D] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[D] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_9B() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - registers[E] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[E] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_9C() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - registers[H] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[H] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_9D() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - registers[L] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[L] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_9E() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - memory[GetHL()] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (memory[GetHL()] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_9F() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t result = registers[A] - registers[A] - carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (registers[A] & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_A0() {
    uint8_t result = registers[A] &= registers[B];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A1() {
    uint8_t result = registers[A] &= registers[C];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A2() {
    uint8_t result = registers[A] &= registers[D];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A3() {
    uint8_t result = registers[A] &= registers[E];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A4() {
    uint8_t result = registers[A] &= registers[H];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A5() {
    uint8_t result = registers[A] &= registers[L];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A6() {
    uint8_t result = registers[A] &= memory[GetHL()];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A7() {
    uint8_t result = registers[A] &= registers[A];
    UFAAO(result);
    pc += 1;
}

void CPU::OP_A8() {
    uint8_t result = registers[A] ^= registers[B];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_A9() {
    uint8_t result = registers[A] ^= registers[C];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_AA() {
    uint8_t result = registers[A] ^= registers[D];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_AB() {
    uint8_t result = registers[A] ^= registers[E];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_AC() {
    uint8_t result = registers[A] ^= registers[H];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_AD() {
    uint8_t result = registers[A] ^= registers[L];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_AE() {
    uint8_t result = registers[A] ^= memory[GetHL()];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_AF() {
    uint8_t result = registers[A] ^= registers[A];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B0() {
    uint8_t result = registers[A] |= registers[B];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B1() {
    uint8_t result = registers[A] |= registers[C];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B2() {
    uint8_t result = registers[A] |= registers[D];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B3() {
    uint8_t result = registers[A] |= registers[E];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B4() {
    uint8_t result = registers[A] |= registers[H];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B5() {
    uint8_t result = registers[A] |= registers[L];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B6() {
    uint8_t result = registers[A] |= memory[GetHL()];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B7() {
    uint8_t result = registers[A] |= registers[A];
    UFAOXO(result);
    pc += 1;
}

void CPU::OP_B8() {
    uint8_t result = registers[A] - registers[B];
    UFARA(result, registers[A], registers[B], true);
    pc += 1;
}

void CPU::OP_B9() {
    uint8_t result = registers[A] - registers[C];
    UFARA(result, registers[A], registers[C], true);
    pc += 1;
}

void CPU::OP_BA() {
    uint8_t result = registers[A] - registers[D];
    UFARA(result, registers[A], registers[D], true);
    pc += 1;
}

void CPU::OP_BB() {
    uint8_t result = registers[A] - registers[E];
    UFARA(result, registers[A], registers[E], true);
    pc += 1;
}

void CPU::OP_BC() {
    uint8_t result = registers[A] - registers[H];
    UFARA(result, registers[A], registers[H], true);
    pc += 1;
}

void CPU::OP_BD() {
    uint8_t result = registers[A] - registers[L];
    UFARA(result, registers[A], registers[L], true);
    pc += 1;
}

void CPU::OP_BE() {
    uint8_t result = registers[A] - memory[GetHL()];
    UFARA(result, registers[A], memory[GetHL()], true);
    pc += 1;
}

void CPU::OP_BF() {
    uint8_t result = registers[A] - registers[A];
    UFARA(result, registers[A], registers[A], true);
    pc += 1;
}

void CPU::OP_C0() {
    // return operations don't increment the PC
    if (!GetFlag(FLAG_Z))
    {
        pc = PopFromStack();
    }
    else
    {
        pc += 1;
    }
}

void CPU::OP_C1() {
    SetBC(PopFromStack());
    pc += 1;
}

void CPU::OP_C2() {
    if (!GetFlag(FLAG_Z))
    {
        uint16_t offset = (this->*addressModeTable[2])();
        pc += offset;
    }
    else 
    {
        pc += 3;
    }
}

void CPU::OP_C3() {
    uint16_t offset = (this->*addressModeTable[2])();
    pc += offset;
}

void CPU::OP_C4() {
    if (!GetFlag(FLAG_Z))
    {
        uint16_t returnaddr = pc + 3;
        PushToStack(returnaddr);

        uint16_t addr = (this->*addressModeTable[2])();
        pc = addr;
    }
    else
    {
        pc += 3;
    }
}

void CPU::OP_C5() {
    PushToStack(GetBC());
    pc += 1;
}

void CPU::OP_C6() {
    uint8_t op2 = (this->*addressModeTable[0])();
    uint16_t result = registers[A] += op2;
    UFARA(result, registers[A], op2, false);
    pc += 2;
}

void CPU::OP_C7() {
    PushToStack(pc);
    pc = 0x0000;
}

void CPU::OP_C8() {
    if (GetFlag(FLAG_Z))
    {
        pc = PopFromStack();
    }
    else 
    {
        pc += 1;
    }
}

void CPU::OP_C9()
{
    pc = PopFromStack();
}

void CPU::OP_CA() {
    if (GetFlag(FLAG_Z)) 
    {
        uint16_t offset = (this->*addressModeTable[2])();
        pc += offset;
    }
    else
    {
        pc += 3;
    }
}

void CPU::OP_CC() {
    if (GetFlag(FLAG_Z))
    {
        uint16_t returnaddr = pc + 3;
        PushToStack(returnaddr);

        uint16_t addr = (this->*addressModeTable[2])();
        pc = addr;
    }
    else
    {
        pc += 3;
    }
}

void CPU::OP_CD() {
    uint16_t returnaddr = pc + 3;
    PushToStack(returnaddr);

    uint16_t addr = (this->*addressModeTable[2])();
    pc = addr;
}

void CPU::OP_CE() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t op2 = (this->*addressModeTable[0])();
    uint8_t result = registers[A] + op2 + carry;

    registers[A] = result & 0xFF;

    SetFlag(FLAG_Z, registers[A] == 0);
    SetFlag(FLAG_N, false);

    bool halfcarry = ((registers[A] & 0x0F) + (op2 & 0x0F) + carry) > 0x0F;
    SetFlag(FLAG_H, halfcarry);

    SetFlag(FLAG_C, result > 0xFF);
    pc += 1;
}

void CPU::OP_CF() {
    PushToStack(pc);
    pc = 0x08;
}

void CPU::OP_D0() {
    if (!GetFlag(FLAG_C))
    {
        pc = PopFromStack();
    }
    else
    {
        pc += 1;
    }
}

void CPU::OP_D1() {
    SetDE(PopFromStack());
    pc += 1;
}

void CPU::OP_D2() {
    if (!GetFlag(FLAG_C))
    {
        uint16_t offset = (this->*addressModeTable[2])();
        pc += offset;
    }
    else
    {
        pc += 3;
    }
}

void CPU::OP_D3() {
    io_port[registers[C]] = registers[A];
    pc += 1;
}

void CPU::OP_D4() {
    if (!GetFlag(FLAG_C))
    {
        uint16_t returnaddr = pc + 3;
        PushToStack(returnaddr);

        uint16_t addr = (this->*addressModeTable[2])();
        pc = addr;
    }
    else
    {
        pc += 3;
    }
}

void CPU::OP_D5() {
    PushToStack(GetDE());
    pc += 1;
}

void CPU::OP_D6() {
    uint8_t op2 = (this->*addressModeTable[0])();
    uint16_t result = registers[A] += op2;
    UFARA(result, registers[A], op2, true);
    pc += 2;
}

void CPU::OP_D7() {
    PushToStack(pc);
    pc = 0x0010;
}

void CPU::OP_D8() {
    if (GetFlag(FLAG_C))
    {
        pc = PopFromStack();
    }
    else
    {
        pc += 1;
    }
}

void CPU::OP_D9() {
    pc = PopFromStack();
    IME = true;
}

void CPU::OP_DA() {
    if (GetFlag(FLAG_C))
    {
        uint16_t offset = (this->*addressModeTable[2])();
        pc += offset;
    }
    else
    {
        pc += 3;
    }
}

void CPU::OP_DB() {
    registers[A] = io_port[registers[C]];
    pc += 1;
}

void CPU::OP_DC() {
    if (GetFlag(FLAG_C))
    {
        uint16_t returnaddr = pc + 3;
        PushToStack(returnaddr);

        uint16_t addr = (this->*addressModeTable[2])();
        pc = addr;
    }
    else
    {
        pc += 3;
    }
}

void CPU::OP_DD()
{
    pc += 1;
}

void CPU::OP_DE() {
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t op2 = (this->*addressModeTable[0])();
    uint8_t result = registers[A] - op2 - carry;

    registers[A] = result;

    // Set the Zero flag
    SetFlag(FLAG_Z, registers[A] == 0);
    // Set the Subtract (N) flag
    SetFlag(FLAG_N, true);

    // Set the Half-Carry (H) flag
    bool halfcarry = ((registers[A] & 0x0F) < (op2 & 0x0F) + carry);
    SetFlag(FLAG_H, halfcarry);

    // Set the Carry (C) flag
    SetFlag(FLAG_C, (result > 0xFF));

    pc += 1;
}

void CPU::OP_DF() {
    PushToStack(pc);
    pc = 0x18;
}

void CPU::OP_E0() {
    uint8_t val = (this->*addressModeTable[0])();
    uint16_t addr = 0xFF00 + val;
    memory[addr] = registers[A];
    pc += 2;
}

void CPU::OP_E1() {
    SetHL(PopFromStack());
    pc += 1;
}

void CPU::OP_E2() {
    memory[registers[C]] = registers[A];
    pc += 2;
}

void CPU::OP_E3()
{
    pc += 1;
}

void CPU::OP_E4()
{
    pc += 1;
}

void CPU::OP_E5() {
    PushToStack(GetHL());
    pc += 1;
}

void CPU::OP_E6() {
    uint8_t val = (this->*addressModeTable[0])();
    registers[A] &= val;
    pc += 2;
}

void CPU::OP_E7() {
    PushToStack(pc);
    pc = 0x0020;
}

void CPU::OP_E8() {
    int8_t immediateValue = static_cast<int8_t>(memory[pc + 1]); // use this since I need to fetch a signed int
    uint16_t originalSP = sp;

    sp = sp + immediateValue;

    // Set flags
    SetFlag(FLAG_Z, false); 
    SetFlag(FLAG_N, false);

    SetFlag(FLAG_H, ((originalSP & 0xF) + (immediateValue & 0xF)) > 0xF);

    SetFlag(FLAG_C, ((originalSP & 0xFF) + (immediateValue & 0xFF)) > 0xFF);

    pc += 2;
}

void CPU::OP_E9() {
    pc = GetHL();
}

void CPU::OP_EA() {
    uint16_t addr = (this->*addressModeTable[2])();
    memory[addr] = registers[A];
    pc += 3;
}

void CPU::OP_EB()
{
    pc += 1;
}

void CPU::OP_EC()
{
    pc += 1;
}

void CPU::OP_ED()
{
    pc += 1;
}

void CPU::OP_EE() {
    uint8_t val = (this->*addressModeTable[0])();
    registers[A] ^= val;
    pc += 2;
}

void CPU::OP_EF() {
    PushToStack(pc);
    pc = 0x0028;
}

void CPU::OP_F0() {
    uint8_t val = (this->*addressModeTable[0])();
    registers[A] = memory[val];
    pc += 2;
}

void CPU::OP_F1() {
    SetAF(PopFromStack());
    pc += 1;
}

void CPU::OP_F2() {
    registers[A] = memory[registers[C]];
    pc += 2;
}

void CPU::OP_F3() {
    IME = false;
    pc += 1;
}

void CPU::OP_F4() {
    pc += 1;
}

void CPU::OP_F5() {
    PushToStack(GetAF());
    pc += 1;
}

void CPU::OP_F6() {
    uint8_t val = (this->*addressModeTable[0])();
    uint16_t result = registers[A] |= val;
    UFAOXO(result);
    pc += 2;
}

void CPU::OP_F7() {
    PushToStack(pc);
    pc = 0x0030;
}

void CPU::OP_F8() {
    int8_t offset = (int8_t)memory[pc + 1];

    uint16_t result = sp + offset;

    SetHL(result);

    SetFlag(FLAG_N, false); 
    SetFlag(FLAG_Z, false); 

    bool halfCarry = ((sp & 0xF) + (offset & 0xF)) > 0xF;
    SetFlag(FLAG_H, halfCarry);

    bool carry = ((sp & 0xFF) + (offset & 0xFF)) > 0xFF;
    SetFlag(FLAG_C, carry);
    
    pc += 2; 
}

void CPU::OP_F9() {
    sp = GetHL();
    pc += 1;
}

void CPU::OP_FA() {
    uint16_t val = (this->*addressModeTable[2])();
    registers[A] = val;
    pc += 3; 
}

void CPU::OP_FB() {
    IME = true;
    pc += 1;
}

void CPU::OP_FC() {
    pc += 1;
}

void CPU::OP_FD() {
    pc += 1;
}

void CPU::OP_FE() {
    uint8_t val = (this->*addressModeTable[0])();
    val = ~val;
    pc += 2;
}

void CPU::OP_FF() {
    PushToStack(pc);
    pc = 0x0038;
}

void CPU::CB_00() {}
void CPU::CB_01() {}
void CPU::CB_02() {}
void CPU::CB_03() {}
void CPU::CB_04() {}
void CPU::CB_05() {}
void CPU::CB_06() {}
void CPU::CB_07() {}
void CPU::CB_08() {}
void CPU::CB_09() {}
void CPU::CB_0A() {}
void CPU::CB_0B() {}
void CPU::CB_0C() {}
void CPU::CB_0D() {}
void CPU::CB_0E() {}
void CPU::CB_0F() {}
