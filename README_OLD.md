# Conveyor System – Setup & Usage

## 1. Clone the Repository

```bash
git clone git@github.com:EpitechMscProPromo2026/T-IOT-911-NAN_1.git

cd T-IOT-911-NAN_1
```

## 2. Install Required Tools (Windows)

### Install Python 3.12

```bash
winget install Python.Python.3.12
```

### Install PlatformIO CLI
```bash
pip install platformio

pio --version
```

## 3. Build the Firmware
```bash
pio run -e m5stack-grey
```

The firmware will be generated in:

```bash
.pio/build/m5stack-grey/firmware.bin
```

## 4. Upload the Firmware to M5Stack

### Step 1 – Find the COM Port

```bash
pio device list
```

You should see something like:

```bash
COM5
Description: Silicon Labs CP210x USB to UART Bridge
```

### Step 2 – Upload
```bash
pio run -e m5stack-grey -t upload --upload-port COM5
```

Replace COM5 with your actual port.

## 5. Open Serial Monitor (Debugging)
To view logs from the ESP32:

```bash
pio run -t monitor -e m5stack-grey
```
## 6. Clean Build Files
```bash
pio run -e m5stack-grey -t clean
```

# How to setup Dolibarr app

## 1. Build docker container

```bash
docker compose up --build
```
## 2. Setup Dolibarr in browser

-> go to localhost:80

### Active http request 

Active module "API / Web services (serveur REST)"

Create a token Dolapikey in user card

### Products

Active module "Products"

Then create an extrafield : "options_default_target_warehouse_id"