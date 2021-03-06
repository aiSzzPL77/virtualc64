// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Preferences
 *
 * This class stores all emulator settings that belong to the application level.
 * There is a single object of this class stored in the application delegate.
 * The object is shared among all emulator instances.
 *
 * See class "Configuration" for instance specific settings.
 */

class Preferences {
    
    var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    
    //
    // General
    //
    
    // Floppy
     var driveBlankDiskFormat = EmulatorDefaults.std.driveBlankDiskFormat
     var driveBlankDiskFormatIntValue: Int {
         get { return Int(driveBlankDiskFormat.rawValue) }
         set { driveBlankDiskFormat = FileSystemType.init(newValue) }
     }
     var driveEjectUnasked = EmulatorDefaults.std.driveEjectUnasked
     var driveSounds = EmulatorDefaults.std.driveSounds
     var driveSoundPan = EmulatorDefaults.std.driveSoundPan
     var driveInsertSound = EmulatorDefaults.std.driveInsertSound
     var driveEjectSound = EmulatorDefaults.std.driveEjectSound
     var driveHeadSound = EmulatorDefaults.std.driveHeadSound
     var driveConnectSound = EmulatorDefaults.std.driveConnectSound
     
     // Fullscreen
     var keepAspectRatio = EmulatorDefaults.std.keepAspectRatio
     var exitOnEsc = EmulatorDefaults.std.exitOnEsc
         
     // Snapshots and screenshots
     var autoSnapshots = EmulatorDefaults.std.autoSnapshots
     var snapshotInterval = 0 {
         didSet { for c in myAppDelegate.controllers { c.startSnapshotTimer() } }
     }
     var autoScreenshots = EmulatorDefaults.std.autoScreenshots

    var screenshotSource = EmulatorDefaults.std.screenshotSource
     var screenshotTarget = EmulatorDefaults.std.screenshotTarget
     var screenshotTargetIntValue: Int {
         get { return Int(screenshotTarget.rawValue) }
         set { screenshotTarget = NSBitmapImageRep.FileType(rawValue: UInt(newValue))! }
     }
     
     // Warp mode
     var warpMode = EmulatorDefaults.std.warpMode {
         didSet { for c in myAppDelegate.controllers { c.updateWarp() } }
     }
     var warpModeIntValue: Int {
         get { return Int(warpMode.rawValue) }
         set { warpMode = WarpMode.init(rawValue: newValue)! }
     }
     
     // Misc
     var closeWithoutAsking = EmulatorDefaults.std.closeWithoutAsking
     var pauseInBackground = EmulatorDefaults.std.pauseInBackground
    
    //
    // Media
    //
    
    var mountAction: [String: AutoMountAction] = MediaDefaults.std.mountAction
    var autoType: [String: Bool] = MediaDefaults.std.autoType
    var autoText: [String: String] = MediaDefaults.std.autoText
    
    //
    // Devices
    //
    
    // Emulation keys
    var keyMaps = [ DevicesDefaults.std.joyKeyMap1,
                    DevicesDefaults.std.joyKeyMap2 ]
    
    // Joystick
    var disconnectJoyKeys = DevicesDefaults.std.disconnectJoyKeys
    var autofire = DevicesDefaults.std.autofire {
        didSet {
            for c64 in myAppDelegate.proxies {
                c64.port1.setAutofire(autofire)
                c64.port2.setAutofire(autofire)
            }
        }
    }
    var autofireBullets = DevicesDefaults.std.autofireBullets {
        didSet {
            for c64 in myAppDelegate.proxies {
                c64.port1.setAutofireBullets(autofireBullets)
                c64.port2.setAutofireBullets(autofireBullets)
            }
        }
    }
    var autofireFrequency = DevicesDefaults.std.autofireFrequency {
        didSet {
            for c64 in myAppDelegate.proxies {
                c64.port1.setAutofireFrequency(autofireFrequency)
                c64.port2.setAutofireFrequency(autofireFrequency)
            }
        }
    }
    
    // Mouse
    var mouseModel = MouseModel.MOUSE1350
    
    //
    // Keyboard
    //
    
    // Mapping
    var mapKeysByPosition = false
    var keyMap: [MacKey: C64Key] = [:] 
    
    //
    // General
    //
    
    func loadEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Floppy
        driveBlankDiskFormatIntValue = defaults.integer(forKey: Keys.driveBlankDiskFormat)
        driveEjectUnasked = defaults.bool(forKey: Keys.driveEjectUnasked)
        driveSounds = defaults.bool(forKey: Keys.driveSounds)
        driveSoundPan = defaults.double(forKey: Keys.driveSoundPan)
        driveInsertSound = defaults.bool(forKey: Keys.driveInsertSound)
        driveEjectSound = defaults.bool(forKey: Keys.driveEjectSound)
        driveHeadSound = defaults.bool(forKey: Keys.driveHeadSound)
        driveConnectSound = defaults.bool(forKey: Keys.driveConnectSound)
        
        // Fullscreen
        keepAspectRatio = defaults.bool(forKey: Keys.keepAspectRatio)
        exitOnEsc = defaults.bool(forKey: Keys.exitOnEsc)
        
        // Snapshots and screenshots
        autoSnapshots = defaults.bool(forKey: Keys.autoSnapshots)
        snapshotInterval = defaults.integer(forKey: Keys.autoSnapshotInterval)
        autoScreenshots = defaults.bool(forKey: Keys.autoScreenshots)
        screenshotSource = defaults.integer(forKey: Keys.screenshotSource)
        screenshotTargetIntValue = defaults.integer(forKey: Keys.screenshotTarget)
        
        // Warp mode
        warpModeIntValue = defaults.integer(forKey: Keys.warpMode)
        
        // Misc
        pauseInBackground = defaults.bool(forKey: Keys.pauseInBackground)
        closeWithoutAsking = defaults.bool(forKey: Keys.closeWithoutAsking)
    }
    
    func saveEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Floppy
        defaults.set(screenshotTargetIntValue, forKey: Keys.screenshotTarget)
        defaults.set(driveEjectUnasked, forKey: Keys.driveEjectUnasked)
        defaults.set(driveSounds, forKey: Keys.driveSounds)
        defaults.set(driveSoundPan, forKey: Keys.driveSoundPan)
        defaults.set(driveInsertSound, forKey: Keys.driveInsertSound)
        defaults.set(driveEjectSound, forKey: Keys.driveEjectSound)
        defaults.set(driveHeadSound, forKey: Keys.driveHeadSound)
        defaults.set(driveConnectSound, forKey: Keys.driveConnectSound)
        defaults.set(driveBlankDiskFormatIntValue, forKey: Keys.driveBlankDiskFormat)
        
        // Fullscreen
        defaults.set(keepAspectRatio, forKey: Keys.keepAspectRatio)
        defaults.set(exitOnEsc, forKey: Keys.exitOnEsc)
        
        // Snapshots and screenshots
        defaults.set(autoSnapshots, forKey: Keys.autoSnapshots)
        defaults.set(snapshotInterval, forKey: Keys.autoSnapshotInterval)
        defaults.set(autoScreenshots, forKey: Keys.autoScreenshots)
        defaults.set(screenshotSource, forKey: Keys.screenshotSource)
        
        // Warp mode
        defaults.set(warpModeIntValue, forKey: Keys.warpMode)
        
        // Misc
        defaults.set(pauseInBackground, forKey: Keys.pauseInBackground)
        defaults.set(closeWithoutAsking, forKey: Keys.closeWithoutAsking)
    }
    
    //
    // Devices
    //
        
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.decode(&keyMaps[0], forKey: Keys.joyKeyMap1)
        defaults.decode(&keyMaps[1], forKey: Keys.joyKeyMap2)
        disconnectJoyKeys = defaults.bool(forKey: Keys.disconnectJoyKeys)
        
        // Joysticks
        autofire = defaults.bool(forKey: Keys.autofire)
        autofireBullets = defaults.integer(forKey: Keys.autofireBullets)
        autofireFrequency = defaults.float(forKey: Keys.autofireFrequency)
    }
    
    func saveDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.encode(keyMaps[0], forKey: Keys.joyKeyMap1)
        defaults.encode(keyMaps[1], forKey: Keys.joyKeyMap2)
        defaults.set(disconnectJoyKeys, forKey: Keys.disconnectJoyKeys)
        
        // Joysticks
        defaults.set(autofire, forKey: Keys.autofire)
        defaults.set(autofireBullets, forKey: Keys.autofireBullets)
        defaults.set(autofireFrequency, forKey: Keys.autofireFrequency)
    }
    
    //
    // Keyboard
    //

    func loadKeyboardUserDefaults() {
        
        let defaults = UserDefaults.standard
        mapKeysByPosition = defaults.bool(forKey: Keys.mapKeysByPosition)
        defaults.decode(&keyMap, forKey: Keys.keyMap)
    }
    
    func saveKeyboardUserDefaults() {
        
        let defaults = UserDefaults.standard
        defaults.encode(keyMap, forKey: Keys.keyMap)
        defaults.set(mapKeysByPosition, forKey: Keys.mapKeysByPosition)
    }
    
    //
    // Media
    //
    
    func loadMediaUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.decode(&mountAction, forKey: Keys.mountAction)
        defaults.decode(&autoType, forKey: Keys.autoType)
        defaults.decode(&autoText, forKey: Keys.autoText)
    }
    
    func saveMediaUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.encode(mountAction, forKey: Keys.mountAction)
        defaults.encode(autoType, forKey: Keys.autoType)
        defaults.encode(autoText, forKey: Keys.autoText)
    }
}
