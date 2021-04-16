#pragma once
#include <Arduino.h>
#include "SimpleBBSerial.h"

class XbeeCell
{
public:
    XbeeCell(SimpleBBSerial *ser);
    bool enterCommandMode();
    bool printHardwareInfo();
    String sendAT(String command, String param);
    String sendAT(String command);
    bool exitCommandMode();
    bool printSIMInfo();
    bool setAccessPointName(String ap_name);
    bool setAccessPointServer(String server, unsigned int port);
    bool setAccessPointNameAndServerConnect(String ap_name, String server, unsigned int port);
    bool setAccessPointNameAndServerConnect();
    bool setTextDelimiterCR();
    bool applyChanges();
    String checkAssociation();
    bool shutDown();
    bool setupSleep(bool turn_on);
    String checkAssociationCmd();
    bool setDeviceOptionsNoUSA();
    bool getNetworkInfo();
    String sendData(String data);
    void removeNulls(String &s);
    bool checkIfConnected();
    bool reconnectIfRequired(String ap_name, String server, unsigned int port);
    bool reconnectIfRequired();
    bool checkIfConnectedPoll();
    bool setSleepPins();
    bool saveToMemory();
    void setCachedNetworkInfo(String ap_name, String server, unsigned int port);
    bool enterCommandSetupSleep();
    bool isModuleSetToSleep();
    bool restoreDefaults();
    bool checkNotBlankOK(String s);
private:
    void pd(String s);
    String convertPortToHex(unsigned int port);
    bool checkOK();
    long int hexStringToInt(String input);
    String hexStringToIntString(String input);
    SimpleBBSerial *serial;
    String ap_name_cached;
    String server_cached;
    unsigned int port_cached;
}; // end

