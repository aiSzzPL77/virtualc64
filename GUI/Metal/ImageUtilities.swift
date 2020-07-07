// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension UInt32 {
    
    init(r: UInt8, g: UInt8, b: UInt8) {
        let red = UInt32(r) << 24
        let green = UInt32(g) << 16
        let blue = UInt32(b) << 8
        self.init(bigEndian: red | green | blue)
    }
}

//
// Extensions to MTLTexture
//

extension MTLTexture {
    
}

//
// Extensions to NSImage
//

public extension NSImage {
    
    convenience init(color: NSColor, size: NSSize) {
        
        self.init(size: size)
        lockFocus()
        color.drawSwatch(in: NSRect(origin: .zero, size: size))
        unlockFocus()
    }

    static func make(texture: MTLTexture, rect: CGRect) -> NSImage? {
        
        guard let cgImage = CGImage.make(texture: texture, rect: rect) else {
            track("Failed to create CGImage.")
            return nil
        }
        
        let size = NSSize(width: cgImage.width, height: cgImage.height)
        return NSImage(cgImage: cgImage, size: size)
    }

    static func make(data: UnsafeMutableRawPointer, rect: CGSize) -> NSImage? {
        
        guard let cgImage = CGImage.make(data: data, size: rect) else {
            track("Failed to create CGImage.")
            return nil
        }
        
        let size = NSSize(width: cgImage.width, height: cgImage.height)
        return NSImage(cgImage: cgImage, size: size)
    }

    func expand(toSize size: NSSize) -> NSImage? {
 
        let newImage = NSImage.init(size: size)
    
        NSGraphicsContext.saveGraphicsState()
        newImage.lockFocus()

        let t = NSAffineTransform()
        t.translateX(by: 0.0, yBy: size.height)
        t.scaleX(by: 1.0, yBy: -1.0)
        t.concat()
        
        let inRect = NSRect.init(x: 0, y: 0, width: size.width, height: size.height)
        let fromRect = NSRect.init(x: 0, y: 0, width: self.size.width, height: self.size.height)
        let operation = NSCompositingOperation.copy
        self.draw(in: inRect, from: fromRect, operation: operation, fraction: 1.0)
        
        newImage.unlockFocus()
        NSGraphicsContext.restoreGraphicsState()
        
        return newImage
    }
    
    func toTexture(device: MTLDevice) -> MTLTexture? {
 
        // let imageRect = NSMakeRect(0, 0, self.size.width, self.size.height);
        let imageRef = self.cgImage(forProposedRect: nil, context: nil, hints: nil)
        
        // Create a suitable bitmap context for extracting the bits of the image
        let width = imageRef!.width
        let height = imageRef!.height
    
        if width == 0 || height == 0 { return nil }
        
        // Allocate memory
        guard let data = malloc(height * width * 4) else { return nil }
        let rawBitmapInfo =
            CGImageAlphaInfo.noneSkipLast.rawValue |
                CGBitmapInfo.byteOrder32Big.rawValue
        let bitmapContext = CGContext(data: data,
                                      width: width,
                                      height: height,
                                      bitsPerComponent: 8,
                                      bytesPerRow: 4 * width,
                                      space: CGColorSpaceCreateDeviceRGB(),
                                      bitmapInfo: rawBitmapInfo)

        bitmapContext?.translateBy(x: 0.0, y: CGFloat(height))
        bitmapContext?.scaleBy(x: 1.0, y: -1.0)
        bitmapContext?.draw(imageRef!, in: CGRect.init(x: 0, y: 0, width: width, height: height))
        // CGContextDrawImage(bitmapContext!, CGRectMake(0, 0, CGFloat(width), CGFloat(height)), imageRef)
        
        let textureDescriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: MTLPixelFormat.rgba8Unorm,
            width: width,
            height: height,
            mipmapped: false)
        let texture = device.makeTexture(descriptor: textureDescriptor)
        let region = MTLRegionMake2D(0, 0, width, height)
        texture?.replace(region: region, mipmapLevel: 0, withBytes: data, bytesPerRow: 4 * width)

        free(data)
        return texture
    }
}

//
// Extensions to MetalView
//

extension Renderer {

    //
    // Image handling
    //

    func screenshot(texture: MTLTexture) -> NSImage? {

        // Use the blitter to copy the texture data back from the GPU
        let queue = texture.device.makeCommandQueue()!
        let commandBuffer = queue.makeCommandBuffer()!
        let blitEncoder = commandBuffer.makeBlitCommandEncoder()!
        blitEncoder.synchronize(texture: texture, slice: 0, level: 0)
        blitEncoder.endEncoding()
        commandBuffer.commit()
        commandBuffer.waitUntilCompleted()
        
        return NSImage.make(texture: texture, rect: textureRect)
    }
    
    func screenshot(afterUpscaling: Bool = true) -> NSImage? {

        if afterUpscaling {
            return screenshot(texture: upscaledTexture)
        } else {
            return screenshot(texture: emulatorTexture)
        }
    }
    
    func createBackgroundTexture() -> MTLTexture? {

        // Grab the current wallpaper as an NSImage
        let opt = CGWindowListOption.optionOnScreenOnly
        let id = CGWindowID(0)
        guard
            let windows = CGWindowListCopyWindowInfo(opt, id)! as? [NSDictionary],
            let screenBounds = NSScreen.main?.frame else { return nil }
        
        // Iterate through all windows
        var cgImage: CGImage?
        for i in 0 ..< windows.count {
            
            let window = windows[i]
            
            // Skip all windows that are not owned by the dock
            let owner = window["kCGWindowOwnerName"] as? String
            if owner != "Dock" {
                continue
            }
            
            // Skip all windows that do not have the same bounds as the main screen
            guard
                let bounds = window["kCGWindowBounds"] as? NSDictionary,
                let width  = bounds["Width"] as? CGFloat,
                let height = bounds["Height"] as? CGFloat  else { continue }

            if width != screenBounds.width || height != screenBounds.height {
                continue
            }
            
            // Skip all windows without having a name
            guard let name = window["kCGWindowName"] as? String else {
                continue
            }
                
            // Skip all windows with a name other than "Desktop picture - ..."
            if !name.hasPrefix("Desktop Picture") {
                continue
            }
                
            // Found it!
            guard let nr = window["kCGWindowNumber"] as? Int else {
                continue
            }

            cgImage = CGWindowListCreateImage(
                CGRect.null,
                CGWindowListOption(arrayLiteral: CGWindowListOption.optionIncludingWindow),
                CGWindowID(nr),
                [])!
            break
        }
        
        // Create image
        var wallpaper: NSImage?
        if cgImage != nil {
            wallpaper = NSImage.init(cgImage: cgImage!, size: NSSize.zero)
            wallpaper = wallpaper?.expand(toSize: NSSize(width: 1024, height: 512))
        } else {
            // Fall back to an opaque gray background
            let size = NSSize(width: 128, height: 128)
            wallpaper = NSImage(color: .lightGray, size: size)
        }
        
        // Return image as texture
        return wallpaper?.toTexture(device: device)
    }
}
