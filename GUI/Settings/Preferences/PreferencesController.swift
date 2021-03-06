// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PreferencesController: DialogController {
    
    var pref: Preferences { return parent.pref }
    var gamePadManager: GamePadManager { return parent.gamePadManager! }
    var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    
    @IBOutlet weak var tabView: NSTabView!
    
    //
    // General preferences
    //
        
    // Drive
    @IBOutlet weak var emuDriveBlankDiskFormat: NSPopUpButton!
    @IBOutlet weak var emuEjectUnasked: NSButton!
    @IBOutlet weak var emuDriveSounds: NSButton!
    @IBOutlet weak var emuDriveSoundPan: NSPopUpButton!
    @IBOutlet weak var emuDriveInsertSound: NSButton!
    @IBOutlet weak var emuDriveEjectSound: NSButton!
    @IBOutlet weak var emuDriveHeadSound: NSButton!
    @IBOutlet weak var emuDriveConnectSound: NSButton!

    // Fullscreen
    @IBOutlet weak var emuAspectRatioButton: NSButton!
    @IBOutlet weak var emuExitOnEscButton: NSButton!

    // Snapshots and Screenshots
    @IBOutlet weak var emuAutoSnapshots: NSButton!
    @IBOutlet weak var emuSnapshotInterval: NSTextField!
    @IBOutlet weak var emuScreenshotSourcePopup: NSPopUpButton!
    @IBOutlet weak var emuScreenshotTargetPopup: NSPopUpButton!
        
    // Warp mode
    @IBOutlet weak var emuWarpMode: NSPopUpButton!

    // Misc
    @IBOutlet weak var emuPauseInBackground: NSButton!
    @IBOutlet weak var emuCloseWithoutAskingButton: NSButton!
    
    //
    // Devices preferences
    //
    
    // Tag of the button that is currently being recorded
    var devRecordedKey: Int?
    
    // Joystick emulation keys
    @IBOutlet weak var devLeft1: NSTextField!
    @IBOutlet weak var devLeft1button: NSButton!
    @IBOutlet weak var devRight1: NSTextField!
    @IBOutlet weak var devRight1button: NSButton!
    @IBOutlet weak var devUp1: NSTextField!
    @IBOutlet weak var devUp1button: NSButton!
    @IBOutlet weak var devDown1: NSTextField!
    @IBOutlet weak var devDown1button: NSButton!
    @IBOutlet weak var devFire1: NSTextField!
    @IBOutlet weak var devFire1button: NSButton!
    @IBOutlet weak var devLeft2: NSTextField!
    @IBOutlet weak var devLeft2button: NSButton!
    @IBOutlet weak var devRight2: NSTextField!
    @IBOutlet weak var devRight2button: NSButton!
    @IBOutlet weak var devUp2: NSTextField!
    @IBOutlet weak var devUp2button: NSButton!
    @IBOutlet weak var devDown2: NSTextField!
    @IBOutlet weak var devDown2button: NSButton!
    @IBOutlet weak var devFire2: NSTextField!
    @IBOutlet weak var devFire2button: NSButton!
    @IBOutlet weak var devDisconnectKeys: NSButton!
    
    // Joystick buttons
    @IBOutlet weak var devAutofire: NSButton!
    @IBOutlet weak var devAutofireCease: NSButton!
    @IBOutlet weak var devAutofireCeaseText: NSTextField!
    @IBOutlet weak var devAutofireBullets: NSTextField!
    @IBOutlet weak var devAutofireFrequency: NSSlider!
    
    // Mouse
    @IBOutlet weak var devMouseModel: NSPopUpButton!
    @IBOutlet weak var devMouseInfo: NSTextField!
    
    @IBOutlet weak var devOkButton: NSButton!
    @IBOutlet weak var devPowerButton: NSButton!
    
    //
    // Keymap preferences
    //
    
    @IBOutlet weak var keyMappingText: NSTextField!
    @IBOutlet weak var keyMappingPopup: NSPopUpButton!
    @IBOutlet weak var keyMapText: NSTextField!
    @IBOutlet weak var keyBox: NSBox!

    // Array holding a reference to the view of each key button
    var keyView = Array(repeating: nil as RecordButton?, count: 66)

    // Arrays holding the key caps for each record key
    var keyImage = Array(repeating: nil as NSImage?, count: 66)
    var pressedKeyImage = Array(repeating: nil as NSImage?, count: 66)
    var mappedKeyImage = Array(repeating: nil as NSImage?, count: 66)
    
    // The C64 key that has been selected to be mapped
    var selectedKey: C64Key?
    
    @IBOutlet weak var keyOkButton: NSButton!
    @IBOutlet weak var keyPowerButton: NSButton!
    
    //
    // Media files
    //
    
    @IBOutlet weak var medD64Popup: NSPopUpButton!
    @IBOutlet weak var medPrgPopup: NSPopUpButton!
    @IBOutlet weak var medT64Popup: NSPopUpButton!
    @IBOutlet weak var medTapPopup: NSPopUpButton!
    @IBOutlet weak var medCrtPopup: NSPopUpButton!

    @IBOutlet weak var medD64AutoTypeButton: NSButton!
    @IBOutlet weak var medPrgAutoTypeButton: NSButton!
    @IBOutlet weak var medT64AutoTypeButton: NSButton!
    @IBOutlet weak var medTapAutoTypeButton: NSButton!
    @IBOutlet weak var medCrtAutoTypeButton: NSButton!

    @IBOutlet weak var medD64AutoTypeText: NSTextField!
    @IBOutlet weak var medPrgAutoTypeText: NSTextField!
    @IBOutlet weak var medT64AutoTypeText: NSTextField!
    @IBOutlet weak var medTapAutoTypeText: NSTextField!
    @IBOutlet weak var medCrtAutoTypeText: NSTextField!

    // The tab to open first
    var firstTab: String?

    func showSheet(tab: String) {

        firstTab = tab
        showSheet()
    }

    override func awakeFromNib() {

        super.awakeFromNib()
        
        if let id = firstTab { tabView.selectTabViewItem(withIdentifier: id) }
        refresh()
    }

    override func sheetDidShow() {

        if let id = firstTab { tabView.selectTabViewItem(withIdentifier: id) }
    }

    func refresh() {
        
        if let id = tabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Emulator": refreshEmulatorTab()
            case "Devices": refreshDevicesTab()
            case "Keyboard": refreshKeyboardTab()
            case "Media": refreshMediaTab()
            default: break
            }
        }
    }
    
    @discardableResult
    func keyDown(with key: MacKey) -> Bool {
        
        if let id = tabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Devices": return devKeyDown(with: key)
            case "Keyboard": return mapSelectedKey(to: key)
            default: break
            }
        }
        return false
    }
    
    @IBAction override func okAction(_ sender: Any!) {
        
        track()
        pref.saveEmulatorUserDefaults()
        pref.saveDevicesUserDefaults()
        hideSheet()
    }
}

extension PreferencesController: NSTabViewDelegate {
    
    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        
        track()
        refresh()
    }
}

extension PreferencesController: NSTextFieldDelegate {
    
    func controlTextDidChange(_ obj: Notification) {
        
        track()
        
        if let view = obj.object as? NSTextField {
            
            let formatter = view.formatter as? NumberFormatter
            
            switch view {
                
            case emuSnapshotInterval:
                
                if formatter?.number(from: view.stringValue) != nil {
                    emuSnapshotIntervalAction(view)
                }
                
            case medD64AutoTypeText, medPrgAutoTypeText, medT64AutoTypeText,
                 medTapAutoTypeText, medCrtAutoTypeText:
                
                medAutoTextAction(view)
                
            case devAutofireBullets:
                
                if formatter?.number(from: view.stringValue) != nil {
                    devAutofireBulletsAction(view)
                }
                
            default:
                break
            }
        }
    }
}
