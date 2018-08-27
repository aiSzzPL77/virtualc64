/*!
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   Dirk W. Hoffmann, 2018
 */
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "C64.h"

uint8_t
VIC::peek(uint16_t addr)
{
    uint8_t result;
    
    assert(addr <= 0x3F);
    
    switch(addr) {
        case 0x00: // Sprite X (lower 8 bits)
        case 0x02:
        case 0x04:
        case 0x06:
        case 0x08:
        case 0x0A:
        case 0x0C:
        case 0x0E:
            
            return reg.current.sprX[addr >> 1];
            
        case 0x01: // Sprite Y
        case 0x03:
        case 0x05:
        case 0x07:
        case 0x09:
        case 0x0B:
        case 0x0D:
        case 0x0F:
            
            return reg.current.sprY[addr >> 1];
            
        case 0x10: // Sprite X (upper bits)
        {
            return
            ((reg.current.sprX[0] & 0x100) ? 0b00000001 : 0) |
            ((reg.current.sprX[1] & 0x100) ? 0b00000010 : 0) |
            ((reg.current.sprX[2] & 0x100) ? 0b00000100 : 0) |
            ((reg.current.sprX[3] & 0x100) ? 0b00001000 : 0) |
            ((reg.current.sprX[4] & 0x100) ? 0b00010000 : 0) |
            ((reg.current.sprX[5] & 0x100) ? 0b00100000 : 0) |
            ((reg.current.sprX[6] & 0x100) ? 0b01000000 : 0) |
            ((reg.current.sprX[7] & 0x100) ? 0b10000000 : 0);
        }
                   
        case 0x11: // SCREEN CONTROL REGISTER #1
            return (reg.current.ctrl1 & 0x7f) | (yCounter > 0xFF ? 0x80 : 0);
            
        case 0x12: // VIC_RASTER_READ_WRITE
            return yCounter & 0xff;
            
        case 0x13: // LIGHTPEN X
            return latchedLightPenX;
            
        case 0x14: // LIGHTPEN Y
            return latchedLightPenY;
            
        case 0x15:
            return reg.current.sprEnable;
            
        case 0x16:
            // The two upper bits always read back as '1'
            return (reg.current.ctrl2 & 0xFF) | 0xC0;
            
        case 0x17:
            return reg.current.sprExpandY;
            
        case 0x18:
            return memSelect | 0x01; // Bit 1 is unused (always 1)
            
        case 0x19: // Interrupt Request Register (IRR)
            return (irr & imr) ? (irr | 0xF0) : (irr | 0x70);
            
        case 0x1A: // Interrupt Mask Register (IMR)
            return imr | 0xF0;
            
        case 0x1B:
            return reg.current.sprPriority;
            
        case 0x1C:
            return reg.current.sprMC;
            
        case 0x1D: // SPRITE_X_EXPAND
            return reg.current.sprExpandX;
            
        case 0x1E: // Sprite-to-sprite collision
            result = spriteSpriteCollision;
            spriteSpriteCollision = 0; // Clear on read
            return result;
            
        case 0x1F: // Sprite-to-background collision
            result = spriteBackgroundColllision;
            spriteBackgroundColllision = 0; // Clear on read
            return result;
            
        case 0x20: // Border color
            return reg.current.colors[COLREG_BORDER] | 0xF0;
            
        case 0x21: // Background color 0
            return reg.current.colors[COLREG_BG0] | 0xF0;
            
        case 0x22: // Background color 1
            return reg.current.colors[COLREG_BG1] | 0xF0;

        case 0x23: // Background color 2
            return reg.current.colors[COLREG_BG2] | 0xF0;

        case 0x24: // Background color 3
            return reg.current.colors[COLREG_BG3] | 0xF0;
            
        case 0x25: // Sprite extra color 1 (for multicolor sprites)
            return reg.current.colors[COLREG_SPR_EX1] | 0xF0;
            
        case 0x26: // Sprite extra color 2 (for multicolor sprites)
            return reg.current.colors[COLREG_SPR_EX2] | 0xF0;
            
        case 0x27: // Sprite color 1
        case 0x28: // Sprite color 2
        case 0x29: // Sprite color 3
        case 0x2A: // Sprite color 4
        case 0x2B: // Sprite color 5
        case 0x2C: // Sprite color 6
        case 0x2D: // Sprite color 7
        case 0x2E: // Sprite color 8
            
            return reg.current.colors[COLREG_SPR0 + addr - 0x27] | 0xF0;
    }
    
    assert(addr >= 0x2F && addr <= 0x3F);
    return 0xFF;
}

uint8_t
VIC::spypeek(uint16_t addr)
{
    assert(addr <= 0x003F);
    
    switch(addr) {
            
        case 0x1E:
            return spriteSpriteCollision;
            
        case 0x1F:
            return spriteBackgroundColllision;
            
        default:
            return peek(addr);
    }
}

void
VIC::poke(uint16_t addr, uint8_t value)
{
    assert(addr < 0x40);
    
    switch(addr) {
        case 0x00: // Sprite X (lower 8 bits)
        case 0x02:
        case 0x04:
        case 0x06:
        case 0x08:
        case 0x0A:
        case 0x0C:
        case 0x0E:
            
            reg.current.sprX[addr >> 1] &= 0x100;
            reg.current.sprX[addr >> 1] |= value;
            break;
        
        case 0x01: // Sprite Y
        case 0x03:
        case 0x05:
        case 0x07:
        case 0x09:
        case 0x0B:
        case 0x0D:
        case 0x0F:
            
            reg.current.sprY[addr >> 1] = value;
            break;
            
        case 0x10: // Sprite X (upper bit)
            
            for (unsigned i = 0; i < 8; i++) {
                reg.current.sprX[i] &= 0xFF;
                reg.current.sprX[i] |= GET_BIT(value, i) ? 0x100 : 0;
            }
            break;
            
        case 0x11: // Control register 1
            
            if ((reg.delayed.ctrl1 & 0x80) != (value & 0x80)) {
                
                reg.current.ctrl1 = value;
                // Check if we need to trigger a rasterline interrupt
                if (yCounter == rasterInterruptLine())
                    triggerDelayedIRQ(1);
                
            } else {
                reg.current.ctrl1 = value;
            }
            
            // Check the DEN bit. If it gets set somehwere in line 30, a bad
            // line conditions occurs.
            if (c64->rasterline == 0x30 && (value & 0x10))
                DENwasSetInRasterline30 = true;
            
            if ((badLine = badLineCondition()))
                delay |= VICSetDisplayState;
            
            upperComparisonVal = upperComparisonValue();
            lowerComparisonVal = lowerComparisonValue();
            break;
            
        case 0x12: // RASTER_COUNTER
            
            if (rasterIrqLine != value) {
                
                rasterIrqLine = value;
                
                // Check if we need to trigger a rasterline interrupt
                if (yCounter == rasterInterruptLine())
                    triggerDelayedIRQ(1);
            }
            return;
            
        case 0x13: // Lightpen X
        case 0x14: // Lightpen Y
            
            return;
            
        case 0x15: // SPRITE_ENABLED
            
            reg.current.sprEnable = value;
            break;
            
        case 0x16: // CONTROL_REGISTER_2
            
            reg.current.ctrl2 = value;
            leftComparisonVal = leftComparisonValue();
            rightComparisonVal = rightComparisonValue();
            break;
            
        case 0x17: // SPRITE Y EXPANSION
           
            reg.current.sprExpandY = value;
            cleared_bits_in_d017 = (~value) & (~expansionFF);
            
            /* "The expansion flip flip is set as long as the bit in MxYE in
             *  register $d017 corresponding to the sprite is cleared." [C.B.]
             */
            expansionFF |= ~value;
            break;
            
        case 0x18: // Memory address pointers
            
            // Inform the GUI if the second bit changes. It switches between
            // upper case or lower case mode.
            if ((value & 0x02) != (memSelect & 0x02)) {
                memSelect = value;
                c64->putMessage(MSG_CHARSET);
                return;
            }
            
            memSelect = value;
            return;
    
        case 0x19: // Interrupt Request Register (IRR)
            
            // Bits are cleared by writing '1'
            irr &= (~value) & 0x0F;
            
            if (!(irr & imr)) {
                delay |= VICReleaseIrq;
            }
            return;
            
        case 0x1A: // Interrupt Mask Register (IMR)
            
            imr = value & 0x0F;
            
            if (irr & imr) {
                triggerDelayedIRQ(1);
            } else {
                delay |= VICReleaseIrq;
            }
            return;
            
        case 0x1B: // Sprite priority
            
            reg.current.sprPriority = value;
            break;
            
        case 0x1C: // Sprite multicolor
            
            reg.current.sprMC = value;
            break;
            
        case 0x1D: // SPRITE_X_EXPAND
            reg.current.sprExpandX = value;
            break;
            
        case 0x1E:
        case 0x1F:
            // Writing has no effect
            return;
            
        case 0x20: // Color registers
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
            
            // Schedule the new color to show up in the next cycle
            reg.current.colors[addr - 0x20] = value & 0xF;
            
            // Emulate the gray dot bug
            if (hasGrayDotBug() && emulateGrayDotBug) {
                reg.delayed.colors[addr - 0x20] = 0xF;
            }
            break;
    }
    
    delay |= VICUpdateRegisters;
}

uint8_t
VIC::memAccess(uint16_t addr)
{
    /* VIC has only 14 address lines. To be able to access the complete 64KB
     * main memory, it inverts bit 0 and bit 1 of the CIA2 portA register and
     * uses these values as the upper two address bits.
     */
    
    assert((addr & 0xC000) == 0); // 14 bit address
    assert((bankAddr & 0x3FFF) == 0); // multiple of 16 KB
    
    addrBus = bankAddr | addr;
    
    // VIC memory mapping (http://www.harries.dk/files/C64MemoryMaps.pdf)
    // Note: Final Cartridge III (freezer mode) only works when BLANK is replaced
    //       by RAM. So this mapping might not be 100% correct.
    //
    //          Ultimax  Standard
    // 0xF000:   ROMH      RAM
    // 0xE000:   RAM       RAM
    // 0xD000:   RAM       RAM
    // 0xC000:   BLANK     RAM
    // --------------------------
    // 0xB000:   ROMH      RAM
    // 0xA000:   BLANK     RAM
    // 0x9000:   RAM       CHAR
    // 0x8000:   RAM       RAM
    // --------------------------
    // 0x7000:   ROMH      RAM
    // 0x6000:   BLANK     RAM
    // 0x5000:   BLANK     RAM
    // 0x4000:   BLANK     RAM
    // --------------------------
    // 0x3000:   ROMH      RAM
    // 0x2000:   BLANK     RAM
    // 0x1000:   BLANK     CHAR
    // 0x0000:   RAM       RAM
    
    if (!c64->getUltimax()) {
        switch (addrBus >> 12) {
            case 0x9:
            case 0x1:
                dataBus = c64->mem.rom[0xC000 + addr];
                break;
            default:
                dataBus = c64->mem.ram[addrBus];
        }
    } else {
        switch (addrBus >> 12) {
            case 0xF:
            case 0xB:
            case 0x7:
            case 0x3:
                dataBus = c64->expansionport.peek(addrBus | 0xF000);
                break;
            case 0xE:
            case 0xD:
            case 0x9:
            case 0x8:
            case 0x0:
                dataBus = c64->mem.ram[addrBus];
                break;
            default:
                dataBus = c64->mem.ram[addrBus];
        }
    }
    
    return dataBus;
}

uint8_t
VIC::memIdleAccess()
{
    /* "As described, the VIC accesses in every first clock phase although there
     *  are some cycles in which no other of the above mentioned accesses is
     *  pending. In this case, the VIC does an idle access; a read access to
     *  video address $3fff (i.e. to $3fff, $7fff, $bfff or $ffff depending on
     *  the VIC bank) of which the result is discarded." [C.B.]
     */
    return memAccess(0x3FFF);
}

void
VIC::cAccess()
{
    // Only proceed if the BA line is pulled down
    // if (!badLineCondition)
    //     return;
    
    // If BA is pulled down for at least three cycles, perform memory access
    if (BApulledDownForAtLeastThreeCycles()) {
        
        // |VM13|VM12|VM11|VM10| VC9| VC8| VC7| VC6| VC5| VC4| VC3| VC2| VC1| VC0|
        uint16_t addr = (VM13VM12VM11VM10() << 6) | vc;
        
        videoMatrix[vmli] = memAccess(addr);
        colorLine[vmli] = c64->mem.colorRam[vc] & 0x0F;
    }
    
    // VIC has no access, yet
    else {
        
        /* "Nevertheless, the VIC accesses the video matrix, or at least it
         *  tries, because as long as AEC is still high in the second clock
         *  phase, the address and data bus drivers D0-D7 of the VIC are in
         *  tri-state and the VIC reads the value $ff from D0-D7 instead of the
         *  data from the video matrix in the first three cycles. The data lines
         *  D8-D13 of the VIC however don't have tri-state drivers and are
         *  always set to input. But the VIC doesn't get valid Color RAM data
         *  from there either, because as AEC is high, the 6510 is still
         *  considered the bus master and unless it doesn't by chance want to
         *  read the next opcode from the Color RAM, the chip select input of
         *  the Color RAM is not active. [...]
         *  To make a long story short: In the first three cycles after BA went
         *  low, the VIC reads $ff as character pointers and as color
         *  information the lower 4 bits of the opcode after the access to
         *  $d011. Not until then, regular video matrix data is read." [C.B.]
         */
        videoMatrix[vmli] = 0xFF;
        colorLine[vmli] = c64->mem.ram[c64->cpu.getPC()] & 0x0F;
    }
}

void
VIC::gAccess()
{
    if (displayState) {
        
        /* "The address generator for the text/bitmap accesses (c- and
         *  g-accesses) has basically 3 modes for the g-accesses (the c-accesses
         *  always follow the same address scheme). In display state, the BMM
         *  bit selects either character generator accesses (BMM=0) or bitmap
         *  accesses (BMM=1). In idle state, the g-accesses are always done at
         *  video address $3fff. If the ECM bit is set, the address generator
         *  always holds the address lines 9 and 10 low without any other
         *  changes to the addressing scheme (e.g. the g-accesses in idle state
         *  then occur at address $39ff)." [C.B.]
         */
        
        //  BMM=1: |CB13| VC9| VC8|VC7|VC6|VC5|VC4|VC3|VC2|VC1|VC0|RC2|RC1|RC0|
        //  BMM=0: |CB13|CB12|CB11|D7 |D6 |D5 |D4 |D3 |D2 |D1 |D0 |RC2|RC1|RC0|
        
        // Determine value of BMM bit
        uint8_t bmm = GET_BIT(reg.delayed.ctrl1, 5);
        if (!is856x()) {
            bmm |= GET_BIT(reg.current.ctrl1, 5);
        }
        
        uint16_t addr;
        if (BMMbit()) {
            addr = (CB13() << 10) |
            (vc << 3) | rc;
        } else {
            addr = (CB13CB12CB11() << 10) |
            (videoMatrix[vmli] << 3) | rc;
        }
        
        /* "If the ECM bit is set, the address generator always holds the
         *  address lines 9 and 10 low without any other changes to the
         *  addressing scheme (e.g. the g-accesses in idle state then occur at
         *  address $39ff)." [C.B.]
         */
        if (ECMbit())
            addr &= 0xF9FF;
        
        // Store result
        gAccessResult.write(LO_LO_HI(memAccess(addr),                // Character
                                        colorLine[vmli],       // Color
                                        videoMatrix[vmli])); // Data
        
        
        // "VC and VMLI are incremented after each g-access in display state."
        vc = (vc + 1) & 0x3FF;
        vmli = (vmli + 1) & 0x3F;
        
    } else {
        
        // In idle state, g-accesses read from $3FFF
        uint16_t addr = ECMbit() ? 0x39FF : 0x3FFF;

        // Store result
        gAccessResult.write(memAccess(addr));
    }
}

void
VIC::pAccess(unsigned sprite)
{
    assert(sprite < 8);
    
    // |VM13|VM12|VM11|VM10|  1 |  1 |  1 |  1 |  1 |  1 |  1 |  Spr.-Nummer |
    spritePtr[sprite] = memAccess((VM13VM12VM11VM10() << 6) | 0x03F8 | sprite) << 6;
    
}

void
VIC::sFirstAccess(unsigned sprite)
{
    assert(sprite < 8);
    
    uint8_t data = 0x00; // TODO: VICE is doing this: vicii.last_bus_phi2;
    
    isFirstDMAcycle = (1 << sprite);
    
    if (spriteDmaOnOff & (1 << sprite)) {
        
        assert(BApulledDownForAtLeastThreeCycles());
        // if (BApulledDownForAtLeastThreeCycles())
        data = memAccess(spritePtr[sprite] | mc[sprite]);
        
        mc[sprite]++;
        mc[sprite] &= 0x3F; // 6 bit overflow
    }
    
    spriteSr[sprite].chunk1 = data;
}

void
VIC::sSecondAccess(unsigned sprite)
{
    assert(sprite < 8);
    
    uint8_t data = 0x00; // TODO: VICE is doing this: vicii.last_bus_phi2;
    bool memAccessed = false;
    
    isFirstDMAcycle = 0;
    isSecondDMAcycle = (1 << sprite);
    
    if (spriteDmaOnOff & (1 << sprite)) {
        
        assert(BApulledDownForAtLeastThreeCycles());
        // if (BApulledDownForAtLeastThreeCycles())
        {
            data = memAccess(spritePtr[sprite] | mc[sprite]);
            memAccessed = true;
        }
        
        mc[sprite]++;
        mc[sprite] &= 0x3F; // 6 bit overflow
    }
    
    // If no memory access has happened here, we perform an idle access
    // The obtained data might be overwritten by the third sprite access
    if (!memAccessed)
        memIdleAccess();
    
    spriteSr[sprite].chunk2 = data;
}

void
VIC::sThirdAccess(unsigned sprite)
{
    assert(sprite < 8);
    
    uint8_t data = 0x00; // TODO: VICE is doing this: vicii.last_bus_phi2;
    
    if (spriteDmaOnOff & (1 << sprite)) {
        
        assert(BApulledDownForAtLeastThreeCycles());
        // if (BApulledDownForAtLeastThreeCycles())
            data = memAccess(spritePtr[sprite] | mc[sprite]);
        
        mc[sprite]++;
        mc[sprite] &= 0x3F; // 6 bit overflow
    }
    
    spriteSr[sprite].chunk3 = data;
}

void VIC::sFinalize(unsigned sprite)
{
    assert(sprite < 8);
    isSecondDMAcycle = 0;
}


