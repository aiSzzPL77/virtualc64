// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CPU_H
#define _CPU_H

#include "C64Component.h"
#include "CPUDebugger.h"
#include "CPUInstructions.h"
#include "ProcessorPort.h"

class Memory;

class CPU : public C64Component {
        
    friend class CPUDebugger;
    friend class Breakpoints;
    friend class Watchpoints;

    // Reference to the connected memory (MOVE TO SUBCLASS)
    Memory &mem;
    
    // Result of the latest inspection (MOVE TO C64CPU SUBCLASS ?!)
    CPUInfo info;
    
    
    //
    // Sub components
    //
    
public:
    
    // Processor Port
    ProcessorPort pport = ProcessorPort(c64);
    
    // CPU debugger
    CPUDebugger debugger = CPUDebugger(c64);
    
private:
    
    //
    // Chip properties
    //
    
    /*! @brief    Selected model (DEPRECATED, USE CPUConfig)
     *  @details  Right now, this atrribute is only used to distinguish the
     *            C64 CPU (MOS6510) from the VC1541 CPU (MOS6502). Hardware
     *            differences between both models are not emulated.
     */
    CPUModel model;
    
    
    //
    // Lookup tables
    //
    
    /* Mapping from opcodes to microinstructions. This array stores the tags
     * of the second microcycle which is microcycle cycle following the fetch
     * phase.
     */
    MicroInstruction actionFunc[256];
                
    
    //
    // Internal state
    //
    
    /* Flags
     *
     * CPU_LOG_INSTRUCTION:
     *     This flag is set if instruction logging is enabled. If set, the
     *     CPU records the current register contents in a log buffer.
     *
     * CPU_CHECK_BP:
     *    This flag indicates whether the CPU should check for breakpoints.
     *
     * CPU_CHECK_WP:
     *    This flag indicates whether the CPU should check fo watchpoints.
     */
    int flags;
    static const int CPU_LOG_INSTRUCTION   = (1 << 0);
    static const int CPU_CHECK_BP          = (1 << 1);
    static const int CPU_CHECK_WP          = (1 << 2);
    
public:
    
    // Elapsed clock cycles since power up
    u64 cycle;
    
    // Indicates whether the CPU is jammed
    bool halted;
                
private:

    // The next microinstruction to be executed
    MicroInstruction next;
        
    
    //
    // Registers
    //
    
private:
    
    Registers reg;
    
public:
    
    u8 regA;
    u8 regX;
    u8 regY;
    u16 regPC;
    u8 regSP;
    
private:
    
    /*! @brief     Processor status register (flags)
     *  @details   7 6 5 4 3 2 1 0
     *             N O - B D I Z C
     */
    u8 regP;
    
    //! @brief    Address data (low byte)
    u8 regADL;
    
    //! @brief    Address data (high byte)
    u8 regADH;
    
    //! @brief    Input data latch (indirect addressing modes)
    u8 regIDL;
    
    //! @brief    Data buffer
    u8 regD;
    
    /*! @brief    Address overflow indicater
     *  @details  Indicates when the page boundary has been crossed.
     */
    bool overflow;
    
    /* Frozen program counter.
     * This variable matches the value of the program counter when the CPU
     * starts to execute an instruction. In contrast to the real program
     * counter, the value isn't changed until the CPU starts to process the
     * next instruction. In other words: This value always contains the start
     * address of the currently executed command, even if some microcycles of
     * the command have already been computed.
     */
    // TODO: Rename to pc0
    u16 pc;
    
    
    //
    // Port lines
    //
    
public:
    
    /* Ready line (RDY)
     * If this line is low, the CPU freezes on the next read access. RDY is
     * pulled down by VICII to perform longer lasting read operations.
     */
    bool rdyLine;
    
private:
    
    // Cycle of the most recent rising edge of the RDY line
    u64 rdyLineUp;
    
    // Cycle of the most recent falling edge of the RDY line
    u64 rdyLineDown;
    
public:
    
    /* Interrupt lines.
     * Usally both variables equal 0 which means that the two interrupt lines
     * are high. When an external component requests an interrupt, the NMI or
     * the IRQ line is pulled low. In that case, the corresponding variable is
     * set to a positive value which indicates the interrupt source. The
     * variables are used in form of bit fields since both interrupt lines are
     * driven by multiple sources.
     */
    u8 nmiLine;
    u8 irqLine;
    
private:
    
    /* Edge detector (NMI line)
     * https://wiki.nesdev.com/w/index.php/CPU_interrupts
     * "The NMI input is connected to an edge detector. This edge detector polls
     *  the status of the NMI line during φ2 of each CPU cycle (i.e., during the
     *  second half of each cycle) and raises an internal signal if the input
     *  goes from being high during one cycle to being low during the next. The
     *  internal signal goes high during φ1 of the cycle that follows the one
     *  where the edge is detected, and stays high until the NMI has been
     *  handled."
     */
    TimeDelayed<u8> edgeDetector = TimeDelayed<u8>(1, &cycle);
    
    /* Level detector of IRQ line.
     * https://wiki.nesdev.com/w/index.php/CPU_interrupts
     * "The IRQ input is connected to a level detector. If a low level is
     *  detected on the IRQ input during φ2 of a cycle, an internal signal is
     *  raised during φ1 the following cycle, remaining high for that cycle only
     *  (or put another way, remaining high as long as the IRQ input is low
     *  during the preceding cycle's φ2).
     */
    TimeDelayed<u8> levelDetector = TimeDelayed<u8>(1, &cycle);
    
    /* Result of the edge detector polling operation.
     * https://wiki.nesdev.com/w/index.php/CPU_interrupts
     * "The output from the edge detector and level detector are polled at
     *  certain points to detect pending interrupts. For most instructions, this
     *  polling happens during the final cycle of the instruction, before the
     *  opcode fetch for the next instruction. If the polling operation detects
     *  that an interrupt has been asserted, the next "instruction" executed
     *  is the interrupt sequence. Many references will claim that interrupts
     *  are polled during the last cycle of an instruction, but this is true
     *  only when talking about the output from the edge and level detectors."
     */
    bool doNmi;
    
    /* Result of the level detector polling operation.
     * https://wiki.nesdev.com/w/index.php/CPU_interrupts
     * "If both an NMI and an IRQ are pending at the end of an instruction, the
     *  NMI will be handled and the pending status of the IRQ forgotten (though
     *  it's likely to be detected again during later polling)."
     */
    bool doIrq;
    
    
    //
    // Constructing and serializing
    //
    
public:
    
    CPU(CPUModel model, C64& ref, Memory &memref);
    
private:
        
    // Registers the instruction set
    void registerInstructions();
    void registerLegalInstructions();
    void registerIllegalInstructions();
    
    // Registers a single instruction
    void registerCallback(u8 opcode,
                          const char *mnemonic,
                          AddressingMode mode,
                          MicroInstruction mInstr);
    
    //
    // Configuring
    //
    
    // Returns true if this is the C64's CPU
    bool isC64CPU() { return model == MOS_6510; }
    
    
    //
    // Analyzing
    //
    
public:
    
    // Returns the result of the latest inspection
    CPUInfo getInfo() { return HardwareComponent::getInfo(info); }
    
    DisassembledInstruction getInstrInfo(long nr, u16 startAddr);
    DisassembledInstruction getInstrInfo(long nr);
    DisassembledInstruction getLoggedInstrInfo(long nr);
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _reset() override;
    void _inspect() override;
    void _inspect(u32 dasmStart);
    void _dump() override;
    void _setDebug(bool enable) override;
    size_t stateSize() override;
    void didLoadFromBuffer(u8 **buffer) override;
    void didSaveToBuffer(u8 **buffer) override;

    
    //
    // Getter and setter
    //

public:
    
    
    u16 getPC() { return pc; }
    void jumpToAddress(u16 addr) { pc = regPC = addr; next = fetch; }
    void setPCL(u8 lo) { regPC = (regPC & 0xff00) | lo; }
    void setPCH(u8 hi) { regPC = (regPC & 0x00ff) | ((u16)hi << 8); }
    void incPC(u8 offset = 1) { regPC += offset; }
    void incPCL(u8 offset = 1) { setPCL(LO_BYTE(regPC) + offset); }
    void incPCH(u8 offset = 1) { setPCH(HI_BYTE(regPC) + offset); }

    u8 getN() { return regP & N_FLAG; }
    void setN(u8 bit) { bit ? regP |= N_FLAG : regP &= ~N_FLAG; }
    
    u8 getV() { return regP & V_FLAG; }
    void setV(u8 bit) { bit ? regP |= V_FLAG : regP &= ~V_FLAG; }
    
    u8 getB() { return regP & B_FLAG; }
    void setB(u8 bit) { bit ? regP |= B_FLAG : regP &= ~B_FLAG; }
    
    u8 getD() { return regP & D_FLAG; }
    void setD(u8 bit) { bit ? regP |= D_FLAG : regP &= ~D_FLAG; }
    
    u8 getI() { return regP & I_FLAG; }
    void setI(u8 bit) { bit ? regP |= I_FLAG : regP &= ~I_FLAG; }
    
    u8 getZ() { return regP & Z_FLAG; }
    void setZ(u8 bit) { bit ? regP |= Z_FLAG : regP &= ~Z_FLAG; }
    
    u8 getC() { return regP & C_FLAG; }
    void setC(u8 bit) { bit ? regP |= C_FLAG : regP &= ~C_FLAG; }
        
    u8 getP() { return regP | 0b00100000; }
    u8 getPWithClearedB() { return getP() & 0b11101111; }
    void setP(u8 p) { regP = p; }
    void setPWithoutB(u8 p) { regP = (p & 0b11101111) | (regP & 0b00010000); }
    
private:
    
    // Loads the accumulator. The Z- and N-flag may change.
    void loadA(u8 a) { regA = a; setN(a & 0x80); setZ(a == 0); }
    
    // Loads the X register. The Z- and N-flag may change.
    void loadX(u8 x) { regX = x; setN(x & 0x80); setZ(x == 0); }
    
    // Loads the Y register. The Z- and N-flag may change.
    void loadY(u8 y) { regY = y; setN(y & 0x80); setZ(y == 0); }
    
    
    //
    // Operating the ALU
    //
    
    // Performs an arithmetic operation
    void adc(u8 op);
    void adc_binary(u8 op);
    void adc_bcd(u8 op);
    void sbc(u8 op);
    void sbc_binary(u8 op);
    void sbc_bcd(u8 op);

    // Performs a logical operation
    void cmp(u8 op1, u8 op2);
    u8 ror(u8 op);
    u8 rol(u8 op);

    // Emulates a banching instruction
    // void branch(i8 offset);
    
    
    //
    // Handling interrupts
    //
    
public:
    
    // Pulls down or releases an interrupt line
    void pullDownNmiLine(IntSource source);
    void releaseNmiLine(IntSource source);
    void pullDownIrqLine(IntSource source);
    void releaseIrqLine(IntSource source);
    
    // Sets the RDY line
    void setRDY(bool value);
    
        
    //
    // Executing the device
    //
    
public:

    // Returns true if the CPU is jammed
    bool isHalted() { return halted; }
    
    // Returns true if the next cycle marks the beginning of an instruction
    bool inFetchPhase() { return next == fetch; }

    // Executes the next micro instruction
    void executeOneCycle();

private:

    // Processes debug flags
    void processFlags();
};

#endif
