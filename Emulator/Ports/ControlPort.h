// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef CONTROLPORT_H
#define CONTROLPORT_H

#include "C64Component.h"

class ControlPort : public C64Component {

private:
    
    // Represented control port (1 or 2)
    int nr;
    
    // True, if button is pressed
    bool button;

    /* Horizontal joystick position
     * Valid valued are -1 (LEFT), 1 (RIGHT), or 0 (RELEASED)
     */
    int axisX;

    /* Vertical joystick position
     * Valid valued are -1 (UP), 1 (DOWN), or 0 (RELEASED)
     */
    int axisY;
    
    // True if multi-shot mode in enabled
    bool autofire;

    // Number of bullets per gun volley
    int autofireBullets;

    // Autofire frequency in Hz
    float autofireFrequency;

    // Bullet counter used in multi-fire mode
    u64 bulletCounter; 

    // Next frame to auto-press or auto-release the fire button
    u64 nextAutofireFrame;
    
    
    //
    // Initializing
    //
    
public:
 
    ControlPort(int p, C64 &ref);

private:
    
    void _reset() override;
    
    
    //
    // Configuring
    //
    
public:
    
    bool getAutofire() { return autofire; }
    void setAutofire(bool value);

    int getAutofireBullets() { return autofireBullets; }
    void setAutofireBullets(int value);

    float getAutofireFrequency() { return autofireFrequency; }
    void setAutofireFrequency(float value) { autofireFrequency = value; }
    
    
    //
    // Analyzing
    //
    
private:
    
    void _dump() override;
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }
    
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(u8 *buffer) override;
    
    
    //
    // Emulating
    //
    
public:
    


    // Updates variable nextAutofireFrame
    void scheduleNextShot();
    
    /* This method is invoked at the end of each frame. It is needed to
     * implement the autofire functionality, only.
     */
    void execute();
    
    // Triggers a joystick event
    void trigger(GamePadAction event);
    
    /* Returns the current joystick movement in form a bit mask. The bits are
     * in the same order as they show up in the CIA's data port registers.
     */
    u8 bitmask();

    // Returns the potentiometer values (analog mouse)
    u8 potX();
    u8 potY();
};

#endif
