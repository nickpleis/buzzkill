//
//  UartModuleViewController.swift
//  Basic Chat
//
//  Created by Trevor Beaton on 12/4/16.
//  Copyright © 2016 Vanguard Logic LLC. All rights reserved.
//





import UIKit
import CoreBluetooth

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
        outgoingData(inputText: inputTextField.text!)
        
    }
    
    @IBAction func button1(_ sender: UIButton) {
        
        outgoingData (inputText: "TCRB" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue))
    }
    
    @IBAction func button2(_ sender: UIButton) {
        outgoingData (inputText: "TFDR" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue))
        
    }
    @IBAction func button3(_ sender: UIButton) {
         outgoingData (inputText: "RBCY" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue))
    }
    @IBAction func button4(_ sender: Any) {
        outgoingData (inputText: "MOHW" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue))
    }
    @IBAction func button5(_ sender: UIButton) {
        outgoingData (inputText: "MOCC" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue))
    }
    
    @IBAction func button6(_ sender: UIButton) {
        outgoingData (inputText: "LRSC" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue))
    }
    
    @IBAction func button7(_ sender: UIButton) {
        outgoingData (inputText: "TKRD" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue))
    }

    @IBAction func button8(_ sender: UIButton) {
    }
    
    @IBAction func maxButton(_ sender: UIButton) {
        outgoingData(inputText: "MAXX" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue));
    }
    
    @IBAction func offButton(_ sender: UIButton) {
        outgoingData(inputText: "OFFF" + "," + String(speed) + "," + String(bright) + "," + String(red) + "," + String(green) + "," + String(blue));
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
    
    func outgoingData (inputText : String) {
        let appendString = "\n"
        
       
       //let inputText = inputTextField.text
        //var dataString = inputText + "\0"
        //var timeStampString = "\(CACurrentMediaTime())" + "\0"
       //print (timeStampString)
        
        
        
        var totalDevices = peripheralsArray.count
        var counter = 0
        var mapIndex = 0
        var executeInMs = totalDevices * 30; //we budget 30ms transmission time per device

        while counter < totalDevices
        {
            var sendText = inputText;
            sendText += "," + String(executeInMs);
            executeInMs -= 30;

            print ("Sending Data to peripheral:")
            blePeripheral = peripheralsArray[counter]
            print (counter)
            var mystring = blePeripheral?.identifier.uuidString
             mapIndex = mapArray.index(of: mystring)!
            print (mystring)
            print (mapIndex)
            txCharacteristic = txArray[mapIndex]
            print (blePeripheral)
            
            writeValue(data: sendText + "\0")
            print("                    ")
            print (txCharacteristic)
            print("                   ")
            //writeValue(data: timeStampString)
            counter = counter + 1
        }
        
        let myFont = UIFont(name: "Helvetica Neue", size: 15.0)
        let myAttributes1 = [NSAttributedStringKey.font: myFont!, NSAttributedStringKey.foregroundColor: UIColor.blue]
       
        
        
      
       
        
        let attribString = NSAttributedString(string: "[Outgoing]: " + inputText + appendString, attributes: myAttributes1)
        let newAsciiText = NSMutableAttributedString(attributedString: self.consoleAsciiText!)
        newAsciiText.append(attribString)
        
        
        
        consoleAsciiText = attribString
        debugLabel.text = inputText
        //baseTextView.attributedText = consoleAsciiText
        //erase what's in the text field
        //inputTextField.text = ""
        
    }
    
    // Write functions
    func writeValue(data: String){
        let valueString = (data as NSString).data(using: String.Encoding.utf8.rawValue)
        
     
     print ("TX Char ________________________________________________________")
        
        print (txCharacteristic)
        
       print ("TX Char ________________________________________________________")
             //change the "data" to valueString
        
            if let blePeripheral = blePeripheral{
                if let txCharacteristic = txCharacteristic {
                    blePeripheral.writeValue(valueString!, for: txCharacteristic, type: CBCharacteristicWriteType.withResponse)
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































