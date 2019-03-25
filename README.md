# PixLedDevice :sheep: :rainbow:
ESP32 program embedded in PixLed strip modules.

Those modules can be used with ![PixLedServer](https://github.com/PaulBreugnot/PixLedServer) and ![PixLed Androïd](https://github.com/PaulBreugnot/PixLedAndroid) to build an awesome IoT LED strip lighting system!

# Supported devices

- [x] Strips
- [ ] Panels
- [ ] Others

# Supported leds

- [x] WS2812
- [ ] SK6812
- [ ] SK6812 RGBW

# Prerequisite
## Install the ESP-IDF
In order to build and flash the PixLedDevice firmware to your ESP32, you need to install and setup the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/) Toolchain.

To do so, you can follow the [official ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html#step-1-set-up-the-toolchain).

## Get the PixLedDevice firmware
Go to the directory where you want to download the firmware, and run :
```
git clone https://github.com/PixLed/PixLedDevice-ESP32/
```

## Setup
Now, run 
```
cd PixLedDevice-ESP32
make menuconfig
```
You should see a menu like this one :
![MenuconfigHome](https://github.com/PixLed/PixLedDevice-ESP32/blob/master/docs/pictures/menuconfig_home.png)

### ESP-IDF config
Firstly, go to `SDK tool configuration` and check that the Python 2 interpreter specified correspond to your installation, depending on your OS.

Come back, and in `Serial flasher config` you can set up the port on which your ESP32 is connected.

### PixLed config
Navigate to `Module Configuration` :
![ModuleConfiguration](https://github.com/PixLed/PixLedDevice-ESP32/blob/master/docs/pictures/ModuleConfiguration.png)

**Hardware config**
1. **Led Pin** : The GPIO on which your leds are connected
2. **Led Count** : Number of leds in your device.
3. **Blink GPIO** : The GPIO of a led indicator that blink when connecting for example. Even if it's not mandatory, this should be the built-in LED. So the default value is 2 there, but this might change depending on your ESP32 dev-kit.
4. **Enables mode handler** : Enables advance modes features, that will be described later. You should let this uncheck for now.

**WiFi config**

Connection information to your wifi network. Obviously, your PixLedServer must also be accessible from this network.

5. **WiFi SSID** : ssid
6. **WiFi PASS** : password

**Server config**

Those parameters are optionnal if you use mDNS. (See the [PixLedServer doc](https://github.com/PixLed/PixLedServer#avahi))
But even if you use mDNS, you can specify them as a fallback in case of trouble : **at each boot**, those parameters will be saved, and they will be overwritten **if a PixLedServer or a MQTT broker is found using mDNS**.

5. **Server IP** : The IP of your [PixLedServer](https://github.com/PixLed/PixLedServer)
6. **Server port** : The port of your PixLedServer. (default : 8080)
7. **MQTT Broker IP** : IP of the device that host your MQTT broker (See the [PixLedServer doc](https://github.com/PixLed/PixLedServer#mosquitto))
8. **MQTT Broker port** : port of the MQTT broker. (default : 1883)

**Note :** Even if server and mqtt broker IPs can be configured independently, both currently must be the same due to server limitations (the PixLedServer and the broker must be on the same host)

### Save config
Once everything is set up, go to save using the right arrow, save and then `Exit` the menu.

## Build
Run
```
make all
```
to build the project.
Then, run
```
make flash
```
to upload the code to your ESP32. You can then run `make monitor` to check useful logs and check that everything is ok. (You can also run directly `make flash monitor`)

As extra information : 
* Normally, the led strip color should switch to light white at boot, and power off once the module as successfully connected to the server.
* The built-in LED (or other, specified by `Blink GPIO`) should blink until the module is connected to your WiFi network.

# You're done!
Now you can set up all the devices that you want to include in your installation with the same method, just running `make flash` after connecting your new modules. Don't forget to run `make menuconfig` again if you need to change the led count or other parameters.

# App and modules
If not done yet, you can now install your ![PixLed Androïd app](https://github.com/PaulBreugnot/PixLedAndroid) and set up your ![PixLedServer](https://github.com/PaulBreugnot/PixLedServer) to control your devices! :sheep: :rainbow:

# LICENSE
This software and all the PixLed tools are released under the [GNU General Public License v3.0](https://github.com/PixLed/PixLedDevice-ESP32/blob/master/LICENSE).
