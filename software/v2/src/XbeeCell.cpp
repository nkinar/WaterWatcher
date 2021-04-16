#include <Arduino.h>
#include "XbeeCell.h"
#include "cell_responses.h"


/* 
NOTES:
For use with Hologram.io
https://support.hologram.io/hc/en-us/articles/360035697013
*/

const uint32_t TIMEOUT_CYCLES_QUICK = 10000;
const uint32_t TIMEOUT_CYCLES_DEFAULT = 1000000; 
static const char OK_STR[] = "OK\r";
static const char ERROR_STR[] = "ERROR\r";
static const size_t OK_CHARS = 3;
static const char CR[] = "\r";
const uint32_t MAX_CHARS_RESP_DEFAULT = 100;
static const char NO_STR[] = "";
static const char AT[] = "AT";
const uint32_t ASSOC_RETRY = 60;
const uint32_t ASSOC_WAIT_MS = 1000;
static const char CRLF[] = "\r\n";
static const char LF[] = "\n";
static uint32_t TIMEOUT_CYCLES_RESP = 10;
static const char RESP_CONNECTED_CODE[] = "0";
static const uint32_t SIM_RETRIES = 10; 
static const uint32_t SIM_WAIT_RETRIES_MS = 1000;

XbeeCell::XbeeCell(SimpleBBSerial *ser)
{
    serial = ser;
} // end


void XbeeCell::pd(String s)
{
    #ifdef XBEE_CELL_DEBUG
        Serial.println(s);
    #endif
} // end


bool XbeeCell::checkOK()
{
    uint32_t TIMEOUT_2SEC = 2000000;
     String resp = serial->receiveString(TIMEOUT_2SEC, OK_CHARS, CR);
     if (resp == OK_STR) return true;
     return false;
} // end


bool XbeeCell::enterCommandMode()
{
    delay(1000);
    serial->sendString("+++"); 
    // NOTE that sometimes OK is returned before 1 s guard time is up, so just check for the OK
    return checkOK();
} // end


String XbeeCell::sendAT(String command)
{
    return sendAT(command, NO_STR);
} // end


String XbeeCell::sendAT(String command, String param)
{
    String sout = String(AT) + command;
    if(param.length() != 0) sout += param;
    sout += CR;
    serial->sendString(sout);
    String resp = serial->receiveString(TIMEOUT_CYCLES_DEFAULT, MAX_CHARS_RESP_DEFAULT, CR);
    return resp;
} // end

bool XbeeCell::exitCommandMode()
{
    serial->sendString(String(AT)+String("CN\r")); 
    return checkOK();
} // end 

long int XbeeCell::hexStringToInt(String input)
{
    input.trim();
    return strtoul(input.c_str(), NULL, 16);
} // end


String XbeeCell::hexStringToIntString(String input)
{
    return String(hexStringToInt(input));
} // end


bool XbeeCell::printHardwareInfo()
{
    pd("Entering command mode"); 
    bool rv = enterCommandMode();
    if(!rv) return false;

    String firmware_ver, hardware, hardware_ser;
    for (uint32_t k = 0; k < SIM_RETRIES; k++)
    {
        firmware_ver = sendAT("VR");
        hardware = sendAT("HV");
        hardware_ser = sendAT("HS");
        bool check = checkNotBlankOK(firmware_ver) && checkNotBlankOK(hardware) && checkNotBlankOK(hardware_ser);
        if(check) break;
        delay(SIM_WAIT_RETRIES_MS); 
    }
    Serial.println("Firmware: " + firmware_ver); 
    Serial.println("Hardware: " + hexStringToIntString(hardware)); 
    Serial.println("Hardware Series: " + hexStringToIntString(hardware_ser));

    return exitCommandMode();
} // end


bool XbeeCell::checkNotBlankOK(String s)
{
    s.trim();
    bool check = (s!="") && (s!="OK");
    return check;
} // end


bool XbeeCell::printSIMInfo()
{
    bool rv = enterCommandMode();
    if(!rv) return false;

    String ICCID, IMEI, II;
    for (uint32_t k = 0; k < SIM_RETRIES; k++)
    {
        ICCID = sendAT("S#");
        IMEI = sendAT("IM");
        II = sendAT("II");
        bool check = checkNotBlankOK(ICCID) && checkNotBlankOK(IMEI) && checkNotBlankOK(II);
        if(check) break;
        delay(SIM_WAIT_RETRIES_MS); 
    }
    Serial.println("ICCID: " + ICCID);
    Serial.println("IMEI: " + IMEI);
    Serial.println("II: " + II);
    
    return exitCommandMode();

}  // end


bool XbeeCell::setAccessPointName(String ap_name)
{
    // Command mode must be entered before this command is sent
   String resp = sendAT("AN", ap_name);
   if (resp==OK_STR) return true;
   return false;
} // end


bool XbeeCell::setAccessPointServer(String server, unsigned int port)
{
    // Command mode must be entered before this command is sent
    pd("Server: " + server);
    String resp = sendAT("DL", server);
    if (resp!=OK_STR) return false;

    String hex = convertPortToHex(port);    
    pd("Port (hex): " + hex);
    resp = sendAT("DE", hex);
    if (resp!=OK_STR) return false;

    return true;
} // end


bool XbeeCell::setTextDelimiterCR()
{
    // Command mode must be entered before this command is sent
    String resp = sendAT("TD", "0D");  // set the text delimiter to carriage return
    if (resp!=OK_STR) return false;
    return true;
} // end


bool XbeeCell::setSleepPins()
{
    // Command mode must be entered before this command is sent

    // D8 needs to be set to the sleep mode function
    String resp = sendAT("D8", "1");
    if (resp!=OK_STR) return false;

    // Turn on the sleep function
    resp = sendAT("SM", "1");
    if (resp!=OK_STR) return false;

    // command was successful
    return true;

} // end


// the module must be on before this is called
bool  XbeeCell::isModuleSetToSleep()
{
    bool rv = enterCommandMode();
    if (!rv) return false;

    String resp = sendAT("D8");
    resp.trim();
    if(resp != "1") return false;

    resp = sendAT("SM");
    resp.trim();
    if(resp != "1") return false;

    return true;
} // end


bool XbeeCell::setDeviceOptionsNoUSA()
{
    // Turn off the AT&T option since the device takes a long time to connect outside
    // of the USA
    String resp = sendAT("DO", "41");
    if (resp!=OK_STR) return false;
    return true;
} // end


bool XbeeCell::applyChanges()
{
    // Command mode must be entered before this command is sent
    String resp = sendAT("AC");
    if (resp!=OK_STR) return false;
    return true;
} // end


String XbeeCell::checkAssociation()
{
    String resp = sendAT("AI");
    resp.trim();
    return resp;
} // end 


String XbeeCell::checkAssociationCmd()
{
    bool rv = enterCommandMode();
    if(!rv) return "";
    return checkAssociation();
} // end


// This function must be only called during development to shut down the module.
// DO NOT call for production since the module may not shut down correctly.
bool XbeeCell::shutDown()
{
    bool rv = enterCommandMode();
    if(!rv) return false;

    String resp = sendAT("SD", "0"); // shutdown the module
    delay(3000);  // wait until polling to give the module a chance to shut down

    bool flag = false;
    for(uint32_t k = 0; k < ASSOC_RETRY; k++)
    {
            String assoc = checkAssociation();
            assoc.trim();
            if(assoc=="2D") {flag = true; break; }
            delay(ASSOC_WAIT_MS);
    }

    rv = exitCommandMode();
    if(!rv) return false;

    return flag;
} // end


bool XbeeCell::setAccessPointNameAndServerConnect()
{
    if(ap_name_cached.length()==0) return false;
    return setAccessPointNameAndServerConnect(ap_name_cached, server_cached, port_cached);
} // end


bool XbeeCell::enterCommandSetupSleep()
{
    bool rv = enterCommandMode(); 
    if(!rv) return false;

    rv = setSleepPins();
    return rv;
} // end


String XbeeCell::convertPortToHex(unsigned int port)
{
    char s[5];
    sprintf(s, "%X", port);
    String out = String(s);
    return out;
} // end


// This function sets the device up
bool XbeeCell::setAccessPointNameAndServerConnect(String ap_name, String server, unsigned int port)
{
    pd("Entering command mode");
    bool rv = enterCommandMode();
    if(!rv) return false;

    pd("Set the name of the AP");
    pd("APN:" + ap_name); 
    rv = setAccessPointName(ap_name);
    if(!rv) return false;

    pd("Set the server");
    rv = setAccessPointServer(server, port);
    if(!rv) return false;

    pd("Set the text delimiter");
    rv = setTextDelimiterCR();
    if(!rv) return false;

    pd("apply changes");
    rv = applyChanges();
    if(!rv) return false;

    pd("Checking if connected..."); 
    bool flag = false;
    flag = checkIfConnectedPoll(); 

    pd("Exiting command mode");
    rv = exitCommandMode();
    if(!rv) return false;

    if(!flag) return false;    
    return true; // module is connected
} // end


bool XbeeCell::getNetworkInfo()
{
    bool rv = enterCommandMode();
    if(!rv) return false;

    String number = sendAT("PH");
    Serial.println("Phone number: " + number);

    String oper = sendAT("MN");
    Serial.println("Operator: " + oper);

    String dB = sendAT("DB", "0");
    Serial.println("dB: -" + hexStringToIntString(dB) + " dBm");

    String dT = sendAT("DT", "1");  // read the time in ISO 8601 format.
    Serial.println("dT: " + dT);  

    String my = sendAT("MY");      // IP Address
    Serial.println("IP: " + my);  

    rv = exitCommandMode();         // exit the command mode
    if(!rv) return false;

    return true;
} // end


void XbeeCell::removeNulls(String &s)
{
    unsigned int n = s.length();
    for(unsigned int k = 0; k < n; k++)
    {
        if (s.charAt(k) == 0x00) s.setCharAt(k, ' ');
    }
} // end 


// This cannot be read in command mode
String XbeeCell::sendData(String data)
{
    serial->sendString(data);
    serial->sendString(String(CR));
    for(uint32_t k = 0; k < TIMEOUT_CYCLES_RESP; k++)
    {
        String resp = serial->receiveString(TIMEOUT_CYCLES_DEFAULT, MAX_CHARS_RESP_DEFAULT, CR);
        removeNulls(resp);
        resp.trim();
        if (resp.length() != 0) return resp;
    }
    return String("ERROR"); // return something to indicate that the string could not be read
} // end 


// ENTER COMMAND MODE BEFORE CALLING THIS FUNCTION
bool XbeeCell::checkIfConnected()
{
    String aresp = checkAssociation();
    pd("resp = " + aresp);
    if(aresp==String(RESP_CONNECTED_CODE)) return true;
    return false;
} // end


// ENTER COMMAND MODE BEFORE CALLING THIS FUNCTION
bool XbeeCell::checkIfConnectedPoll()
{
    for(uint32_t k = 0; k < ASSOC_RETRY; k++)
    {
        if (checkIfConnected()) return true;
        delay(ASSOC_WAIT_MS);
    }
    return false;
} // end


bool XbeeCell::reconnectIfRequired()
 {
     if (ap_name_cached.length() == 0) return false;
     return reconnectIfRequired(ap_name_cached, server_cached, port_cached);
 } // end


bool XbeeCell::reconnectIfRequired(String ap_name, String server, unsigned int port)
{
    // enter command mode
    bool rv = enterCommandMode();
     if (rv == false) return false;

    // check if the device is connected to the network
    rv = checkIfConnectedPoll();
    if (rv == true) 
    {
        return exitCommandMode();
    }

    // by here, we know that the device is not connected, so we need to reconnect by setting it up again
    return setAccessPointNameAndServerConnect(ap_name, server, port);
} // end


// Save the state of the modem to memory
bool XbeeCell::saveToMemory()
{
    pd("Entering command mode");
    bool rv = enterCommandMode();
    if (rv == false) return false;
    
    pd("Sending write command...");
    String resp = sendAT("WR");
    if (resp!=OK_STR) return false;

    pd("DONE write to memory");
    return true;
} // end


// This function is used to set the cached network information before connecting
void XbeeCell::setCachedNetworkInfo(String ap_name, String server, unsigned int port)
{
    ap_name_cached = ap_name;
    server_cached = server;
    port_cached = port;
} // end


// This command does not save the state, so the WR command must be issued
// to write the state into flash memory.
bool XbeeCell::restoreDefaults()
{
    bool rv = enterCommandMode();
    if (rv == false) return false;

    String resp = sendAT("RE");
    if (resp!=OK_STR) return false;

    Serial.println("Defaults have been restored, but are not saved. Use WR command");
    return true;
} // end


