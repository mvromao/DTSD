// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char DEVICE_LOGIN_NAME[]  = "1a72ce45-3a79-40e8-aede-727605e8a621";

const char SSID[]               = SECRET_SSID;    // Network SSID (name)
const char PASS[]               = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)
const char DEVICE_KEY[]  = SECRET_DEVICE_KEY;    // Secret device password

void onLuzQuartoDireitaChange();
void onLuzQuartoEsquerdaChange();
void onLuzQuartoTrasChange();
void onLuzSalaChange();

CloudLight luz_quarto_direita;
CloudLight luz_quarto_esquerda;
CloudLight luz_quarto_tras;
CloudLight luz_sala;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(luz_quarto_direita, READWRITE, ON_CHANGE, onLuzQuartoDireitaChange);
  ArduinoCloud.addProperty(luz_quarto_esquerda, READWRITE, ON_CHANGE, onLuzQuartoEsquerdaChange);
  ArduinoCloud.addProperty(luz_quarto_tras, READWRITE, ON_CHANGE, onLuzQuartoTrasChange);
  ArduinoCloud.addProperty(luz_sala, READWRITE, ON_CHANGE, onLuzSalaChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
