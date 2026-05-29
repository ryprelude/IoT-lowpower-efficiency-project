#include "contiki.h"
#include "lib/aes-128.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include <stdint.h>
#include <string.h>
#include <time.h>

#define LOG_MODULE "AESGCM"
#define LOG_LEVEL LOG_LEVEL_INFO
#define REPEAT 10000
#define TAG_SIZE 16

static uint8_t data_16[16] = "SensorData16Byte";
static uint8_t data_32[32] = "SensorData32BytesSensorData32By!";
static uint8_t data_64[64] = "SensorData64BytesSensorData64BytesSensorData64BytesSensorData64!";
static uint8_t key[16] = "SecretKey1234567";
static uint8_t nonce[16] = "RandomNonce12345";
static uint8_t out[96];
static struct etimer et;

static unsigned long
elapsed_us(clock_t start, clock_t end)
{
  return (unsigned long)(((double)(end - start) * 1000000.0) / CLOCKS_PER_SEC);
}

static void
aesgcm_block_process(const uint8_t *msg, int mlen, uint8_t *result)
{
  uint8_t block[16];
  uint8_t tag[16];
  int i;
  int j;

  memset(tag, 0, sizeof(tag));
  AES_128.set_key(key);

  for(i = 0; i < mlen; i += 16) {
    int blen = (mlen - i) < 16 ? (mlen - i) : 16;
    memset(block, 0, sizeof(block));
    memcpy(block, msg + i, blen);

    for(j = 0; j < 16; j++) {
      block[j] ^= nonce[j % sizeof(nonce)];
    }

    AES_128.encrypt(block);
    memcpy(result + i, block, blen);

    for(j = 0; j < 16; j++) {
      tag[j] ^= block[j];
    }
    AES_128.encrypt(tag);
  }

  memcpy(result + mlen, tag, TAG_SIZE);
}

PROCESS(aesgcm_cooja_process, "AESGCM Cooja Test");
AUTOSTART_PROCESSES(&aesgcm_cooja_process);

PROCESS_THREAD(aesgcm_cooja_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, 2 * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  clock_t t0, t1;
  int i;

  t0 = clock();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_block_process(data_16, 16, out);
  }
  t1 = clock();
  LOG_INFO("AESGCM 16bytes host elapsed us (x%d): %lu\n", REPEAT, elapsed_us(t0, t1));

  t0 = clock();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_block_process(data_32, 32, out);
  }
  t1 = clock();
  LOG_INFO("AESGCM 32bytes host elapsed us (x%d): %lu\n", REPEAT, elapsed_us(t0, t1));

  t0 = clock();
  for(i = 0; i < REPEAT; i++) {
    aesgcm_block_process(data_64, 64, out);
  }
  t1 = clock();
  LOG_INFO("AESGCM 64bytes host elapsed us (x%d): %lu\n", REPEAT, elapsed_us(t0, t1));

  PROCESS_END();
}
