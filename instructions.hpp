#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP


//R instruction
void ADD(uint32_t RegSource1, uint32_t RegSource2, uint32_t RegDestination);
void ADDU(uint32_t RegSource1, uint32_t RegSource2, uint32_t RegDestination);
void AND(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void AND(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void JR(uint32_t regSource);
void OR(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void SLTU(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void SUBU(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void XOR(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void SRAV(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void SLL(uint32_t regSource2, uint32_t regDestination,uint32_t shamt);
void SLT(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void SRA(uint32_t regSource2, uint32_t regDestination,uint32_t shamt);
void SRL(uint32_t regSource2, uint32_t regDestination,uint32_t shamt);
void SUB(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void SLLV(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void SRLV(uint32_t regSource,uint32_t regSource2,uint32_t regDestination);
void MFHI(uint32_t regDestination);
void MFLO(uint32_t regDestination);
void MTHI(uint32_t regSource);
void MTLO(uint32_t regSource);
void JALR(uint32_t regSource,uint32_t regDestination);
void DIV(int32_t regSource,int32_t regSource2);
void DIVU(uint32_t regSource,uint32_t regSource2);
void MULT(int32_t regSource,int32_t regSource2);
void MULTU(uint32_t regSource,uint32_t regSource2);

// I type declarations
void XORI(int regSource,int regSource2, int16_t immediate);
void ADDI(int32_t regSource, int32_t regSource2, int16_t immediate);
void ADDIU(uint32_t regSource, uint32_t regSource2, uint16_t immediate);
void ANDI(uint32_t regSource, uint32_t regSource2, int16_t immediate);
void ORI(uint32_t regSource, uint32_t regSource2, int16_t immediate);
void LUI(uint32_t regSource2,uint16_t immediate);
void LW(int32_t regSource,int32_t regSource2,int16_t immediate);
void SW(int32_t regSource,int32_t regSource2,int16_t immediate);
void LH(int32_t regSource,int32_t regSource2,int16_t immediate);
void LHU(uint32_t regSource,uint32_t regSource2,uint16_t immediate);
void BEQ(int32_t regSource,int32_t regSource2,int16_t immediate);
void BNE(int32_t regSource,int32_t regSource2,int16_t immediate);
void BLTZ(int32_t regSource,int32_t regSource2,int16_t immediate);
void BLEZ(int32_t regSource,int32_t regSource2,int16_t immediate);
void BGTZ(int32_t regSource,int32_t regSource2,int16_t immediate);
void SLTI(int32_t regSource,int32_t regSource2,int16_t immediate);
void SLTIU(uint32_t regSource,uint32_t regSource2,uint16_t immediate);
void LB(int32_t regSource,int32_t regSource2,int16_t immediate);
void LBU(uint32_t regSource,uint32_t regSource2,uint16_t immediate);
void BGEZ(int32_t regSource,int32_t regSource2,int16_t immediate);
void BGEZAL(int32_t regSource,int32_t regSource2,int16_t immediate);
void BLTZAL(int32_t regSource,int32_t regSource2,int16_t immediate);
void LWL(int32_t regSource,int32_t regSource2,int16_t immediate);
void LWR(int32_t regSource,int32_t regSource2,int16_t immediate);
void SB(int32_t regSource,int32_t regSource2,int16_t immediate);
void SH(int32_t regSource,int32_t regSource2,int16_t immediate);

// J type declarations
void jump(int opcode, int target);
void JAL(int opcode, int target);


#endif
