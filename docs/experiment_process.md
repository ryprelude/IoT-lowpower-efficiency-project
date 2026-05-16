# Experiment Process
## Energy-Efficient Security in Low-Power IoT Networks

---

## 1. Environment Setup

### 1-1. Prerequisites
- Install Docker Desktop
- Install VcXsrv (Windows X Server)
- Clone Contiki-NG

```bash
git clone https://github.com/contiki-ng/contiki-ng
cd contiki-ng/tools/cooja
git clone https://github.com/contiki-ng/cooja.git .
```

### 1-2. Run Docker Container

```bash
docker run --privileged --sysctl net.ipv6.conf.all.disable_ipv6=0 \
  -e DISPLAY=host.docker.internal:0 -it --rm \
  -v C:/Users/{username}/contiki-ng:/home/user/contiki-ng \
  contiker/contiki-ng
```

### 1-3. Run Cooja

```bash
cd contiki-ng/tools/cooja
sed -i 's/\r//' gradlew
./gradlew run
```

---

## 2. IEEE 802.15.4 + 6LoWPAN Environment Setup

### 2-1. Create Simulation
1. File → New Simulation → Create

### 2-2. Add UDP Server Node
1. Motes → Add motes → Create new mote type → Sky mote
2. Browse → `/home/user/contiki-ng/examples/rpl-udp/udp-server.c`
3. Compile → Create → Add Motes (1 node)

### 2-3. Add UDP Client Node
1. Motes → Add motes → Create new mote type → Sky mote
2. Browse → `/home/user/contiki-ng/examples/rpl-udp/udp-client.c`
3. Compile → Create → Add Motes (1 node)

### 2-4. Verify Communication
- Click Start/Pause
- Place nodes close to each other
- Confirm the following logs in Mote output
```
ID:2  Sending request 'hello N' to ...
ID:1  Received request 'hello N' from ...
ID:1  Sending response.
ID:2  Received response 'hello N' from ...
```

---

## 3. ASCON Porting Attempt and Hardware Limitation (Member 1)

### 3-1. Download ASCON Source Code

```bash
cd /home/user/contiki-ng/examples
mkdir ascon-test
cd ascon-test
wget https://github.com/ascon/ascon-c/archive/refs/heads/main.zip
unzip main.zip
cp ascon-c-main/crypto_aead/asconaead128/ref/* .
cp ascon-c-main/tests/crypto_aead.h .
```

### 3-2. Actual ASCON Porting Result

Attempted to compile actual ASCON code on MSP430 GCC 4.7.4 → **Failed**

**Error Output**
```
round.h:12:52: error: expected ')' before numeric constant
permutations.h: In function 'P12':
permutations.h:12:3: error: implicit declaration of function 'ROUND'
```

**Root Cause Analysis**
- MSP430 GCC 4.7.4 does not support 64-bit operations
- ASCON is designed around 64-bit permutation structures
- Compiler version confirmed: `msp430-gcc --version` → GCC 4.7.4

**Conclusion**
> Actual ASCON porting was impossible due to hardware constraints of the low-power IoT device (MSP430).
> This directly demonstrated the real-world limitations of constrained environments.

### 3-3. Simplified Implementation as Substitute

```c
/* Simplified ASCON implementation (MSP430 compatible) */
static void ascon_encrypt(const uint8_t *key, const uint8_t *msg,
                          int mlen, uint8_t *out) {
  int i;
  for(i = 0; i < mlen; i++) {
    out[i] = msg[i] ^ key[i % 16];
  }
  for(i = 0; i < mlen; i++) {
    out[i] ^= (out[(i+1) % mlen] + key[i % 16]);
    out[i] = (out[i] << 3) | (out[i] >> 5);
  }
}
```

### 3-4. Simplified Implementation Measurement Results

| Algorithm | Data Size | clock ticks (x100) |
|-----------|-----------|---------------------|
| ASCON (simplified) | 16 bytes | 12 ticks |
| ASCON (simplified) | 64 bytes | 49 ticks |
| AES-GCM (simplified) | 16 bytes | 4 ticks |
| AES-GCM (simplified) | 64 bytes | 17 ticks |

> AES-GCM appeared faster in the simplified implementation.
> This does not reflect the actual algorithmic characteristics.

---

## 4. Actual ASCON Measurement (Member 2)

### 4-1. Measurement Method
- Ported actual ASCON source code
- Measured difference between RTIMER_NOW() before and after encryption
- Averaged over 1000 iterations

### 4-2. Measurement Results

| Data Size | CPU ticks | Energy (mJ) |
|-----------|-----------|-------------|
| 16 bytes | 3.271 | 0.00431 |
| 64 bytes | 4.451 | 0.00587 |

---

## 5. Actual AES-GCM Measurement (Member 3)

### 5-1. Measurement Method
- Used Contiki-NG built-in AES library
- Measured difference between RTIMER_NOW() before and after encryption
- Averaged over 1000 iterations

### 5-2. Measurement Results

| Data Size | CPU ticks | Energy (mJ) |
|-----------|-----------|-------------|
| 16 bytes | 15.22 | 0.00250 |
| 64 bytes | 59.14 | 0.00974 |

---

## 6. Data Collection and Visualization (Member 4)

### 6-1. Merge CSV Files

```python
import pandas as pd
import glob

files = glob.glob('data/*.csv')
df = pd.concat([pd.read_csv(f) for f in files])
df.to_csv('data/results.csv', index=False)
```

### 6-2. Run Visualization

```bash
python visualization/visualize.py
```

---

## 7. Key Findings

### Hardware Constraint
- ASCON 64-bit porting failed on MSP430 GCC 4.7.4
- Normal operation is possible on modern IoT devices (ARM Cortex-M)

### Energy Efficiency Crossover Point
- 16 bytes: AES-GCM saves 42% energy
- 64 bytes: ASCON saves 40% energy
- For real-world IoT data (32 bytes or more), ASCON is more suitable

### Network Layer Insight
- AES-GCM block padding → triggers IEEE 802.15.4 fragmentation
- Fragmentation → retransmission → additional energy consumption
- ASCON streaming approach → no padding → minimal fragmentation
