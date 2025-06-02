# Acoustic Entropy

[![license](https://img.shields.io/github/license/alejandrocs3719/acoustic-entropy.svg)](https://github.com/alejandrocs3719/acoustic-entropy/blob/main/LICENSE)

**Acoustic Entropy** is a modular C-based tool that supplements the Linux entropy pool using ambient noise captured from a microphone. It processes the audio to maximize randomness and injects the data into `/dev/random` using the `ioctl(RNDADDENTROPY)` syscall.

---

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Architecture](#architecture)
- [Evaluation](#evaluation)
- [Limitations](#limitations)
- [License](#license)

---

## 🚀 Features

- Modular architecture:
  - 🔊 Audio collection (via `arecord`)
  - ⚙️ Data whitening (SHA256, BLAKE2b, XOR)
  - 📈 Shannon entropy estimation
  - 🧪 Entropy injection into the Linux kernel
  - 🐍 Python script to gather test results with ent, dieharder and rng-test.
- Two modes:
  - Real-time recording
  - Offline processing of `.wav` files
- Configurable via CLI flags
- Compatible with ALSA and OpenSSL

---

## 🔧 Installation

### Dependencies

- GCC
- OpenSSL development libraries
- ALSA (`arecord`)
- Linux kernel headers

### Build

```bash
make
```

---

## 🧭 Usage

```bash
./acoustic_entropy [OPTIONS]
```

### Options

- `-e` : Use `.wav` files from `datos/` instead of recording
- `-w <methods>` : Whitening methods separated by commas (e.g. `sha256,xor`)

### Example

```bash
./acoustic_entropy -w sha256,blake2b
```

---

## 🐍 Python Script

Run the Python script to gather test results. Modify the script itself and specify the `base_dir` and `output_excel` directories to your liking.

**Note:** `base_dir` must be a directory containing multiple subdirectories. Each of these subdirectories should have the same structure as the `/datos` directory that is created by default by the C program.

---

## 🧱 Architecture

```
[ Microphone / .wav File ]
           ↓
 [ XOR-based Mixing ]
           ↓
 [ Whitening (SHA256, BLAKE2b, XOR) ]
           ↓
[ Entropy Estimation (Shannon) ]
           ↓
[ /dev/random Injection ]
```

- `main.c`: Program logic and CLI handling  
- `recolector.c`: Audio capture and mixing  
- `whitening.c`: Whitening transformations  
- `shannon.c`: Entropy calculation  
- `injector.c`: Kernel interaction

---

## ⚠️ Limitations

- Not suitable as the sole entropy source for high-security systems
- Performance can suffer on low-end devices due to hashing overhead
- Dieharder reports weak/failing cases even after whitening
- Entropy strongly influenced by mic quality and environmental noise

---

## 📄 License

AcousticEntropy - A Linux tool to inject audio-based randomness into the entropy pool.
Copyright (C) 2025  alejandrocs3719

This program is free software: you can redistribute it and/or modify  
it under the terms of the GNU General Public License as published by  
the Free Software Foundation, either version 3 of the License, or  
(at your option) any later version.

This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
GNU General Public License for more details.

You should have received a copy of the GNU General Public License  
along with this program.  If not, see <https://www.gnu.org/licenses/>.                    
