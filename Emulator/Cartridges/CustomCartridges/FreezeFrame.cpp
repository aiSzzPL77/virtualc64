// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "C64.h"

void
FreezeFrame::_reset()
{
    Cartridge::_reset();
    
    // In Ultimax mode, the same ROM chip that appears in ROML also appears
    // in ROMH. By default, it it appears in ROML only, so let's bank it in
    // ROMH manually.
    bankInROMH(0, 0x2000, 0);
}

u8
FreezeFrame::peekIO1(u16 addr)
{
    // Reading from IO1 switched to 8K game mode
    expansionport.setCartridgeMode(CRT_8K);
    return 0;
}

u8
FreezeFrame::peekIO2(u16 addr)
{
    // Reading from IO2 disables the cartridge
    expansionport.setCartridgeMode(CRT_OFF);
    return 0;
}

const char *
FreezeFrame::getButtonTitle(unsigned nr)
{
    return (nr == 1) ? "Freeze" : NULL;
}

void
FreezeFrame::pressButton(unsigned nr)
{
    if (nr == 1) {
        
        // Pressing the freeze button triggers an NMI in Ultimax mode
        suspend();
        expansionport.setCartridgeMode(CRT_ULTIMAX);
        cpu.pullDownNmiLine(INTSRC_EXP);
        resume();
    }
}

void
FreezeFrame::releaseButton(unsigned nr)
{
    if (nr == 1) {
        
        suspend();
        cpu.releaseNmiLine(INTSRC_EXP);
        resume();
    }
}
