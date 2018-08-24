/*!
 * @header      PixelEngine.h
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 */
/*              This program is free software; you can redistribute it and/or modify
 *              it under the terms of the GNU General Public License as published by
 *              the Free Software Foundation; either version 2 of the License, or
 *              (at your option) any later version.
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *              GNU General Public License for more details.
 *
 *              You should have received a copy of the GNU General Public License
 *              along with this program; if not, write to the Free Software
 *              Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _PIXELENGINGE_INC
#define _PIXELENGINGE_INC

#include "VirtualComponent.h"
#include "C64_types.h"

// Forward declarations
class VIC;


// Depth of different drawing layers
#define BORDER_LAYER_DEPTH 0x10         /* in front of everything */
#define SPRITE_LAYER_FG_DEPTH 0x20      /* behind border */
#define FOREGROUND_LAYER_DEPTH 0x30     /* behind sprite 1 layer  */
#define SPRITE_LAYER_BG_DEPTH 0x40      /* behind foreground */
#define BACKGROUD_LAYER_DEPTH 0x50      /* behind sprite 2 layer */
#define BEIND_BACKGROUND_DEPTH 0x60     /* behind background */

//
// VIC state pipes
//

/*! @brief    A certain portion of VICs internal state
 *  @details  This structure comprises all state variables that need to be delayed to get
 *            the timing right.
 *  @note     A general note about state pipes:
 *            Each pipe comprises a certain portion of the VICs internal state. I.e., they
 *            comprise those state variables that are accessed by the pixel engine and need to
 *            be delayed by a certain amount to get the timing right. Most state variables need
 *            to be delayed by one cycle. An exception are the color registers that usually
 *            exhibit a value change somewhere in the middle of an pixel chunk. To implement the
 *            delay, both VIC and PixelEngine hold a pipe variable of their own, and the contents
 *            of the VICs variable is copied over the contents of the PixelEngines variable at
 *            the right time. Putting the state variables in seperate structures allows the
 *            compiler to optize the copy process.
 * @deprecated Will use TimeDelayed<> instead
 */
typedef struct {
    
    /*! @brief    Sprite X coordinates
     *  @details  The X coordinate is a 9 bit value. For each sprite, the lower
     *            8 bits are stored in a seperate IO register, while the
     *            uppermost bits are packed in a single register (0xD010). The
     *            sprites X coordinate is updated whenever one the corresponding
     *            IO register changes its value.
     */
    uint16_t spriteX[8];
    
    //! @brief    Sprite X expansion bits
    uint8_t spriteXexpand;
                
    //! @brief    Data value grabbed in gAccess()
    uint8_t g_data;
    
    //! @brief    Character value grabbed in gAccess()
    uint8_t g_character;
    
    //! @brief    Color value grabbed in gAccess()
    uint8_t g_color;
        
    //! @brief    Main frame flipflop
    // uint8_t mainFrameFF;
    
    //! @brief    Vertical frame Flipflop
    // uint8_t verticalFrameFF;
    
} PixelEnginePipe;


//! @class   PixelEngine
/*! @details This component is part of the virtual VICII chip and encapulates
 *           all the functionality that is related to the synthesis of pixels.
 *           Its main entry point are prepareForCycle() and draw() which are
 *           called in every VIC cycle inside the viewable range.
 */
class PixelEngine : public VirtualComponent {
    
    friend class VIC;
    
public:

    //! @brief    Reference to the connected video interface controller (VIC)
    //! @deprecated use c64->vic instead
    VIC *vic;
    
    //! @brief    Constructor
    PixelEngine();
    
    //! @brief    Destructor
    ~PixelEngine();
    
    //! @brief    Method from VirtualComponent
    void reset();

    //! @brief    Initializes both screenBuffers
    /*! @details  This function is needed for debugging, only. It write some
     *            recognizable pattern into both buffers.
     */
    void resetScreenBuffers();

    
    //
    // Pixel buffers and colors
    //
    
private:
    
    /*! @brief    Currently used RGBA values for all sixteen C64 colors
     *  @see      updatePalette()
     */
    uint32_t rgbaTable[16];
    
    /*! @brief    First screen buffer
     *  @details  The VIC chip writes its output into this buffer. The contents
     *            of the array is later copied into to texture RAM of your
     *            graphic card by the drawRect method in the GPU related code.
     */
    int *screenBuffer1 = new int[PAL_RASTERLINES * NTSC_PIXELS];
    
    /*! @brief    Second screen buffer
     *  @details  The VIC chip uses double buffering. Once a frame is drawn, the
     *            VIC chip writes the next frame to the second buffer.
     */
    int *screenBuffer2 = new int [PAL_RASTERLINES * NTSC_PIXELS];
    
    /*! @brief    Target screen buffer for all rendering methods
     *  @details  The variable points either to screenBuffer1 or screenBuffer2 
     */
    int *currentScreenBuffer;
    
    /*! @brief    Pointer to the beginning of the current rasterline
     *  @details  This pointer is used by all rendering methods to write pixels.
     *            It always points to the beginning of a rasterline, either in
     *            screenBuffer1 or screenBuffer2. It is reset at the beginning
     *            of each frame and incremented at the beginning of each
     *            rasterline.
     */
    int *pixelBuffer;
    
    /*! @brief    Synthesized pixel colors
     *  @details  The colors for the eight pixels of a single VICII cycle are
     *            stored temporaraily in this array. At the end of the cycle,
     *            they are translated into RGBA color values and copied into
     *            the screen buffer.
     */
    uint8_t colBuffer[8];
    
    /*! @brief    Z buffer
     *  @details  Depth buffering is used to determine pixel priority. In the
     *            various render routines, a color value is only retained, if it
     *            is closer to the view point. The depth of the closest pixel is
     *            kept in the z buffer. The lower the value, the closer it is to
     *            the viewer.
     */
    uint8_t zBuffer[8];
    
    /*! @brief    Indicates the source of a drawn pixel
     *  @details  Whenever a foreground pixel or sprite pixel is drawn, a
     *            distinct bit in the pixelSource array is set. The information
     *            is needed to detect sprite-sprite and sprite-background
     *            collisions.
     */
    uint8_t pixelSource[8];
    
    /*! @brief    Offset into pixelBuffer
     *  @details  Variable points to the first pixel of the currently drawn 8
     *            pixel chunk.
     */
    short bufferoffset;
    
public:
    
    /*! @brief    Returns the currently stable screen buffer
     *  @details  This method is called by the GPU code at the beginning of each
     *            frame.
     */
    void *screenBuffer() {
        
        if (currentScreenBuffer == screenBuffer1) {
            return screenBuffer2;
        } else {
            return screenBuffer1;
        }
    }

    
    //
    // Rastercycle information
    //

private:
    
    /*! @brief    Indicates wether we are in a visible display column or not
     *  @details  The visible columns comprise canvas columns and border
     *            columns. The first visible column is drawn in cycle 14 (first
     *            left border column) and the last in cycle ?? (fourth right
     *            border column).
     */
    bool visibleColumn;
    
    
    //
    // Execution functions
    //

public:
    
    //! @brief    Prepares for a new frame.
    void beginFrame();
    
    //! @brief    Prepares for a new rasterline.
    void beginRasterline();
    
    //! @brief    Finishes a rasterline.
    void endRasterline();
    
    //! @brief    Finishes a frame.
    void endFrame();

    
    //
    // VIC state latching
    //

    //! @brief    X expansion flip flop value of the currently drawn sprite
    bool spriteXExpansion;

    //! @brief    X coordinate of the currently drawn sprite
    uint16_t spriteXCoord;

    
    
    
    //! @brief    VIC register pipe
    //! @deprecated
    PixelEnginePipe pipe;
    
 
    //
    // Shift register logic for canvas pixels (handled in drawCanvasPixel)
    //
    
    /*! @brief    Main shift register
     *  @details  Eight bit shift register to synthesize canvas pixels.
     */
    struct {
        
        //! @brief    Shift register data
        uint8_t data;

        /*! @brief    Indicates whether the shift register can load data
         *  @details  If true, the register is loaded when the current x scroll
         *            offset matches the current pixel number.
         */
        bool canLoad; 
         
        /*! @brief    Multi-color synchronization flipflop
         *  @details  Whenever the shift register is loaded, the synchronization
         *            flipflop is also set. It is toggled with each pixel and
         *            used to synchronize the synthesis of multi-color pixels.
         */
        bool mc_flop;
        
        /*! @brief    Latched character info
         *  @details  Whenever the shift register is loaded, the current
         *            character value (which was once read during a gAccess) is
         *            latched. This value is used until the shift register loads
         *            again.
         */
        uint8_t latchedCharacter;
        
        /*! @brief    Latched color info
         *  @details  Whenever the shift register is loaded, the current color
         *            value (which was once read during a gAccess) is latched.
         *            This value is used until the shift register loads again.
         */
        uint8_t latchedColor;
        
        /*! @brief    Color bits
         *  @details  Every second pixel (as synchronized with mc_flop), the
         *            multi-color bits are remembered.
         */
        uint8_t colorbits;
        
        /*! @brief    Remaining bits to be pumped out
         *  @details  Makes sure no more than 8 pixels are outputted.
         */
        int remaining_bits;

    } sr;
    
    
    //
    // Shift register logic for sprite pixels (handled in drawSpritePixel)
    //
    
    /*! @brief    Sprite shift registers
     *  @details  The VIC chip has a 24 bit (3 byte) shift register for each
     *            sprite. It stores the sprite for one rasterline. If a sprite
     *            is a display candidate in the current rasterline, its shift
     *            register is activated when the raster X coordinate matches
     *            the sprites X coordinate. The comparison is done in method
     *            drawSprite(). Once a shift register is activated, it remains
     *            activated until the beginning of the next rasterline. However,
     *            after an activated shift register has dumped out its 24 pixels,
     *            it can't draw anything else than transparent pixels (which is
     *            the same as not to draw anything). An exception is during DMA
     *            cycles. When a shift register is activated during such a cycle,
     *            it freezes a short period of time in which it repeats the
     *            previous drawn pixel.
     */
    struct {
        
        //! @brief    Shift register data (24 bit)
        uint32_t data;
        
        //! @brief    The shift register data is read in three chunks
        uint8_t chunk1, chunk2, chunk3;
        
        /*! @brief    Remaining bits to be pumped out
         *  @details  At the beginning of each rasterline, this value is
         *            initialized with -1 and set to 26 when the horizontal
         *            trigger condition is met (sprite X trigger coord reaches
         *            xCounter). When all bits are drawn, this value reaches 0.
         */
        int remaining_bits;

        /*! @brief    Multi-color synchronization flipflop
         *  @details  Whenever the shift register is loaded, the synchronization
         *            flipflop is also set. It is toggled with each pixel and
         *            used to synchronize the synthesis of multi-color pixels.
         */
        bool mc_flop;

        //! @brief    x expansion synchronization flipflop
        bool exp_flop;

        /*! @brief    Color bits of the currently processed pixel
         *  @details  In single-color mode, these bits are updated every cycle
         *            In multi-color mode, these bits are updated every second
         *            cycle (synchronized with mc_flop).
         */
        uint8_t col_bits;

        //! @brief    Sprite color
        uint8_t spriteColor;
        
    } sprite_sr[8];

    //! Sprite extra color 1 (same for all sprites)
    uint8_t spriteExtraColor1;
    
    //! Sprite extra color 2 (same for all sprites)
    uint8_t spriteExtraColor2;

    /*! @brief    Loads the sprite shift register.
     *  @details  The shift register is loaded with the three data bytes fetched
     *            in the previous sAccesses.
     */
    void loadShiftRegister(unsigned nr) {
        sprite_sr[nr].data =
        (sprite_sr[nr].chunk1 << 16) |
        (sprite_sr[nr].chunk2 << 8) |
        (sprite_sr[nr].chunk3);
    }
    
    
    //
    // Accessing colors
    //
    
    
    //
    // External drawing routines (called inside the VICII::cycle functions)
    //

public:
  
    /*! @brief    Synthesize 8 pixels according the the current drawing context.
     *  @details  This is the main entry point and is invoked in each VIC
     *            drawing cycle, except cycle 17 and cycle 55 which are handles
     *            seperately for speedup purposes. To get the correct output,
     *            preparePixelEngine() must be called one cycle before.
     */
    void draw();

    //! @brief    Special draw routine for cycle 17
    void draw17();

    //! @brief    Special draw routine for cycle 55
    void draw55();

    /*! @brief    Draw routine for cycles outside the visible screen region.
     *  @details  The sprite sequencer needs to be run outside the visible area,
     *            although no pixels will be drawn (drawing is omitted by having
     *            visibleColumn set to false.
     */
    void drawOutsideBorder();
    
    
    //
    // Internal drawing routines (called by the external ones)
    //
    
private:
    
    /*! @brief    Draws a part of the border
     *  @details  Invoked inside draw()
     */
    void drawBorder();
    
    /*! @brief    Draws a part of the border
     *  @details  Invoked inside draw17() 
     */
    void drawBorder17();
    
    /*! @brief    Draws a part of the border
     *  @details  Invoked inside draw55()
     */
    void drawBorder55();

    /*! @brief    Draws 8 canvas pixels
     *  @details  Invoked inside draw()
     */
    void drawCanvas();
    
    /*! @brief    Draws a single canvas pixel
     *  @param    pixelnr is the pixel number and must be in the range 0 to 7
     *  @param    loadShiftReg is set to true if the shift register needs to be
     *            reloaded
     *  @param    updateColors is set to true if the four selectable colors
     *            might have changed.
     */
    void drawCanvasPixel(uint8_t pixelnr,
                         uint8_t mode,
                         uint8_t d016,
                         bool loadShiftReg,
                         bool updateColors);
    
    /*! @brief    Draws 8 sprite pixels
     *  @details  Invoked inside draw() 
     */
    void drawSprites();

    /*! @brief    Draws a single sprite pixel for all sprites
     *  @param    pixelnr  Pixel number (0 to 7)
     *  @param    freeze   If the i-th bit is set to 1, the i-th shift register
     *                     will freeze temporarily
     *  @param    halt     If the i-th bit is set to 1, the i-th shift register
     *                     will be deactivated
     *  @param    load     If the i-th bit is set to 1, the i-th shift register
     *                     will grab new data bits
     */
    /*
    void drawSpritePixel(unsigned pixelnr,
                         uint8_t freeze,
                         uint8_t halt,
                         uint8_t load);
     */
    
    /*! @brief    Draws a single sprite pixel for a single sprite
     *  @param    spritenr Sprite number (0 to 7)
     *  @param    pixelnr  Pixel number (0 to 7)
     *  @param    freeze   If set to true, the sprites shift register will
     *                     freeze temporarily
     *  @param    halt     If set to true, the sprites shift shift register will
     *                     be deactivated
     *  @param    load     If set to true, the sprites shift shift register will
     *                     grab new data bits
     */
    void drawSpritePixel(unsigned spritenr,
                         unsigned pixelnr,
                         bool freeze,
                         bool halt,
                         bool load);

    /*! @brief    Draws all sprites into the pixelbuffer
     *  @details  A sprite is only drawn if it's enabled and if sprite drawing
     *            is not switched off for debugging
     */
    void drawAllSprites();
    
    /*! @brief    Draw single sprite into pixel buffer
     *  @details  Helper function for drawSprites 
     */
    void drawSprite(uint8_t nr);
    
    
    //
    // Mid level drawing (semantic pixel rendering)
    //

private:
    
    /*! @brief    This is where loadColors() stores all retrieved colors
     *  @details  [0] : color for '0'  pixels in single color mode
     *                         or '00' pixels in multicolor mode
     *            [1] : color for '1'  pixels in single color mode
     *                         or '01' pixels in multicolor mode
     *            [2] : color for '10' pixels in multicolor mode
     *            [3] : color for '11' pixels in multicolor mode 
     */
    uint8_t col[4];

    //! @brief    Sprite colors
    uint64_t sprExtraCol1;
    uint64_t sprExtraCol2;
    uint64_t sprCol[8];

public:
    
    //! @brief    Determines pixel colors accordig to the provided display mode
    void loadColors(uint8_t pixelNr, uint8_t mode,
                    uint8_t characterSpace, uint8_t colorSpace);

    /*! @brief    Draws single canvas pixel in single-color mode
     *  @details  1s are drawn with drawForegroundPixel, 0s are drawn with
     *            drawBackgroundPixel. Uses the drawing colors that are setup by
     *            loadColors().
     */
    void setSingleColorPixel(unsigned pixelnr, uint8_t bit);
    
    /*! @brief    Draws single canvas pixel in multi-color mode
     *  @details  The left of the two color bits determines whether
     *            drawForegroundPixel or drawBackgroundPixel is used.
     *            Uses the drawing colors that are setup by loadColors(). 
     */
    void setMultiColorPixel(unsigned pixelnr, uint8_t two_bits);
    
    /*! @brief    Draws single sprite pixel in single-color mode
     *  @details  Uses the drawing colors that are setup by updateSpriteColors 
     */
    void setSingleColorSpritePixel(unsigned spritenr, unsigned pixelnr, uint8_t bit);
    
    /*! @brief    Draws single sprite pixel in multi-color mode
     *  @details  Uses the drawing colors that are setup by updateSpriteColors 
     */
    void setMultiColorSpritePixel(unsigned spritenr, unsigned pixelnr, uint8_t two_bits);

    /*! @brief    Draws a single sprite pixel
     *  @details  This function is invoked by setSingleColorSpritePixel() and
     *            setMultiColorSpritePixel(). It takes care of collison and invokes
     *            setSpritePixel(4) to actually render the pixel.
     */
    void drawSpritePixel(unsigned pixelnr, uint8_t color, int nr);
    
    
    //
    // Low level drawing (pixel buffer access)
    //
    
public:

    //! @brief    Draws the frame pixels for a certain range
    void drawFramePixels(unsigned first, unsigned last, uint8_t color);

    //! @brief    Draws a single frame pixel
    void drawFramePixel(unsigned nr, uint8_t color) { drawFramePixels(nr, nr, color); }

    //! @brief    Draws all eight frame pixels of a single cycle
    void drawFramePixels(uint8_t color) { drawFramePixels(0, 7, color); }

    //! @brief    Draw a single foreground pixel
    void drawForegroundPixel(unsigned pixelnr, uint8_t color);
    
    //! @brief    Draw a single foreground pixel
    //! @deprecated
    // void setForegroundPixel(unsigned pixelnr, int rgba);

    //! @brief    Draw a single background pixel
    void drawBackgroundPixel(unsigned pixelNr, uint8_t color);

    //! @brief    Draw a single background pixel
    //! @deprecated
    // void setBackgroundPixel(unsigned pixelnr, int rgba);

    //! @brief    Draw eight background pixels in a row
    void drawEightBackgroundPixels(uint8_t color) {
        for (unsigned i = 0; i < 8; i++) drawBackgroundPixel(i, color); }

    //! @brief    Draw eight background pixels in a row
    //! @deprecated
    /*
    void setEightBackgroundPixels(int rgba) {
        for (unsigned i = 0; i < 8; i++) setBackgroundPixel(i, rgba); }
    */
    
    //! @brief    Draw a single sprite pixel
    void putSpritePixel(unsigned pixelnr, uint8_t color, int depth, int source);
    
    //! @brief    Draw a single sprite pixel
    //! @deprecated
    // void setSpritePixel(unsigned pixelnr, int rgba, int depth, int source);

    /*! @brief    Copies eight synthesized pixels into to the pixel buffer.
     *  @details  Each pixel is first translated to the corresponding RGBA value
     *            and then copied over.
     */
    void copyPixels();
    
    /*! @brief    Extend border to the left and right to look nice.
     *  @details  This functions replicates the color of the leftmost and
     *            rightmost pixel
     */
    void expandBorders();

    /*! @brief    Draw a horizontal colored line into the screen buffer
     *  @details  This method is utilized for debugging purposes, only.
     */
    void markLine(uint8_t color, unsigned start = 0, unsigned end = NTSC_PIXELS);
    
};

#endif
