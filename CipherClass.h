/*
 * CipherClass.h
 * Original author: joseph, Created on: Feb 28, 2019
 * Changes/additions for my app 1/2024, Author: Scott Swift
 */

#ifndef CipherH
#define CipherH

#include "mbedtls/aes.h"
#include <Arduino.h>

// NOTE: be sure to put the backslash escape before the " or \ char!
// NOTE:  only all lower-case works reliably!
#define HTTP_CIPKEY_INIT "J<cM7?v/CH)aQg\x07tS*u%p/=\"o4VT5,SF" // 32 chars for 256-bit AES encryption (escape sequence \x01-\xff allowed)

// initialization vector
#define CIPH_INIT_VECT "gLO#\x0bP7kjy*b&c3B" // 16 chars (escape sequence \x01-\xff allowed)

#define CIPKEY_MAX 32 // need 16 bytes for 128-bit and 32 bytes for 256-bit encryption
#define CIPKEY_BITS_LENGTH (CIPKEY_MAX*8)
#define CIPBUF_SIZE 16 // decrypt in 16-char chunks

#define CIPH_CONTEXT_FOREGROUND 0
#define CIPH_CONTEXT_BACKGROUND 1

class CipherClass {
public:
  /** Default constructor, privateChiperKey property will be set on a default, unsecure value
   *
   *  @param  ---
   *  @return ---
  */
  CipherClass();
  
  /** Overloaded constructor, privateChiperKey will be set on @param key
   *
   *  @param key secure key as pointer on char array
   *  @return ---
  */
//	Cipher(String sKey);

  virtual ~CipherClass();

  String getInitializationVector();
  void setCiphKey(uint8_t * key);
  void setCiphKey(String sKey);
  String getCiphKey();
  int encryptBuf(uint8_t * bufInOut, int iLength, int token, int context);
  String encryptString(String sIn, int token, int context);
  int decryptBuf(uint8_t * bufInOut, int iLength, int token, int context);
  String decryptString(String sIn, int token, int context);
  void saveCiphKey(int context);
  void restoreCiphKey(int context);
  uint8_t* getShiftArrayPtr(int context);
  mbedtls_aes_context* getAesContextPtr(int context);

private:

  uint8_t* cyphBufFromString(String sIn);
  void generateInitializationVector();
  String encryptDecryptString(String sIn, bool bDecrypt, int token, int context);
  int encryptDecryptBuf(uint8_t * bufInOut, int iLength, bool bDecrypt, int token, int context);
  void shiftCiphKey(int token, uint8_t *keyPtr);

  uint8_t iv[CIPBUF_SIZE+1]; // initialization vector

  uint8_t _ciphKeyArr[CIPKEY_MAX+1], _shiftKeyArr1[CIPKEY_MAX+1], _shiftKeyArr2[CIPKEY_MAX+1];  

  mbedtls_aes_context _aes1, _aes2;
  String _sSaveKey1, _sSaveKey2;
};

#endif /* CIPHER_H_ */

extern CipherClass CIP;
