#include "contiki.h"
#include "sys/log.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include <string.h>
#include <stdint.h>

#define LOG_MODULE "CRYPTO"
#define LOG_LEVEL LOG_LEVEL_INFO
#define REPEAT 100

/* ASCON 단순화 구현 */
static void ascon_encrypt(const uint8_t *key, const uint8_t *msg, int mlen, uint8_t *out) {
  int i;
  for(i = 0; i < mlen; i++) {
    out[i] = msg[i] ^ key[i % 16];
  }
  for(i = 0; i < mlen; i++) {
    out[i] ^= (out[(i+1) % mlen] + key[i % 16]);
    out[i] = (out[i] << 3) | (out[i] >> 5);
  }
}

/* AES-GCM 단순화 구현 */
static void aesgcm_encrypt(const uint8_t *key, const uint8_t *msg, int mlen, uint8_t *out) {
  int i, j;
  uint8_t block[16];
  /* AES 특성 시뮬레이션: SubBytes + ShiftRows + MixColumns 흉내 */
  for(i = 0; i < mlen; i += 16) {
    int blen = (mlen - i) < 16 ? (mlen - i) : 16;
    memcpy(block, msg + i, blen);
    /* SubBytes */
    for(j = 0; j < blen; j++) {
      block[j] = (block[j] ^ key[j % 16]) + 0x63;
    }
    /* ShiftRows 흉내 */
    for(j = 0; j < blen - 1; j++) {
      block[j] ^= block[j+1];
    }
    /* MixColumns 흉내 */
    for(j = 0; j < blen; j++) {
      block[j] = (block[j] << 1) ^ (block[j] >> 7);
    }
    memcpy(out + i, block, blen);
  }
  /* GCM 태그 시뮬레이션 */
  for(i = 0; i < mlen; i++) {
    out[i] ^= key[(i+1) % 16];
  }
}

static uint8_t data_16[16] = "SensorData16Byte";
static uint8_t data_32[32] = "SensorData32BytesSensorData32By!";
static uint8_t data_64[64] = "SensorData64BytesSensorData64BytesSensorData64BytesSensorData64!";
static uint8_t key[16]     = "SecretKey1234567";
static uint8_t out[80];
static struct etimer et;

PROCESS(crypto_test_process, "Crypto Test");
AUTOSTART_PROCESSES(&crypto_test_process);

PROCESS_THREAD(crypto_test_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, 2 * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  clock_time_t t0, t1;
  int i;

  /* ASCON 16 bytes */
  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    ascon_encrypt(key, data_16, 16, out);
  }
  t1 = clock_time();
  LOG_INFO("ASCON 16bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  /* ASCON 32 bytes */
  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    ascon_encrypt(key, data_32, 32, out);
  }
  t1 = clock_time();
  LOG_INFO("ASCON 32bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  /* ASCON 64 bytes */
  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    ascon_encrypt(key, data_64, 64, out);
  }
  t1 = clock_time();
  LOG_INFO("ASCON 64bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  /* AES-GCM 16 bytes */
  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_encrypt(key, data_16, 16, out);
  }
  t1 = clock_time();
  LOG_INFO("AESGCM 16bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  /* AES-GCM 32 bytes */
  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_encrypt(key, data_32, 32, out);
  }
  t1 = clock_time();
  LOG_INFO("AESGCM 32bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  /* AES-GCM 64 bytes */
  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_encrypt(key, data_64, 64, out);
  }
  t1 = clock_time();
  LOG_INFO("AESGCM 64bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  PROCESS_END();
}
