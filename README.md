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

This project is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.html).

You are free to use, modify, and distribute this software under the terms of the GPLv3. See the `LICENSE` file for more details.
