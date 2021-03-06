// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension NSDraggingInfo {

    var url: URL? {
        let pasteBoard = draggingPasteboard
        let types = [NSPasteboard.PasteboardType.compatibleFileURL]
        if pasteBoard.availableType(from: types) != nil {
            return NSURL.init(from: pasteBoard) as URL?
        }
        return nil
    }
}

class RomDropView: NSImageView {

    @IBOutlet var parent: ConfigurationController!
    var c64: C64Proxy { return parent.c64 }
        
    override func awakeFromNib() {

        registerForDraggedTypes([NSPasteboard.PasteboardType.compatibleFileURL])
    }
    
    func acceptDragSource(url: URL) -> Bool { return false }

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {

        if let url = sender.url {
            if acceptDragSource(url: url) {
                image = NSImage.init(named: "rom_medium")
                return .copy
            }
        }
        return NSDragOperation()
    }
    
    override func draggingExited(_ sender: NSDraggingInfo?) {

        parent.refresh()
    }
    
    override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool {

        return true
    }

    /*
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {

    }
    */
    
    override func concludeDragOperation(_ sender: NSDraggingInfo?) {

        parent.refresh()
    }
}

class BasicRomDropView: RomDropView {

    override func acceptDragSource(url: URL) -> Bool {
        return c64.isPoweredOff && c64.isBasicRom(url)
    }
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        return sender.url == nil ? false : c64.loadBasicRom(fromFile: sender.url)
    }
}

class CharRomDropView: RomDropView {
    override func acceptDragSource(url: URL) -> Bool {
        return c64.isPoweredOff && c64.isCharRom(url)
    }
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        return sender.url == nil ? false : c64.loadCharRom(fromFile: sender.url)
    }
}

class KernalRomDropView: RomDropView {

    override func acceptDragSource(url: URL) -> Bool {
        return c64.isPoweredOff && c64.isKernalRom(url)
    }
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        return sender.url == nil ? false : c64.loadKernalRom(fromFile: sender.url)
    }
}

class Vc1541RomDropView: RomDropView {
    
    override func acceptDragSource(url: URL) -> Bool {
        return c64.isPoweredOff && c64.isVC1541Rom(url)
    }
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        return sender.url == nil ? false : c64.loadVC1541Rom(fromFile: sender.url)
    }
}
