// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {
    
    func refreshEmulatorTab() {
        
        track()
        
        guard let c64 = proxy, let controller = myController else { return }
        
        // VC1541
        emuWarpLoad.state = c64.warpLoad() ? .on : .off
        emuDriveSounds.state = myAppDelegate.pref.driveSounds ? .on : .off
        
        // Screenshots
        emuScreenshotSourcePopup.selectItem(withTag: controller.screenshotSource)
        emuScreenshotTargetPopup.selectItem(withTag: controller.screenshotTargetIntValue)
        
        // Documents
        emuCloseWithoutAskingButton.state = controller.closeWithoutAsking ? .on : .off
        emuEjectWithoutAskingButton.state = controller.ejectWithoutAsking ? .on : .off
        
        // Miscellaneous
        emuPauseInBackground.state = controller.pauseInBackground ? .on : .off
        emuAutoSnapshots.state = c64.takeAutoSnapshots() ? .on : .off
        emuSnapshotInterval.integerValue = c64.snapshotInterval()
        emuSnapshotInterval.isEnabled = c64.takeAutoSnapshots()
        
        // Media files
        func refresh(_ s: String, _ p: NSPopUpButton, _ b: NSButton, _ t: NSTextField) {
            
            let autoAction = controller.autoMountAction[s]?.rawValue ?? 0
            let autoType = controller.autoType[s] ?? false
            let autoTypeText = controller.autoTypeText[s] ?? ""
            p.selectItem(withTag: autoAction)
            b.isEnabled = autoAction != 0
            b.state = autoType ? .on : .off
            t.isEnabled = autoAction != 0 && autoType
            t.stringValue = autoTypeText
        }
        refresh("D64", emuD64Popup, emuD64AutoTypeButton, emuD64AutoTypeText)
        refresh("PRG", emuPrgPopup, emuPrgAutoTypeButton, emuPrgAutoTypeText)
        refresh("T64", emuT64Popup, emuT64AutoTypeButton, emuT64AutoTypeText)
        refresh("TAP", emuTapPopup, emuTapAutoTypeButton, emuTapAutoTypeText)
        refresh("CRT", emuCrtPopup, emuCrtAutoTypeButton, emuCrtAutoTypeText)

        // Power button
        emuPowerButton.isHidden = !bootable
    }

    //
    // Action methods (VC1541)
    //
    
    @IBAction func emuWarpLoadAction(_ sender: NSButton!) {
        
        proxy?.setWarpLoad(sender.state == .on)
        refresh()
    }
    
    @IBAction func emuDriveSoundsAction(_ sender: NSButton!) {
        
        myAppDelegate.pref.driveSounds = sender.state == .on
        refresh()
    }

    //
    // Action methods (Screenshots)
    //
    
    @IBAction func emuScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        myController?.screenshotSource = sender.selectedTag()
        refresh()
    }
    
    @IBAction func emuScreenshotTargetAction(_ sender: NSPopUpButton!) {
        
        myController?.screenshotTargetIntValue = sender.selectedTag()
        refresh()
    }

    //
    // Action methods (User Dialogs)
    //
    
    @IBAction func emuCloseWithoutAskingAction(_ sender: NSButton!) {
        
        myController?.closeWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    @IBAction func emuEjectWithoutAskingAction(_ sender: NSButton!) {
        
        myController?.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }

    //
    // Action methods (Miscellaneous)
    //
    
    @IBAction func emuPauseInBackgroundAction(_ sender: NSButton!) {
        
        myController?.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func emuAutoSnapshotAction(_ sender: NSButton!) {
        
        proxy?.setTakeAutoSnapshots(sender.state == .on)
        refresh()
    }
    
    @IBAction func emuSnapshotIntervalAction(_ sender: NSTextField!) {
        
        track("\(sender.integerValue)")
        if sender.integerValue > 0 {
            proxy?.setSnapshotInterval(sender.integerValue)
        } else {
            track("IGNORING")
        }
        refresh()
    }

    //
    // Action methods (Media files)
    //
    
    private func mediaFileType(_ tag: Int) -> String? {
        switch tag {
        case 0: return "D64"
        case 1: return "PRG"
        case 2: return "T64"
        case 3: return "TAP"
        case 4: return "CRT"
        default: return nil
        }
    }
    
    @IBAction func emuAutoMountAction(_ sender: NSPopUpButton!) {
        
        if let fileType = mediaFileType(sender.tag) {
            let action = AutoMountAction(rawValue: sender.selectedTag())
            myController?.autoMountAction[fileType] = action
            refresh()
        }
    }
    
    @IBAction func emuAutoTypeAction(_ sender: NSButton!) {
        
        if let fileType = mediaFileType(sender.tag) {
            myController?.autoType[fileType] = (sender.intValue == 0) ? false : true
            refresh()
        }
    }

    @IBAction func emuAutoTypeTextAction(_ sender: NSTextField!) {
        
        if let fileType = mediaFileType(sender.tag) {
            myController?.autoTypeText[fileType] = sender.stringValue
            refresh()
        }
    }
    
    @IBAction func emuPresetAction(_ sender: NSPopUpButton!) {
        
        track()
        assert(sender.selectedTag() == 0)
        
        UserDefaults.resetEmulatorUserDefaults()
        refresh()
     }
    
    @IBAction func emuDefaultsAction(_ sender: NSButton!) {
        
        track()
    }
}
