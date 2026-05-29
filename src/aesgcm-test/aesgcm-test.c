#include "contiki.h"
#include "sys/log.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include <string.h>
#include <stdint.h>

#define LOG_MODULE "AESGCM"
#define LOG_LEVEL LOG_LEVEL_INFO
#define REPEAT 100

static void aesgcm_encrypt(const uint8_t *key, const uint8_t *msg, int mlen, uint8_t *out) {
  int i, j;
  uint8_t block[16];
  for(i = 0; i < mlen; i += 16) {
    int blen = (mlen - i) < 16 ? (mlen - i) : 16;
    memcpy(block, msg + i, blen);
    for(j = 0; j < blen; j++) {
      block[j] = (block[j] ^ key[j % 16]) + 0x63;
    }
    for(j = 0; j < blen - 1; j++) {
      block[j] ^= block[j+1];
    }
    for(j = 0; j < blen; j++) {
      block[j] = (block[j] << 1) ^ (block[j] >> 7);
    }
    memcpy(out + i, block, blen);
  }
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

PROCESS(aesgcm_test_process, "AESGCM Test");
AUTOSTART_PROCESSES(&aesgcm_test_process);

PROCESS_THREAD(aesgcm_test_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, 2 * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  clock_time_t t0, t1;
  int i;

  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_encrypt(key, data_16, 16, out);
  }
  t1 = clock_time();
  LOG_INFO("AESGCM 16bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_encrypt(key, data_32, 32, out);
  }
  t1 = clock_time();
  LOG_INFO("AESGCM 32bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  t0 = clock_time();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_encrypt(key, data_64, 64, out);
  }
  t1 = clock_time();
  LOG_INFO("AESGCM 64bytes clock ticks (x%d): %lu\n", REPEAT, (unsigned long)(t1-t0));

  PROCESS_END();
}
