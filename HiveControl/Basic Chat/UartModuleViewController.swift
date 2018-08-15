//
//  UartModuleViewController.swift
//  Basic Chat
//
//  Created by Trevor Beaton on 12/4/16.
//  Copyright © 2016 Vanguard Logic LLC. All rights reserved.
//

import UIKit
import CoreBluetooth

var modeMap = [
    "MOST" : 0,   // FX_MODE_STATIC
    "BLCL" : 1,   // FX_MODE_BLINK
    "MOBR" : 2,   // FX_MODE_BREATH
    "CLWI" : 3,   // FX_MODE_CLWI
    "MOIW" : 4,   // FX_MODE_COLOR_WIPE_INV
    "MORW" : 5,   // FX_MODE_COLOR_WIPE_REV
    "IRWP" : 6,   // FX_MODE_COLOR_WIPE_REV_INV
    "RAWP" : 7,   // FX_MODE_COLOR_WIPE_RANDOM
    "RACL" : 8,   // FX_MODE_RANDOM_COLOR
    "MOSD" : 9,   // FX_MODE_SINGLE_DYNAMIC
    "MOMD" : 10,  // FX_MODE_MULTI_DYNAMIC
    "MORB" : 11,  // FX_MODE_RAINBOW
    "RBCY" : 12,  // FX_MODE_RAINBOW_CYCLE
    "MOMS" : 13,  // FX_MODE_SCAN
    "MODS" : 14,  // FX_MODE_DUAL_SCAN
    "MOFD" : 15,  // FX_MODE_FADE
    "MOTC" : 16,  // FX_MODE_THEATER_CHASE
    "TCRB" : 17,  // FX_MODE_THEATER_CHASE_RAINBOW
    "MORL" : 18,  // FX_MODE_RUNNING_LIGHTS
    "MOTK" : 19,  // FX_MODE_TWINKLE
    "TKRD" : 20,  // FX_MODE_TWINKLE_RANDOM
    "TKFD" : 21,  // FX_MODE_TWINKLE_FADE
    "TFDR" : 22,  // FX_MODE_TWINKLE_FADE_RANDOM
    "MOSP" : 23,  // FX_MODE_SPARKLE
    "FLSP" : 24,  // FX_MODE_FLASH_SPARKLE
    "HYSP" : 25,  // FX_MODE_HYPER_SPARKLE
    "MSTR" : 26,  // FX_MODE_STROBE
    "STRB" : 27,  // FX_MODE_STROBE_RAINBOW
    "STMU" : 28,  // FX_MODE_MULTI_STROBE
    "BLRB" : 29,  // FX_MODE_BLINK_RAINBOW
    "CHWT" : 30,  // FX_MODE_CHASE_WHITE
    "CHCL" : 31,  // FX_MODE_CHASE_COLOR
    "CHRD" : 32,  // FX_MODE_CHASE_RANDOM
    "CHRB" : 33,  // FX_MODE_CHASE_RAINBOW
    "CHFL" : 34,  // FX_MODE_CHASE_FLASH
    "CHFR" : 35,  // FX_MODE_CHASE_FLASH_RANDOM
    "CHRW" : 36,  // FX_MODE_CHASE_RAINBOW_WHITE
    "CHBL" : 37,  // FX_MODE_CHASE_BLACKOUT
    "CHBR" : 38,  // FX_MODE_CHASE_BLACKOUT_RAINBOW
    "CLSR" : 39,  // FX_MODE_COLOR_SWEEP_RANDOM
    "MORC" : 40,  // FX_MODE_RUNNING_COLOR
    "RNRB" : 41,  // FX_MODE_RUNNING_RED_BLUE
    "RNRD" : 42,  // FX_MODE_RUNNING_RANDOM
    "LRSC" : 43,  // FX_MODE_LARSON_SCANNER
    "MOCT" : 44,  // FX_MODE_COMET
    "MOFW" : 45,  // FX_MODE_FIREWORKS
    "FWRD" : 46,  // FX_MODE_FIREWORKS_RANDOM
    "CHRS" : 47,  // FX_MODE_MERRY_CHRISTMAS
    "MOFF" : 48,  // FX_MODE_FIRE_FLICKER
    "FFST" : 49,  // FX_MODE_FIRE_FLICKER_SOFT
    "FFIN" : 50,  // FX_MODE_FIRE_FLICKER_INTENSE
    "MOCC" : 51,  // FX_MODE_CIRCUS_COMBUSTUS
    "MOHW" : 52,  // FX_MODE_HALLOWEEN
    "BICH" : 53,  // FX_MODE_BICOLOR_CHASE
    "TRCH" : 54,  // FX_MODE_TRICOLOR_CHASE
    "MOIC" : 55,  // FX_MODE_ICU
    "OFFF" : 255, // CUSTOM_MODE_OFF
    "MAXX" : 254, // CUSTOM_MODE_MAX
    "BEEE" : 253  // CUSTOM_MODE_BEE
]

class UartModuleViewController: UIViewController, CBPeripheralManagerDelegate, UITextViewDelegate, UITextFieldDelegate {
    
    //UI
    //@IBOutlet weak var baseTextView: UITextView!
    @IBOutlet weak var sendButton: UIButton!
    @IBOutlet weak var inputTextField: UITextField!
    @IBOutlet weak var scrollView: UIScrollView!
    @IBOutlet weak var switchUI: UISwitch!
    //Data
    @IBOutlet weak var debugLabel: UILabel!
    var peripheralManager: CBPeripheralManager?
    var peripheral: CBPeripheral!
    var peripheralsArray: [CBPeripheral] = []
    var numOfPeripherals  = 1
    var speed = 50
    var bright = 50
    var red = 255
    var green = 0
    var blue = 0
    var selectedColor = UIColor (hue: 1, saturation: 1, brightness: 1, alpha: 1)
    
    private var consoleAsciiText:NSAttributedString? = NSAttributedString(string: "")
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.navigationItem.backBarButtonItem = UIBarButtonItem(title:"Back", style:.plain, target:nil, action:nil)
        /*self.baseTextView.delegate = self
        self.inputTextField.delegate = self
        //Base text view setup
        self.baseTextView.layer.borderWidth = 3.0
        self.baseTextView.layer.borderColor = UIColor.blue.cgColor
        self.baseTextView.layer.cornerRadius = 3.0
        self.baseTextView.text = ""*/
        //Input Text Field setup
        //self.inputTextField.layer.borderWidth = 2.0
        //self.inputTextField.layer.borderColor = UIColor.blue.cgColor
        //elf.inputTextField.layer.cornerRadius = 3.0
        //Create and start the peripheral manager
        peripheralManager = CBPeripheralManager(delegate: self, queue: nil)
        //-Notification for updating the text view with incoming text
        updateIncomingData()
    }
    
    override func viewDidAppear(_ animated: Bool) {
        //self.baseTextView.text = ""
        
        
    }
    
    override func viewDidDisappear(_ animated: Bool) {
        // peripheralManager?.stopAdvertising()
        // self.peripheralManager = nil
        super.viewDidDisappear(animated)
        NotificationCenter.default.removeObserver(self)
        
    }
    
    func updateIncomingData () {
        NotificationCenter.default.addObserver(forName: NSNotification.Name(rawValue: "Notify"), object: nil , queue: nil){
            notification in
            let appendString = "\n"
            let myFont = UIFont(name: "Helvetica Neue", size: 15.0)
            let myAttributes2 = [NSAttributedStringKey.font: myFont!, NSAttributedStringKey.foregroundColor: UIColor.red]
            let attribString = NSAttributedString(string: "[Incoming]: " + (characteristicASCIIValue as String) + appendString, attributes: myAttributes2)
            let newAsciiText = NSMutableAttributedString(attributedString: self.consoleAsciiText!)
            //self.baseTextView.attributedText = NSAttributedString(string: characteristicASCIIValue as String , attributes: myAttributes2)
            
            newAsciiText.append(attribString)
            
            self.consoleAsciiText = newAsciiText
           // self.baseTextView.attributedText = self.consoleAsciiText
            
        }
    }
    
    @IBAction func clickSendAction(_ sender: AnyObject) {
        outgoingData(mode: inputTextField.text!)
        
    }
    
    @IBAction func button1(_ sender: UIButton) {
        outgoingData (mode: "TCRB", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    
    @IBAction func button2(_ sender: UIButton) {
        outgoingData (mode: "TFDR", speed : speed, brightness : bright, red : red, green : green, blue : blue);
        
    }
    @IBAction func button3(_ sender: UIButton) {
         outgoingData (mode: "RBCY", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    @IBAction func button4(_ sender: Any) {
        outgoingData (mode: "MOHW", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    @IBAction func button5(_ sender: UIButton) {
        outgoingData (mode: "MOCC", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    
    @IBAction func button6(_ sender: UIButton) {
        outgoingData (mode: "LRSC", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    
    @IBAction func button7(_ sender: UIButton) {
        outgoingData (mode: "TKRD", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }

    @IBAction func button8(_ sender: UIButton) {
        outgoingData(mode: "BEEE", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    
    @IBAction func maxButton(_ sender: UIButton) {
        outgoingData(mode: "MAXX", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    
    @IBAction func offButton(_ sender: UIButton) {
        outgoingData(mode: "OFFF", speed : speed, brightness : bright, red : red, green : green, blue : blue);
    }
    
    @IBAction func speedSlider(_ sender: UISlider) {
        speed = Int(sender.value)
    }
    
    @IBAction func colorSlider(_ sender: GradientSlider) {
        selectedColor = UIColor(hue: sender.value, saturation: 1.0, brightness: 1.0, alpha: 1.0)
        sender.thumbColor = selectedColor
       
        var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
        selectedColor.getRed(&r, green: &g, blue: &b, alpha: &a)
        
        red = (Int(r*255))
        blue = (Int(b*255))
        green = (Int(g*255))
        print("red: \(red), green: \(green), blue: \(blue)")
    }
    

    @IBAction func brightSlider(_ sender: UISlider) {
        bright = Int(sender.value)
    }
    
    func outgoingData (mode : String, speed : Int = 50, brightness: Int = 50, red: Int = 255, green: Int = 0, blue : Int = 0) {
        let appendString = "\n"

        var totalDevices = peripheralsArray.count
        var counter = 0
        var mapIndex = 0
        var executeInMs:UInt16 = UInt16(totalDevices) * 30; //we budget 30ms transmission time per device
        var displayString = ""

        if let modeId = modeMap[mode] {
            displayString = "Mode: " + mode;
            displayString += ", Speed: " + String(speed);
            displayString += ", Bright: " + String(bright);
            displayString += ", Red: " + String(red);
            displayString += ", Blue: " + String(blue);
            displayString += ", Green: " + String(green);

            while counter < totalDevices
            {
                // We send an 8 byte binary data stream that looks like:
                // 1 byte: mode
                // 1 byte: speed
                // 1 byte: brightness
                // 1 byte: red
                // 1 byte: green
                // 1 byte: blue
                // 2 bytes: milliseconds into the future to execute at
                
                var writeData:Data = Data();
                writeData.append(UInt8(modeId));
                writeData.append(UInt8(speed));
                writeData.append(UInt8(brightness));
                writeData.append(UInt8(red));
                writeData.append(UInt8(green));
                writeData.append(UInt8(blue));
                writeData.append(UnsafeBufferPointer(start: &executeInMs, count: 1));
                
                print ("Sending Data to peripheral:")
                blePeripheral = peripheralsArray[counter]
                print (counter)
                var mystring = blePeripheral?.identifier.uuidString
                mapIndex = mapArray.index(of: mystring)!
                print (mystring)
                print (mapIndex)
                txCharacteristic = txArray[mapIndex]
                print (blePeripheral)
                
                writeValue(data: writeData);
                print("                    ")
                print (txCharacteristic)
                print("                   ")
                //writeValue(data: timeStampString)
                counter = counter + 1
            }
        } else {
            displayString = "ERROR: Unknown mode " + mode;
        }
        
        let myFont = UIFont(name: "Helvetica Neue", size: 15.0)
        let myAttributes1 = [NSAttributedStringKey.font: myFont!, NSAttributedStringKey.foregroundColor: UIColor.blue]

        let attribString = NSAttributedString(string: "[Outgoing]: " + displayString + appendString, attributes: myAttributes1)
        let newAsciiText = NSMutableAttributedString(attributedString: self.consoleAsciiText!)
        newAsciiText.append(attribString)

        consoleAsciiText = attribString
        debugLabel.text = displayString
    }
    
    // Write functions
    func writeValue(data: Data){
        print ("TX Char ________________________________________________________")
        print (txCharacteristic)
        print ("TX Char ________________________________________________________")
        
        if let blePeripheral = blePeripheral{
            if let txCharacteristic = txCharacteristic {
                blePeripheral.writeValue(data, for: txCharacteristic, type: CBCharacteristicWriteType.withResponse)
            }
        }
    }
    
    func writeCharacteristic(val: Int8){
        var val = val
        let ns = NSData(bytes: &val, length: MemoryLayout<Int8>.size)
        blePeripheral!.writeValue(ns as Data, for: txCharacteristic!, type: CBCharacteristicWriteType.withResponse)
    }

    //MARK: UITextViewDelegate methods
    func textViewShouldBeginEditing(_ textView: UITextView) -> Bool {
        /*if textView === baseTextView {
            //tapping on consoleview dismisses keyboard
            inputTextField.resignFirstResponder()
            return false
        }*/
        return true
    }
    
    func textFieldDidBeginEditing(_ textField: UITextField) {
        scrollView.setContentOffset(CGPoint(x:0, y:250), animated: true)
    }
    
    func textFieldDidEndEditing(_ textField: UITextField) {
        scrollView.setContentOffset(CGPoint(x:0, y:0), animated: true)
    }
    
    func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
        if peripheral.state == .poweredOn {
            return
        }
        print("Peripheral manager is running")
    }
    
    //Check when someone subscribe to our characteristic, start sending the data
    func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didSubscribeTo characteristic: CBCharacteristic) {
        print("Device subscribe to characteristic")
    }
    
    /*//This on/off switch sends a value of 1 and 0 to the Arduino
    //This can be used as a switch or any thing you'd like
    @IBAction func switchAction(_ sender: Any) {
        if switchUI.isOn {
            print("On ")
            writeCharacteristic(val: 1)
        }
        else
        {
            print("Off")
            writeCharacteristic(val: 0)
            print(writeCharacteristic)
        }
    }*/
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        self.view.endEditing(true)
    }
    
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        textField.resignFirstResponder()
        //outgoingData(inputText: "TEST Data")
        return(true)
    }
    
    func peripheralManagerDidStartAdvertising(_ peripheral: CBPeripheralManager, error: Error?) {
        if let error = error {
            print("\(error)")
            return
        }
    }
}































