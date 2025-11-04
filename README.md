
```text
//    ░██████████░█████████  ░██████████                                 
//    ░██        ░██     ░██ ░██                                         
//    ░██        ░██     ░██ ░██        ░██    ░██ ░█████████ ░█████████ 
//    ░█████████ ░█████████  ░█████████ ░██    ░██      ░███       ░███  
//    ░██        ░██   ░██   ░██        ░██    ░██    ░███       ░███    
//    ░██        ░██    ░██  ░██        ░██   ░███  ░███       ░███      
//    ░██        ░██     ░██ ░██         ░█████░██ ░█████████ ░█████████ 
//                                                                       
//                                                                       
//                                                                       
```

# FRFuzz: a Fuzzing FRamework

⚠️ Beta release: expect bugs and unfinished features. Use this software at your own risk.

## Quickstart
```bash
./grconsole list

./grconsole install binutils@2.22

cd /home/antonio/.frfuzz/projects/binutils/2.22/default
./grconsole fuzz AFL -p PROFILE_333

```

## Build Dependencies
- Libxml2
- libcurl
- SQLite
- OpenSSL
- libX11
- libmagic
- libyara
- libtar
- nlohmann-json3

## Runtime Dependencies
- AFL++
- LCOV
- colordiff
- wmctrl
- lm-sensors

## Installation

### Ubuntu 24.04:
```bash
sudo apt update
sudo apt install -y build-essential pkg-config libxml2-dev libssl-dev libsqlite3-dev libcurl4-openssl-dev libx11-dev libmagic-dev libyara-dev libtar-dev nlohmann-json3-dev lcov afl++ colordiff wmctrl lm-sensors

./configure
make -j$(nproc)
sudo make install
```

## Presentations

- **Ekoparty Security Conference 2025** – [View slides](https://github.com/antonio-morales/Ekoparty2025)

