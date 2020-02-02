#include "HomomorphicEncryption.h"

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <gmp.h>
#include "paillier.h"



void HomomorphicEncryption::generateKeyPair(paillier_pubkey_t** publicKey, paillier_prvkey_t** privateKey) {
    paillier_pubkey_t* testPub;
    paillier_prvkey_t* testPrv;
    paillier_keygen(HomomorphicEncryption::MODULUS_BITS, publicKey, privateKey, paillier_get_rand_devurandom);
    std::cout << "test" << std::endl;
}

char* HomomorphicEncryption::createSerializedPublicKey(paillier_pubkey_t* publicKey) {
    return paillier_pubkey_to_hex(publicKey);
}

paillier_pubkey_t* HomomorphicEncryption::createDeserializedPublicKey(const char* serializedPublicKey) {
    return paillier_pubkey_from_hex(const_cast<char*>(serializedPublicKey));
}



HomomorphicEncryption::HomomorphicEncryption() {
    this->isInitialized = false;
    this->publicKey = NULL;
}

HomomorphicEncryption::~HomomorphicEncryption() {
}


bool HomomorphicEncryption::initialize(paillier_pubkey_t* publicKey) {
    this->publicKey = publicKey;
    this->isInitialized = true;
    return this->isInitialized;
}

paillier_ciphertext_t* HomomorphicEncryption::encrypt(unsigned long int plaintext) {
    paillier_plaintext_t* plainPtr = paillier_plaintext_from_ui(plaintext);
    paillier_ciphertext_t* cipherPtr = paillier_enc(NULL, this->publicKey, plainPtr, paillier_get_rand_devurandom);
    paillier_freeplaintext(plainPtr);
    return cipherPtr;
}

void* HomomorphicEncryption::serializeCipherAndFree(paillier_ciphertext_t* cipherObject) {
    void* cipherByteArr = paillier_ciphertext_to_bytes(PAILLIER_BITS_TO_BYTES(this->publicKey->bits)*2, cipherObject);
    paillier_freeciphertext(cipherObject);
    return cipherByteArr;
}

paillier_ciphertext_t* HomomorphicEncryption::deserializeCipherAndFree(void* byteArray) {
    paillier_ciphertext_t* cipherObject = paillier_ciphertext_from_bytes(byteArray, PAILLIER_BITS_TO_BYTES(this->publicKey->bits)*2);
    delete[] byteArray;
    return cipherObject;
}

unsigned long int HomomorphicEncryption::decrypt(paillier_ciphertext_t* cipherObject, paillier_prvkey_t* privateKey) {
    paillier_plaintext_t* plainPtr = paillier_dec(NULL, this->publicKey, privateKey, cipherObject);
    unsigned long int plaintext = mpz_get_ui(plainPtr->m);
    paillier_freeplaintext(plainPtr);
    return plaintext;
}

paillier_ciphertext_t* HomomorphicEncryption::pow(paillier_ciphertext_t* ciphertext, unsigned long int scalar) {
    paillier_ciphertext_t* resultCipher = paillier_create_enc_zero();
    paillier_plaintext_t* scalarPlain = paillier_plaintext_from_ui(scalar);
    paillier_exp(this->publicKey, resultCipher, ciphertext, scalarPlain);
    paillier_freeplaintext(scalarPlain);
    paillier_freeciphertext(ciphertext);
    return resultCipher;
}

paillier_ciphertext_t* HomomorphicEncryption::mul(paillier_ciphertext_t* cipher1, paillier_ciphertext_t* cipher2) {
    paillier_ciphertext_t* resultCipher = paillier_create_enc_zero();
    paillier_mul(this->publicKey, resultCipher, cipher1, cipher2);
    paillier_freeciphertext(cipher1);
    paillier_freeciphertext(cipher2);
    return resultCipher;
}
