// this file CipherClass.cpp
#include "FanController.h"

/*
 * Chiper.h
 *
 * Original author:
 *  Created on: Feb 28, 2019
 *      Author: joseph
 * Custom mods for my app by Scott Swift 1/2023
 */

//Cipher * cipher = new Cipher();
CipherClass CIP;

const char CIPH_PADDING[CIPKEY_MAX+1] = HTTP_CIPKEY_INIT;

// Constructor/destructor (not used since we don't instantiate as an object using "new",
// The code representing this class is stored in flash, not RAM!)
CipherClass::CipherClass(){
}

CipherClass::~CipherClass(){
}

// Public methods ------------------------------------------------------------------------------
// you must call setCiphKey() when the main key is read from flash on startup - then call when
// the main key is changed. Don't call each time you encrypt/decrypt because setCiphKey() changes the
// initialization vector also!

void CipherClass::setCiphKey(uint8_t* key){
  setCiphKey(String((char*)key));
}
// automatically pads or truncates sKey to 64 chars
void CipherClass::setCiphKey(String sKey){
  
  generateInitializationVector();
  
  if (sKey.isEmpty()){
    sKey = HTTP_CIPKEY_INIT;
    prtln("CipherClass::setKey() key is empty!!! Using HTTP_CIPKEY_INIT");
  }
  
  int len = sKey.length();

  int ii;
  for (ii=0; ii<len && ii<CIPKEY_MAX; ii++)
    _ciphKeyArr[ii] = sKey[ii];
  for (; ii<CIPKEY_MAX; ii++)
    _ciphKeyArr[ii] = CIPH_PADDING[ii];
  _ciphKeyArr[CIPKEY_MAX] = '\0';
  
  if (len > CIPKEY_MAX)
    prtln("Cipher key too long! Truncated to " + String(CIPKEY_MAX) + " chars!");
//  prtln("Cipher key set to: \"" + getCiphKey() + "\"");
}

String CipherClass::getCiphKey(){
  return String((char*)_ciphKeyArr);
}

// Pass in context of CIPH_CONTEXT_FOREGROUND or CIPH_CONTEXT_BACKGROUND
// automatically saves the previous key which can be restored using restoreCiphKey()

void CipherClass::saveCiphKey(int context){
  if (context == CIPH_CONTEXT_FOREGROUND)
    _sSaveKey1 = String((char*)_ciphKeyArr);
  else
    _sSaveKey2 = String((char*)_ciphKeyArr);
}

void CipherClass::restoreCiphKey(int context){
  if (context == CIPH_CONTEXT_FOREGROUND){
    if (!_sSaveKey1.isEmpty()){
      setCiphKey(_sSaveKey1);  
      _sSaveKey1 = "";
    }
  }else{
    if (!_sSaveKey2.isEmpty()){
      setCiphKey(_sSaveKey2);  
      _sSaveKey2 = "";
    }
  }
}

int CipherClass::encryptBuf(uint8_t * bufInOut, int iLength, int token, int context){
  return encryptDecryptBuf(bufInOut, iLength, false, token, context);
}

String CipherClass::encryptString(String sIn, int token, int context){
  return encryptDecryptString(sIn, false, token, context);
}

int CipherClass::decryptBuf(uint8_t * bufInOut, int iLength, int token, int context){
  return encryptDecryptBuf(bufInOut, iLength, true, token, context);
}

String CipherClass::decryptString(String sIn, int token, int context){
  return encryptDecryptString(sIn, true, token, context);
}

String CipherClass::getInitializationVector(){
  return String((char*)iv);
}

// Private methods -------------------------------------------------------------------

String CipherClass::encryptDecryptString(String sIn, bool bDecrypt, int token, int context){
  uint8_t* p = cyphBufFromString(sIn);
  
  if (!p)
    return "";
  
//prtln("1:encryptDecryptString(): \"" + String((char*)p) + "\", sIn.length()=" + String(sIn.length()));

  int retVal = encryptDecryptBuf(p, sIn.length(), bDecrypt, token, context);

  if (retVal < 0){
    free(p);
    return "";
  }
  
  // NOTE: if encrypting, we need to retain the size as a multiple of 16,
  // but when decrypting, we need to trim off any excess '\0' chars.
  String sOut = bDecrypt ? String((char*)p) : String((char*)p, sIn.length());
  
//prtln("2:encryptDecryptString(): \"" + sOut + "\", sOut.length()=" + String(sOut.length()));

  free(p);
  return sOut;
}

// we expect bufLen to be the size of bufInOut[] and it must be a multiple of 16 (CIPBUF_SIZE)
// we expect bufInOut[] to be padded to the end with '\0'
int CipherClass::encryptDecryptBuf(uint8_t* bufInOut, int bufLen, bool bDecrypt, int token, int context){

  if (bufLen <= 0)
    return -2;

//prtln("shift array = \"" + String(keyPtr) + "\"");  

  uint8_t *keyPtr = getShiftArrayPtr(context);
  shiftCiphKey(token, keyPtr);

  // init
  mbedtls_aes_context *aesPtr = getAesContextPtr(context);
  mbedtls_aes_init(aesPtr);
  
  int modeFlag;
  if (bDecrypt){
    modeFlag = MBEDTLS_AES_DECRYPT;
    mbedtls_aes_setkey_dec(aesPtr, (const unsigned char*)keyPtr, CIPKEY_BITS_LENGTH);
  }
  else{
    modeFlag = MBEDTLS_AES_ENCRYPT;
    mbedtls_aes_setkey_enc(aesPtr, (const unsigned char*)keyPtr, CIPKEY_BITS_LENGTH);
  }
    
  uint8_t ivcopy[CIPBUF_SIZE+1]; // initialization vector copy

  for (int ii=0; ii<CIPBUF_SIZE+1; ii++)
    ivcopy[ii] = iv[ii];
    
  uint8_t * ptr = bufInOut;
  
  int block_count = bufLen/CIPBUF_SIZE;

  for (int block=0; block<block_count; block++){    
    // decrypt/encrypt plainText buffer of length CIPBUF_SIZE characters
    mbedtls_aes_crypt_cbc(aesPtr, modeFlag, CIPBUF_SIZE, ivcopy, (const unsigned char *)ptr, (unsigned char *)ptr);
    ptr += CIPBUF_SIZE;    
    yield();
  }

  // free...
  mbedtls_aes_free(aesPtr);
  *aesPtr = {};
  
  // clean up
  for (int ii=0; ii<CIPBUF_SIZE+1; ii++)
    ivcopy[ii] = 0;

  return 0;
}

// make a shifted table using token as an offset
void CipherClass::shiftCiphKey(int token, uint8_t *keyPtr){
  
  int tokIdx = token;
  
  _ciphKeyArr[CIPKEY_MAX] = 0; // just in case...
  int len = strlen((char*)_ciphKeyArr); // should be CIPKEY_MAX - but allow for a shorter key-string...
//prtln("keylen:" + String(len));

  // handle case of a token larger than the key-length
  if (tokIdx >= len)
    tokIdx %= len;
//prtln("tokIdx:" + String(tokIdx));

  for (int ii=0; ii<len; ii++){
    if (tokIdx >= len)
      tokIdx = 0;
    keyPtr[tokIdx++] = _ciphKeyArr[ii];
//prtln("ii:" + String(ii) + ", tokIdx:" + String(tokIdx) + ", c=" + String(keyPtr[tokIdx-1]));
  }
  keyPtr[len] = 0;
//prtln("DEBUG: shiftCiphKey(): \"" + String((char*)keyPtr) + "\", " + String(token));
}

uint8_t* CipherClass::getShiftArrayPtr(int context){
  return (context == CIPH_CONTEXT_FOREGROUND) ? _shiftKeyArr1 : _shiftKeyArr2;  
}

mbedtls_aes_context* CipherClass::getAesContextPtr(int context){
  return (context == CIPH_CONTEXT_FOREGROUND) ? &_aes1 : &_aes2;
}

void CipherClass::generateInitializationVector(){

  // all units need to be using the same initialization vector...
  strcpy((char*)iv, CIPH_INIT_VECT);

// for now, a random vector won't be useful because the remote units would need it
// passed to them, presumably by the master...
//
//  int idx = random(0,CIPBUF_SIZE);
//  for (int ii=0; ii<CIPBUF_SIZE; ii++){
//    iv[idx++] = (unsigned char)random(1,256);
//    if (idx >= CIPBUF_SIZE)
//      idx = 0;
//  }
//  iv[CIPBUF_SIZE] = '\0';
}

// returns pointer to buffer created from string
// returns NULL if error creating buffer
// call "free(pBuf)" to delete when finished!!!!
uint8_t* CipherClass::cyphBufFromString(String sIn){
  int len = sIn.length();
  if (!len)
    return NULL;
  int numBlocks = len/CIPBUF_SIZE;
  int rem = len%CIPBUF_SIZE;
  if (rem)
    numBlocks++;
  int bufLen = (numBlocks*CIPBUF_SIZE)+1;
  uint8_t* bufInOut = (uint8_t*)malloc(bufLen);
  if (!bufInOut)
    return NULL;
  int ii;
  for (ii=0; ii<len; ii++)
    bufInOut[ii] = sIn[ii];
  for (; ii<bufLen; ii++)
    bufInOut[ii] = '\0';
  return bufInOut;
}
