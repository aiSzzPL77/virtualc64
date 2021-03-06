// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FUNPLAY_H
#define _FUNPLAY_H

#include "Cartridge.h"

class Funplay : public Cartridge {
    
public:

    Funplay(C64 &ref) : Cartridge(ref, "Funplay") { };
    CartridgeType getCartridgeType() override { return CRT_FUNPLAY; }
    
    
    //
    // Accessing cartridge memory
    //
    
public:
    
    void pokeIO1(u16 addr, u8 value) override;
};

#endif
