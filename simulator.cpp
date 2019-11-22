#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include "instructions.hpp"

//NOTE regSource = rs and regSource2 = rt. Confusing slightly..

void r_type(int regSource, int regSource2, int regDestination, int shamt, int function_code);
void i_type(int opcode, int rs, int rt, int16_t immediate);
void j_type(int opcode, int target);

struct Registers
{
  public:
    std::vector<uint32_t> register_vector;

    uint32_t hi;// hih register
    uint32_t lo; //lo register
    uint32_t Program_Counter; // pc


    //program counter
    void increment_Program_Counter()
    {
      Program_Counter = Program_Counter + 4;
    }
    // all registers
    uint32_t Get_Register(uint32_t RegisterIndex)const
    {
        return register_vector[RegisterIndex];
    }
    void Set_Register(uint32_t RegisterIndex, uint32_t Value)
    {
      if (RegisterIndex != 0)
      {
        register_vector[RegisterIndex] = Value;
      }
    }

} CPU_Registers;


struct memory{
  public:

    std::vector<int> ADDR_INSTR;
    std::vector<int> ADDR_DATA;

    // reading from memory
    int read_ADDR_INSTR(int index)
    {
      return ADDR_INSTR[index];
    }

    int read_ADDR_DATA(int index)
    {
      return ADDR_DATA[index];
    }

    uint32_t read_ADDR_GETC()  // get character...
    {
      char character;
      character = getchar();
      if(std::cin.eof())
      { // if end of line
        return 0b11111111111111111111111111111111;
      }
      else
      {
        return character;
      }
    }


    // writing to memory
    void write_ADDR_PUTC(int input_data)
    {
      if(input_data == 0)
      {
          return;
      }
      else if(putchar(input_data))
      {
          return;
      }
      else
      {
          exit(-21);
      }


    }

    void write_ADDR_DATA(int input_data, int index)
    {
        ADDR_DATA[index] = input_data;
    }
    void load_ADDR_INSTR(int index, int number)
    {
        ADDR_INSTR[index] = number;
    }

    uint32_t memory_map_test(int32_t address, int data, int opcode, bool R_flag, bool W_flag, bool E_flag)
    {
        // in mem map?
        if((address < 0) or (address > 0x30000008))
        {
            exit(-11);
        }

        // jump to 0, exit
        else if(address == 0x0)
        {
          if((R_flag == false) && (W_flag == false) && (E_flag == true))
          {
            exit(CPU_Registers.Get_Register(2));
          }
          else
          {
            exit(-11);
          }
        }

        //first blocked mem space
        else if((address >= 4) && (address < 0x10000000))
        {
          exit(-11);
        }

        // R_flag instr mem, exit if tryna W_flag
        else if((address >= 0x10000000) && (address < 0x11000000))
        {
          address = address & 0xFFFFFFFC;
          if(W_flag == true)
          {
            exit(-11);
          }
          else if(R_flag == true)
          {
            return read_ADDR_INSTR((address - 0x10000000)/4); // divide for pc
          }
          else if(E_flag == true)
          {
            return (address - 0x10000000);
          }
          else
          {
            exit(-11);
          }
        }
        // second blocked off mem from map
        else if((address >= 0x11000000) && (address < 0x20000000))
        {
          exit(-11);
        }
        // DATA BABY!
        else if((address >= 0x20000000) && (address < 24000000))
        {
          address = address & 0xFFFFFFFC;
          if((R_flag==true) && (W_flag==false) && (E_flag == false))
          {
            return read_ADDR_DATA((address - 0x20000000)/4);
          }
          else if((R_flag==false) && (W_flag==true) && (E_flag == false))
          {
            write_ADDR_DATA(data, (address - 0x20000000)/4);
          }
          else
          {
            exit(-11);
          }
        }

        // unused memory: memory error is accessed
        else if((address >= 0x24000000) && (address < 0x30000000))
        {
          exit(-11);
        }

        // getchar!
        else if((address >= 0x30000000) && (address < 0x30000004))
        {
          if(W_flag || E_flag)
          {
            exit(-11);
          }
          else if(R_flag)
          {
            //LW and LWL/LWRcase
            if((opcode == 0b100011) || (opcode == 0b100010) || (opcode == 0b100110))
            {
              uint32_t x;
              x = read_ADDR_GETC();
              if(x == 0xFFFFFFFF)
              {
                return -1;
              }
              else
              {
                return x;
              }
            }
            //LB and LBU
            else if(((opcode == 0b100000) || (opcode == 0b100100)) && (address == 0x30000003))
            {
              uint32_t x;
              x = read_ADDR_GETC();
              if(x == 0xFFFFFFFF)
              {
                return -1;
              }
              else
              {
                return x;
              }
            }
            // LB & LBU
            else if(((opcode == 0b100000) || (opcode == 0b100100)) && (address != 0x30000003))
            {
              read_ADDR_GETC();
              return 0;
            }
            //LH and LHU cases
            else if(((opcode == 0b100101) || (opcode == 0b100001)) && (address == 0x30000002))
            {
              uint32_t x;
              x = read_ADDR_GETC();
              if(x == 0xFFFFFFFF)
              {
                return -1;
              }
              else
              {
                return x;
              }
            }
            else if(((opcode == 0b100101) || (opcode == 0b100001)) && (address != 0x30000002))
            {
              read_ADDR_GETC();
              return 0;
            }
          }
          else
          {
            exit(-20);
          }
        }

        //putchar!
        else if((address >= 0x30000004) && (address < 0x30000008))
        {
          if((R_flag == true) || (E_flag == true))
          {
            exit(-11);
          }
          else if(W_flag)
          {
            if(opcode == 0b101011)
            {
              write_ADDR_PUTC(data & 0xFF);
            }
            //SB case
            else if((opcode == 0b101000) && (address == 0x30000007))
            {
              write_ADDR_PUTC(data & 0xFF);
            }
            else if((opcode == 0b101000) && (address != 0x30000007))
            {
              write_ADDR_PUTC(0);
            }
            //SH case
            else if((opcode == 0b101001) && (address == 0x30000006))
            {
              write_ADDR_PUTC(data & 0xFF);
            }
            else if((opcode == 0b101001) && (address != 0x30000006))
            {
              write_ADDR_PUTC(0);
            }
          }
          else
          {
            exit(-20);
          }
        }

        // unused memory: memory error is accessed
        else if(address >= 0x30000008)
        {
          exit(-11);
        }

        // if none of the previous paths are taken there is a memory error
        else
        {
          exit(-11);
        }
    }

} Mips_memory;

void determine_instruction_to_execute(int binary)
{

  uint32_t opcode = (binary >> 26) & 0x3F;
  // R type instructions//
  if(opcode== 0b000000)
  {
    //=========================== RS ============================//

    uint32_t regSource = binary & 0b00000011111000000000000000000000;
    regSource = regSource >> 21;

    //=========================== RT ============================//

    uint32_t regSource2 = binary & 0b00000000000111110000000000000000;
    regSource2 = regSource2 >> 16;

    //=========================== RD ============================//

    uint32_t regDestination = binary & 0b00000000000000001111100000000000;
    regDestination = regDestination >> 11;

    //=========================== SHAMT ============================//

    uint32_t shamt = binary & 0b00000000000000000000011111000000;
    shamt = (shamt >> 6)& 0b11111;

    //=========================== FunctionCode ============================//

    uint32_t function_code = binary & 0b00000000000000000000000000111111;

    //=========================== Executing Instruction of type R  ============================//

    return r_type(regSource, regSource2, regDestination, shamt, function_code);

  }
  // J type instructions //
  else if(opcode == 0b000011 || opcode == 0b000010)
  {
    //=========================== register to jump to ============================//

    uint32_t Register_jumped_to = binary & 0b00000011111111111111111111111111;

    //=========================== Executing Instruction of type J  ============================//

    return j_type(opcode, Register_jumped_to);
  }

  // I Type Instructions //
  else if(opcode == 0b001000 || opcode == 0b001001 || opcode == 0b001100 || opcode == 0b101000 || \
          opcode == 0b001010 || opcode == 0b001011 || opcode == 0b101001 || opcode == 0b000101 || \
          opcode == 0b100000 || opcode == 0b100100 || opcode == 0b100001 || opcode == 0b000100 || \
          opcode == 0b000001 || opcode == 0b000111 || opcode == 0b000110 || opcode == 0b100101 || \
          opcode == 0b001111 || opcode == 0b100011 || opcode == 0b001101 || opcode == 0b101011 || \
          opcode == 0b001110 || opcode == 0b100010 || opcode == 0b100110){

        //=========================== RS ============================//
      uint32_t regSource = binary & 0b00000011111000000000000000000000;
      regSource = regSource >> 21;

      //=========================== RT ============================//
      uint32_t regSource2 = binary & 0b00000000000111110000000000000000;
      regSource2 = regSource2 >> 16;

      //=========================== IMMEDIATE ============================//

      int16_t immediate = binary & 0b00000000000000001111111111111111;
      //=========================== Executing Instruction of type I  ============================//
      return i_type(opcode, regSource, regSource2, immediate);
  }
  // invalid instruction if opcode doesn't correspond to one of the above
  else{
      exit(-12);
  }

}

void r_type(int regSource, int regSource2, int regDestination, int shamt, int function_code)
{
  // doing it in order of complexity.
  if (function_code == 0b100001)  // ADDU
  {
    ADDU(regSource, regSource2, regDestination);
  }
  else if(function_code == 0b100000)
  {
    ADD(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b100100) // AND
  {
    AND(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b001000) // JR
  {
    JR(regSource);
  }
  else if(function_code == 0b100101) //OR
  {
    OR(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b101011)   // SLTU
  {
    SLTU(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b100011) //SUBU
  {
    SUBU(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b100110){ // XOR
    XOR(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b000111) // SRAV
  {
    SRAV(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b000000)  //SLL
  {
    SLL(regSource2,regDestination, shamt);
  }
  else if(function_code == 0b101010) //SLT
  {
    SLT(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b000011){ //SRA
    SRA(regSource2,regDestination,shamt);
  }
  else if(function_code == 0b000010) //SRL
  { //regDestination,SHAMT
    SRL(regSource2,regDestination,shamt);
  }
  else if(function_code == 0b100010) //SUB
  {
    SUB(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b000100) //SLLV
  {
    SLLV(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b000110) // SRLV
  {
    SRLV(regSource,regSource2,regDestination);
  }
  else if(function_code == 0b010000) //MFHI
  {
    MFHI(regDestination);
  }

  else if(function_code == 0b010010) //MFLO
  {
    MFLO(regDestination);
  }
  else if(function_code == 0b010001) //MTHI
  {
    MTHI(regSource);
  }
  else if(function_code == 0b010011) //MTLO
  {
    MTLO(regSource);
  }

  else if(function_code == 0b001001) //JALR
  {
    JALR(regSource,regDestination);
  }
  else if(function_code == 0b011010)
  {
    DIV(regSource,regSource2);
  }
  else if(function_code == 0b011011) //DIVU
  {
    DIVU(regSource,regSource2);
  }
  else if(function_code == 0b011000) // MULT
  {
    MULT(regSource,regSource2);
  }
  else if(function_code == 0b011001) //MULTU
  {
    MULTU(regSource,regSource2);
  }
  else
  {
      exit(-12); // if function code not found.
  }

}

void j_type(int opcode, int target)
{
  //jump PC = nPC; nPC = (PC & 0xf0000000) | (target << 2);
    if(opcode == 0b000010) //JAL
    {
      jump(opcode,target);
    }
    else if(opcode == 0b000011)
    {
      JAL(opcode,target);
    }

}
void i_type(int opcode, int regSource, int regSource2, int16_t immediate)
{
  if(opcode == 0b001110) //XORI
  {
    XORI(regSource,regSource2,immediate);
  }
  else if(opcode == 0b001000) //ADDI
  {
    ADDI(regSource,regSource2,immediate);
  }
  else if(opcode == 0b001001) //ADDIU
  {
    ADDIU(regSource,regSource2,immediate);
  }
  else if(opcode == 0b001100)//ANDI
  {
    ANDI(regSource,regSource2,immediate);
  }
  else if(opcode == 0b001101) //ORI
  {
    ORI(regSource,regSource2,immediate);
  }
  else if(opcode == 0b001111) //LUI
  {
    LUI(regSource2,immediate);
  }
  else if(opcode == 0b100011) //LW
  {
    LW(regSource,regSource2,immediate);
  }
  else if(opcode == 0b101011) //SW
  {
    SW(regSource,regSource2,immediate);
  }
  else if(opcode == 0b100001) //LH - LOAD HALF WORD.
  {
    LH(regSource,regSource2,immediate);
  }
  else if(opcode == 0b100101) //LHU
  {
    LHU(regSource,regSource2,immediate);
  }
  else if(opcode == 0b000100) //BEQ
  {
    BEQ(regSource,regSource2,immediate);
  }
  else if(opcode == 0b000101) //BNE
  {
    BNE(regSource,regSource2,immediate);
  }
  else if(opcode == 0b000001 && regSource2 == 0b00000) //BLTZ
  {
    BLTZ(regSource,regSource2,immediate);
  }
  else if(opcode == 0b000001 && regSource2 == 0b00001) // BGEZ
  {
    BGEZ(regSource,regSource2,immediate);
  }

  else if(opcode == 0b000110 && regSource2 == 0b00000) // BLEZ
  {
    BLEZ(regSource,regSource2,immediate);
  }
  else if(opcode == 0b000111 && regSource2 == 0b00000) //BGTZ
  {
    BGTZ(regSource,regSource2,immediate);
  }
  else if(opcode == 0b000001 && regSource2 == 0b10001) //BGEZAL
  {
    BGEZAL(regSource,regSource2,immediate);
  }
  else if(opcode == 0b000001 && regSource2 == 0b10000)//BLTZAL
  {
    BLTZAL(regSource, regSource2, immediate);
  }
  else if(opcode == 0b001010) //SLTI
  {
    SLTI(regSource,regSource2,immediate);
  }
  else if(opcode == 0b001011) //SLTIU
  {
    SLTIU(regSource,regSource2,immediate);
  }
  else if(opcode == 0b100000) // LB
  {
    LB(regSource,regSource2,immediate);
  }
  else if(opcode == 0b100100) //LBU
  {
    LBU(regSource,regSource2,immediate);
  }
  else if(opcode == 0b100110) //LWR
  {
    LWR(regSource,regSource2,immediate);
  }
  else if(opcode == 0b100010) //LWL
  {
    LWL(regSource,regSource2,immediate);
  }
  else if(opcode == 0b101000) //SB
  {
    SB(regSource,regSource2,immediate);
  }
  else if(opcode == 0b101001) //SH
  {
    SH(regSource,regSource2,immediate);
  }

  else
  {
      exit(-12);
  }
}


///////////////////////// J type instructions being written here  /////////////////////////////////////////////////////////
void JAL(int opcode, int target)
{
  int32_t temp_address = target << 2;
  int32_t eff_address = temp_address | ((CPU_Registers.Program_Counter + 0x10000004) & 0xF0000000);

  if(eff_address % 4 == 0)
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter +4)/4)); // MIPS BRANCH DELAY...
      // getting and setting next pc address or ending simulation if address == 0x0
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      exit(-11);
  }

}
void jump(int opcode, int target) //PC = nPC; nPC = (PC & 0xf0000000) | (target << 2);
{
  int32_t temp_address = target << 2;
  int32_t eff_address = temp_address | ((CPU_Registers.Program_Counter + 0x10000004) & 0xF0000000);
  if(eff_address % 4 == 0){
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter +4)/4)); // MIPS BRANCH DELAY...

      // getting and setting next pc address or ending simulation if address == 0x0
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      exit(-11);
  }

}


///////////////////////// I type instructions being written here ///////////////////////////////////////////////////////////

void SB(int32_t regSource,int32_t regSource2,int16_t immediate) //MEM[$s + offset] = (0xff & $t); advance_pc (4);
{
    uint32_t temp_address = CPU_Registers.Get_Register(regSource2) & 0x000000FF;
    int32_t immediate_extended = immediate;

    int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
    uint32_t word;
    if((eff_address >= 0x30000004) && (eff_address < 0x30000008))
    {
        word = 0;
    }
    else{
      word = Mips_memory.memory_map_test(eff_address & 0xFFFFFFFC, 0, 0, true, false, false);
    }

    // check for byte
    if((eff_address & 0b11) == 0){
        temp_address = temp_address << 24;
        word = word & 0x00FFFFFF;
        word = word | temp_address;
        Mips_memory.memory_map_test(eff_address, word, 0b101000, false, true, false);
    }
    else if((eff_address & 0b11) == 1){
        temp_address = temp_address << 16;
        word = word & 0xFF00FFFF;
        word = word | temp_address;
        Mips_memory.memory_map_test(eff_address, word, 0b101000, false, true, false);
    }
    else if((eff_address & 0b11) == 2){
        temp_address = temp_address << 8;
        word = word & 0xFFFF00FF;
        word = word | temp_address;
        Mips_memory.memory_map_test(eff_address, word, 0b101000, false, true, false);
    }
    else{
        word = word & 0xFFFFFF00;
        word = word | temp_address;
        Mips_memory.memory_map_test(eff_address, word, 0b101000, false, true, false);
    }
    CPU_Registers.increment_Program_Counter();
}
void SH(int32_t regSource,int32_t regSource2,int16_t immediate) //MEM[$s + offset] = (0xff & $t); advance_pc (4);
{
    uint32_t temp_address = CPU_Registers.Get_Register(regSource2) & 0xFFFF;
    int32_t immediate_extended = immediate;

    int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
    uint32_t word;
    if((eff_address >= 0x30000004) && (eff_address < 0x30000008))
    {
        word = 0;
    }
    else{
      word = Mips_memory.memory_map_test(eff_address & 0xFFFFFFFC, 0, 0, true, false, false);
    }

    // check for byte
    if((eff_address & 0b11) == 0){
        temp_address = temp_address << 16;
        word = word & 0x0000FFFF;
        word = word | temp_address;
        Mips_memory.memory_map_test(eff_address, word, 0b101001, false, true, false);
    }
    else if((eff_address & 0b11) == 1){
        word = word & 0xFFFF0000;
        word = word | temp_address;
        Mips_memory.memory_map_test(eff_address, word, 0b101001, false, true, false);
    }
    else
    {
      exit(-11);
    }
    CPU_Registers.increment_Program_Counter();
}
void LWL(int32_t regSource,int32_t regSource2,int16_t immediate)
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
  uint32_t word = Mips_memory.memory_map_test(eff_address, 0, 0b100010, true, false, false);

  if((eff_address & 0b11) == 0){
      CPU_Registers.Set_Register(regSource2, word);
  }
  else if((eff_address & 0b11) == 1){
      word = (word << 8) | (CPU_Registers.Get_Register(regSource2) & 0x000000FF);
      CPU_Registers.Set_Register(regSource2, word);
  }
  else if((eff_address & 0b11) == 2){
      word = (word << 16) | (CPU_Registers.Get_Register(regSource2) & 0x0000FFFF);
      CPU_Registers.Set_Register(regSource2, word);
  }
  else{
      word = (word << 24) | (CPU_Registers.Get_Register(regSource2) & 0x00FFFFFF);
      CPU_Registers.Set_Register(regSource2, word);
  }

  CPU_Registers.increment_Program_Counter();
}
void LWR(int32_t regSource,int32_t regSource2,int16_t immediate)
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
  uint32_t word = Mips_memory.memory_map_test(eff_address, 0, 0b100110, true, false, false);

  if((eff_address & 0b11) == 0){
      word = ((word >> 24) & 0x000000FF) | (CPU_Registers.Get_Register(regSource2) & 0xFFFFFF00);
      CPU_Registers.Set_Register(regSource2, word);
  }
  else if((eff_address & 0b11) == 1){
      word = ((word >> 16) & 0x0000FFFF) | (CPU_Registers.Get_Register(regSource2) & 0xFFFF0000);
      CPU_Registers.Set_Register(regSource2, word);
  }
  else if((eff_address & 0b11) == 2){
      word = ((word >> 8) & 0x00FFFFFF) | (CPU_Registers.Get_Register(regSource2) & 0xFF000000);
      CPU_Registers.Set_Register(regSource2, word);
  }
  else{
      CPU_Registers.Set_Register(regSource2, word);
  }

  CPU_Registers.increment_Program_Counter();
}
void BGEZAL(int32_t regSource,int32_t regSource2,int16_t immediate) //if $s >= 0 $31 = PC + 8 (or nPC + 4); advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  CPU_Registers.Set_Register(31, CPU_Registers.Program_Counter+ 0x10000008);
  if((CPU_Registers.Get_Register(regSource) >> 31) == 0)
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }

}
void BLTZAL(int32_t regSource,int32_t regSource2,int16_t immediate) //if $s < 0 $31 = PC + 8 (or nPC + 4); advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  CPU_Registers.Set_Register(31, CPU_Registers.Program_Counter+ 0x10000008);
  if((CPU_Registers.Get_Register(regSource) < 0) && ((CPU_Registers.Get_Register(regSource) >> 28) == 1))
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }
}
void LB(int32_t regSource,int32_t regSource2,int16_t immediate) //$t = MEM[$s + offset]; advance_pc (4);
{
      int32_t immediate_extended = immediate;
      int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
      uint32_t word = Mips_memory.memory_map_test(eff_address, 0, 0b100000, true, false, false);
      //check the byte
      if((eff_address & 0b11) == 0)
      {
          int8_t byte = (word >> 24) & 0b00000000000000000000000011111111;
          int32_t sign_extended = byte;
          CPU_Registers.Set_Register(regSource2, sign_extended);
      }
      else if((eff_address & 0b11) == 1)
      {
          int8_t byte = (word >> 16) & 0b00000000000000000000000011111111;
          int32_t sign_extended = byte;
          CPU_Registers.Set_Register(regSource2, sign_extended);
      }
      else if((eff_address & 0b11) == 2)
      {
          int8_t byte = (word >> 8) & 0b00000000000000000000000011111111;
          int32_t sign_extended = byte;
          CPU_Registers.Set_Register(regSource2, sign_extended);
      }
      else
      {
          int8_t byte = word & 0b00000000000000000000000011111111;
          int32_t sign_extended = byte;
          CPU_Registers.Set_Register(regSource2, sign_extended);
      }
      CPU_Registers.increment_Program_Counter();
}
void LBU(uint32_t regSource,uint32_t regSource2,uint16_t immediate) //$t = MEM[$s + offset]; advance_pc (4);
{
      int32_t immediate_extended = immediate;
      int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
      uint32_t word = Mips_memory.memory_map_test(eff_address, 0, 0b100100, true, false, false);

      if((eff_address & 0b11) == 0)
      {
        int8_t byte = (word >> 24) & 0b00000000000000000000000011111111;
        int32_t sign_extended = byte;
        CPU_Registers.Set_Register(regSource2, sign_extended);
      }
      else if((eff_address & 0b11) == 1)
      {
          int8_t byte = (word >> 16) & 0b00000000000000000000000011111111;
          int32_t sign_extended = byte;
          CPU_Registers.Set_Register(regSource2, sign_extended);
      }
      else if((eff_address & 0b11) == 2)
      {
          int8_t byte = (word >> 8) & 0b00000000000000000000000011111111;
          int32_t sign_extended = byte;
          CPU_Registers.Set_Register(regSource2, sign_extended);
      }
      else
      {
          int8_t byte = word & 0b00000000000000000000000011111111;
          int32_t sign_extended = byte;
          CPU_Registers.Set_Register(regSource2, sign_extended);
      }

      CPU_Registers.increment_Program_Counter();
}
void SLTIU(uint32_t regSource,uint32_t regSource2,uint16_t immediate) //if $s < imm $t = 1; advance_pc (4); else $t = 0; advance_pc (4);
{
  uint32_t unsigned_immediate_as_32 = immediate;
  if(CPU_Registers.Get_Register(regSource) < unsigned_immediate_as_32)
  {
    CPU_Registers.Set_Register(regSource2, 1);
  }
  else
  {
    CPU_Registers.Set_Register(regSource2, 0);
  }

  CPU_Registers.increment_Program_Counter();
}
void SLTI(int32_t regSource,int32_t regSource2,int16_t immediate) //if $s < imm $t = 1; advance_pc (4); else $t = 0; advance_pc (4);
{
  int32_t sign_extended = CPU_Registers.Get_Register(regSource);
  int32_t signed_immediate = immediate;
  if(sign_extended < signed_immediate)
  {
    CPU_Registers.Set_Register(regSource2, 1);
  }
  else{
    CPU_Registers.Set_Register(regSource2, 0);
  }

  CPU_Registers.increment_Program_Counter();
}
void BGTZ(int32_t regSource,int32_t regSource2,int16_t immediate) //will be opposite of bltz .... if $s > 0 advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  if((CPU_Registers.Get_Register(regSource) != 0) && ((CPU_Registers.Get_Register(regSource) >> 31) == 0))
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }
}
void BGEZ(int32_t regSource,int32_t regSource2,int16_t immediate)   //gonna be similar to blez... if $s <= 0 advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  if(((CPU_Registers.Get_Register(regSource) >> 31) == 0))
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));

      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }
}
void BLEZ(int32_t regSource,int32_t regSource2,int16_t immediate)   //gonna be similar to blez... if $s <= 0 advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  if((CPU_Registers.Get_Register(regSource) == 0) || ((CPU_Registers.Get_Register(regSource) >> 31) == 1))
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }
}
void BLTZ(int32_t regSource,int32_t regSource2,int16_t immediate)   //if $s < 0 advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  if(((CPU_Registers.Get_Register(regSource) >> 31) == 1))
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }
}
void BNE(int32_t regSource,int32_t regSource2,int16_t immediate)     //if $s != $t advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  if(CPU_Registers.Get_Register(regSource) != CPU_Registers.Get_Register(regSource2))
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }
}
void BEQ(int32_t regSource,int32_t regSource2,int16_t immediate)     //if $s == $t advance_pc (offset << 2)); else advance_pc (4);
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = (immediate_extended << 2) + CPU_Registers.Program_Counter + 0x10000004; //according to memory map.
  if(CPU_Registers.Get_Register(regSource) == CPU_Registers.Get_Register(regSource2))
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(eff_address, 0, 0, false, false, true);
  }
  else
  {
      CPU_Registers.increment_Program_Counter();
  }
}
void LHU(uint32_t regSource,uint32_t regSource2,uint16_t immediate)
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
  uint32_t word = Mips_memory.memory_map_test(eff_address, 0, 0b100101, true, false, false);

  if((eff_address & 0b11) == 0){
      int16_t memory_hword = (word >> 16) & 0b00000000000000001111111111111111;
      int32_t sign_extended = memory_hword;
      CPU_Registers.Set_Register(regSource2, sign_extended);
  }
  else if((eff_address & 0b11) == 2){
      int16_t memory_hword = word & 0b00000000000000001111111111111111;
      int32_t sign_extended = memory_hword;
      CPU_Registers.Set_Register(regSource2, sign_extended);
  }
  else{
      exit(-11);
  }
  CPU_Registers.increment_Program_Counter();
}
void LH(int32_t regSource,int32_t regSource2,int16_t immediate)
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
  uint32_t word = Mips_memory.memory_map_test(eff_address, 0, 0b100001, true, false, false);

  if((eff_address & 0b11) == 0)
  {
      int16_t memory_hword = (word >> 16) & 0b00000000000000001111111111111111;
      int32_t sign_extended = memory_hword;
      CPU_Registers.Set_Register(regSource2, sign_extended);
  }
  else if((eff_address & 0b11) == 2)
  {
      int16_t memory_hword = word & 0b00000000000000001111111111111111;
      int32_t sign_extended = memory_hword;
      CPU_Registers.Set_Register(regSource2, sign_extended);
  }
  else{
      exit(-11);
  }
  CPU_Registers.increment_Program_Counter();
}
void SW(int32_t regSource,int32_t regSource2,int16_t immediate)
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
  if((eff_address & 0x00000000000000000000000000000011) == 0)
  {
    uint32_t value = Mips_memory.memory_map_test(eff_address, CPU_Registers.Get_Register(regSource2), 0b101011, false, true, false);
  }
  else
  {
    exit(-11);
  }
  CPU_Registers.increment_Program_Counter();
}
void LW(int32_t regSource,int32_t regSource2,int16_t immediate)
{
  int32_t immediate_extended = immediate;
  int32_t eff_address = immediate_extended + CPU_Registers.Get_Register(regSource);
  // check least 2 significant bitties
  if((eff_address & 0x00000000000000000000000000000011) == 0)
  {
    uint32_t value = Mips_memory.memory_map_test(eff_address, 0, 0b100011, true, false, false);
    CPU_Registers.Set_Register(regSource2, value);
  }
  else
  {
    exit(-11);
  }
  CPU_Registers.increment_Program_Counter();
}
void LUI(uint32_t regSource2,uint16_t immediate)
{
  CPU_Registers.Set_Register(regSource2, immediate << 16);
  CPU_Registers.increment_Program_Counter();
  // CPU_Registers.Get_Register(regSource2);
  //std::cerr << "lui has been done " << std::endl;

}
void ORI(uint32_t regSource, uint32_t regSource2, int16_t immediate)
{
  int32_t immediate_extended = immediate & 0b00000000000000001111111111111111;
  CPU_Registers.Set_Register(regSource2, CPU_Registers.Get_Register(regSource) | immediate_extended);
  CPU_Registers.increment_Program_Counter();
  //std::cerr << "ori has been done " << std::endl;

}
void ANDI(uint32_t regSource, uint32_t regSource2, int16_t immediate)
{
  int32_t immediate_extended = immediate & 0b00000000000000001111111111111111;
  CPU_Registers.Set_Register(regSource2, CPU_Registers.Get_Register(regSource) & immediate_extended);

  CPU_Registers.increment_Program_Counter();
}
void ADDIU(uint32_t regSource, uint32_t regSource2, uint16_t immediate)
{
  // extend 16 bit immediate into 32 bit immediate
  int32_t immediate_extended = immediate;
  int sum = CPU_Registers.Get_Register(regSource) + immediate_extended;
  CPU_Registers.Set_Register(regSource2, sum);
  CPU_Registers.increment_Program_Counter();
}
void ADDI(int32_t regSource,int32_t regSource2,int16_t immediate)
{
  // extend 16 bits of immediate into 32 bits immediate
  int32_t immediate_extended = immediate;

  int result = CPU_Registers.Get_Register(regSource) + immediate_extended;

  if(((CPU_Registers.Get_Register(regSource) >= 0) && (immediate_extended>= 0) && ( result < 0)) || ((CPU_Registers.Get_Register(regSource) < 0) && (immediate_extended < 0) && ( result >= 0)))
  {
    exit(-10);
  }
  else{
    CPU_Registers.Set_Register(regSource2, result);
    //std::cerr << "value in" << regSource2 << "is: " << CPU_Registers.Get_Register(regSource2) << '\n';
    CPU_Registers.increment_Program_Counter();

  }
}
void XORI(int regSource,int regSource2,int16_t immediate)
{
  // we must extend the 16 bit immediate to 32 bit immediate for the OR operation...
   int32_t immediate_extended = immediate & 0b00000000000000001111111111111111;
   CPU_Registers.Set_Register(regSource2, CPU_Registers.Get_Register(regSource) ^ immediate_extended);
   CPU_Registers.increment_Program_Counter();

}
// -------------------------------- ALL OUR INSTRUCTIONS WILL BE DEFINED IN THIS BLOCK --------------------------------------- //

// R type instructions
void ADD(uint32_t regSource1, uint32_t regSource2, uint32_t regDestination)
{
  uint32_t result = CPU_Registers.Get_Register(regSource1) + CPU_Registers.Get_Register(regSource2);

  if(((CPU_Registers.Get_Register(regSource1) >= 0) && (CPU_Registers.Get_Register(regSource2) >= 0) && (result < 0)) || ((CPU_Registers.Get_Register(regSource1) < 0) && (CPU_Registers.Get_Register(regSource2) < 0) && (result >= 0)))
  {
    exit(-10);
  }
  else
  {
    //std::cerr << "add has been done " << std::endl;
    CPU_Registers.Set_Register(regDestination,result);
    CPU_Registers.increment_Program_Counter();
  }
}
void ADDU(uint32_t regSource1, uint32_t regSource2, uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination,CPU_Registers.Get_Register(regSource1) + CPU_Registers.Get_Register(regSource2));
  CPU_Registers.increment_Program_Counter();
}
void AND(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource) & CPU_Registers.Get_Register(regSource2));
  CPU_Registers.increment_Program_Counter();
}
void JR(uint32_t regSource)
{
  int32_t temp_address = CPU_Registers.Get_Register(regSource);
  if(temp_address % 4 == 0) // TO ENSURE ITS A VALID INSTRUCTION FOR PC...
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));  // MIPS BRANCH DELAY

      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(temp_address,0,0,false,false,true);
      //std::cerr << "JR has been done " << std::endl;
  }
  else
  {
      exit(-11);
  }
}
void OR(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource) | CPU_Registers.Get_Register(regSource2));

  CPU_Registers.increment_Program_Counter();
}
void SLTU(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  if(CPU_Registers.Get_Register(regSource) < CPU_Registers.Get_Register(regSource2))
  {
      CPU_Registers.Set_Register(regDestination, 1);
  }
  else
  {
      CPU_Registers.Set_Register(regDestination, 0);
  }
  CPU_Registers.increment_Program_Counter();
}
void SUBU(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource) - CPU_Registers.Get_Register(regSource2));
  CPU_Registers.increment_Program_Counter();
}
void XOR(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource) ^ CPU_Registers.Get_Register(regSource2));
  CPU_Registers.increment_Program_Counter();
}
void SRAV(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  int32_t signed_rt  = CPU_Registers.Get_Register(regSource2);
  CPU_Registers.Set_Register(regDestination, signed_rt >> CPU_Registers.Get_Register(regSource));
  CPU_Registers.increment_Program_Counter();
}
void SLL(uint32_t regSource2, uint32_t regDestination,uint32_t shamt)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource2) << shamt);
  CPU_Registers.increment_Program_Counter();
}
void SLT(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  int32_t regSource_signed = CPU_Registers.Get_Register(regSource);
  int32_t regSource2_signed = CPU_Registers.Get_Register(regSource2);
  if(regSource2_signed < regSource_signed)
  {
      CPU_Registers.Set_Register(regDestination, 1);
  }
  else
  {
      CPU_Registers.Set_Register(regDestination, 0);
  }
  CPU_Registers.increment_Program_Counter();
}
void SRA(uint32_t regSource2, uint32_t regDestination,uint32_t shamt)
{
  int32_t signed_regSource2  = CPU_Registers.Get_Register(regSource2);
  CPU_Registers.Set_Register(regDestination, signed_regSource2 >> shamt);
  CPU_Registers.increment_Program_Counter();
}
void SRL(uint32_t regSource2, uint32_t regDestination,uint32_t shamt)
{
    CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource2) >> shamt);
    CPU_Registers.increment_Program_Counter();
}
void SUB(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  int subAmount = CPU_Registers.Get_Register(regSource) - CPU_Registers.Get_Register(regSource2);
  if(((CPU_Registers.Get_Register(regSource) >= 0) && (CPU_Registers.Get_Register(regSource2) <0) && (subAmount < 0)) || ((CPU_Registers.Get_Register(regSource) < 0) && (CPU_Registers.Get_Register(regSource2) >= 0) && (subAmount >= 0)))
  {
      exit(-10);
  }
  else
  {
    CPU_Registers.Set_Register(regDestination, subAmount);
    CPU_Registers.increment_Program_Counter();
  }
}
void SLLV(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource2) << CPU_Registers.Get_Register(regSource));
  CPU_Registers.increment_Program_Counter();
}
void SRLV(uint32_t regSource,uint32_t regSource2,uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.Get_Register(regSource2) >> CPU_Registers.Get_Register(regSource));
  CPU_Registers.increment_Program_Counter();
}
void MFHI(uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.hi);
  CPU_Registers.increment_Program_Counter();
}
void MFLO(uint32_t regDestination)
{
  CPU_Registers.Set_Register(regDestination, CPU_Registers.lo);
  CPU_Registers.increment_Program_Counter();
}
void MTHI(uint32_t regSource)
{
  CPU_Registers.hi = (CPU_Registers.Get_Register(regSource));
  CPU_Registers.increment_Program_Counter();
}
void MTLO(uint32_t regSource)
{
  CPU_Registers.lo = (CPU_Registers.Get_Register(regSource));
  CPU_Registers.increment_Program_Counter();
}
void JALR(uint32_t regSource,uint32_t regDestination)
{

  int32_t temp_address = CPU_Registers.Get_Register(regSource);

  CPU_Registers.Set_Register(regDestination, CPU_Registers.Program_Counter + 0x10000008);
  if(temp_address % 4 == 0)
  {
      determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR((CPU_Registers.Program_Counter+4)/4));
      CPU_Registers.Program_Counter = Mips_memory.memory_map_test(temp_address, 0, 0, true, false, false);
  }
  else
  {
      exit(-11);
  }

}
void DIV(int32_t regSource,int32_t regSource2)
{ //we need to calculate both quotient and remainder and appropiately store in low and high reg..
  int32_t temp_address_rs1 = CPU_Registers.Get_Register(regSource);
  int32_t temp_address_rs2 = CPU_Registers.Get_Register(regSource2);

  if(temp_address_rs2 == 0)
  {
    exit(-10);
  }
  else if(temp_address_rs2 != 0) //zero checc
  {
      int32_t quotient = temp_address_rs1 / temp_address_rs2;
      CPU_Registers.lo = quotient;
      int32_t remainderr = temp_address_rs1 % temp_address_rs2;
      CPU_Registers.hi = remainderr;
  }
  CPU_Registers.increment_Program_Counter();
}
void DIVU(uint32_t regSource,uint32_t regSource2)
{
  if(CPU_Registers.Get_Register(regSource2) == 0)
  {
    exit(-10);
  }
  else if(CPU_Registers.Get_Register(regSource2) != 0)
  {
      CPU_Registers.lo = (CPU_Registers.Get_Register(regSource) / CPU_Registers.Get_Register(regSource2));
      CPU_Registers.hi = (CPU_Registers.Get_Register(regSource) % CPU_Registers.Get_Register(regSource2));
  }
  CPU_Registers.increment_Program_Counter();
}
void MULT(int32_t regSource,int32_t regSource2)
{
  int64_t result; //result of mult of 32x32 is 64bit.
  int64_t temp_address_rs1 = CPU_Registers.Get_Register(regSource);
  int64_t temp_address_rs2 = CPU_Registers.Get_Register(regSource2);
  temp_address_rs1 = temp_address_rs1 << 32;
  temp_address_rs1 = temp_address_rs1 >> 32;
  temp_address_rs2 = temp_address_rs2 << 32;
  temp_address_rs2 = temp_address_rs2 >> 32;
  result = temp_address_rs1 * temp_address_rs2;
  //split the result into 32 chunks and store appropiately in lo and high registers...
  CPU_Registers.lo = (result & 0x00000000FFFFFFFF);
  CPU_Registers.hi = ((result >> 32) & 0x00000000FFFFFFFF);

  CPU_Registers.increment_Program_Counter();
}
void MULTU(uint32_t regSource,uint32_t regSource2)
{
  //similar to the above mult function...
  uint64_t result;
  uint32_t temp_address_rs1 = CPU_Registers.Get_Register(regSource);
  uint32_t temp_address_rs2 = CPU_Registers.Get_Register(regSource2);
  result = temp_address_rs1 * temp_address_rs2;
  CPU_Registers.lo = (result & 0x00000000FFFFFFFF);
  CPU_Registers.hi = ((result >> 32) & 0x00000000FFFFFFFF);

  CPU_Registers.increment_Program_Counter();

}



int main(int argc, char* argv[]){
    //load binary into ADDR_INSTR
    Mips_memory.ADDR_INSTR.resize(16777216);
    Mips_memory.ADDR_DATA.resize(4194304);

    // intialised everything.
    CPU_Registers.hi = 0;
    CPU_Registers.lo = 0;
    CPU_Registers.Program_Counter = 0; //initialised to 0
    CPU_Registers.register_vector.resize(32);
    CPU_Registers.register_vector[0]  = 0; //Registers 0 initialised to 0.

    int word;
    std::ifstream file;
    file.open(argv[1], std::ios::binary | std::ios::in);
    int number_of_instructions = 0;
  //  std::cerr << "hello";

    while(true){
        file.seekg(0, std::ios::cur);
        file.read(reinterpret_cast<char*>(&word), 4);
        if(file.eof()) break;
        if(number_of_instructions >= 4194304) break;
        Mips_memory.load_ADDR_INSTR(number_of_instructions, ntohl(word));
        number_of_instructions++;
        //std::cerr<<"Binary read in "<<std::bitset<32>() << std::endl;

    }
    file.close();


    while(true){
        if(CPU_Registers.Program_Counter >= 0x1000000/4){
            exit(-11); //memory exception
        }
        //std::cerr << "The value in PC is before: " << CPU_Registers.Program_Counter << std::endl;

        determine_instruction_to_execute(Mips_memory.read_ADDR_INSTR(CPU_Registers.Program_Counter/4));
        //std::cerr << "The value in PC is after : " << CPU_Registers.Program_Counter << std::endl;

      if(CPU_Registers.Program_Counter == 0)
      {
        uint8_t exit_code = CPU_Registers.register_vector[2] & 0b00000000000000000000000011111111;
        exit(exit_code);
      }

    }
    return 0;
}
