#ifndef AES_H
#define AES_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

enum {
	PARM_ERROR = 1,
	NOT_INIT_KEY = 2
};

#define BLOCK_SIZE 16

typedef struct
{
	uint32_t nr;                     // rounds
	uint32_t *rk;                    // round_key
	uint32_t buf[(BLOCK_SIZE+1)<<2]; // store round_keys, each block is 4 bytes
} aes_context;

int set_aes_key(aes_context *ctx, const uint8_t *key, uint32_t key_bit);

int aes_encrypt_block(aes_context *ctx, uint8_t cipher_text[16], const uint8_t text[16]);
int aes_decrypt_block(aes_context *ctx, uint8_t text[16], const uint8_t cipher_text[16]);

#ifdef __cplusplus
}
#endif
#endif
