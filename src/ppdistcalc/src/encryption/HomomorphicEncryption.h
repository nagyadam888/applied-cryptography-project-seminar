#ifndef SRC_ENCRYPTION_HOMOMORPHICENCRYPTION_H_
#define SRC_ENCRYPTION_HOMOMORPHICENCRYPTION_H_

struct paillier_pubkey_t;
struct paillier_prvkey_t;
struct paillier_ciphertext_t;

/*
 * Wrapper object to conveniently use an additive homomorphic encryption scheme.
 * Note: In this case, we use the Paillier scheme.
 */
class HomomorphicEncryption {
public:
    static const long MODULUS_BITS = 1024;

    static void generateKeyPair(paillier_pubkey_t** publicKey, paillier_prvkey_t** privateKey);
    static char* createSerializedPublicKey(paillier_pubkey_t* publicKey);
    static paillier_pubkey_t* createDeserializedPublicKey(const char* serializedPublicKey);

    HomomorphicEncryption();
    virtual ~HomomorphicEncryption();

    /*
     * Initialize with a given public key
     */
    virtual bool initialize(paillier_pubkey_t* publicKey);

    /*
     * Encrypt a number and receive a ciphertext object
     */
    virtual paillier_ciphertext_t* encrypt(unsigned long int plaintext);

    /*
     * Convert a ciphertext object to an array of bytes.
     * Useful for storing it or sending it across the network.
     */
    virtual void* serializeCipherAndFree(paillier_ciphertext_t* cipherObject);

    /*
     * Convert a byte array back to a ciphertext object in order to apply operations on it.
     */
    virtual paillier_ciphertext_t* deserializeCipherAndFree(void* byteArray);

    /*
     * Decrypt a cipheretext stored as a byte array and receive a number
     * Note: Only succeeds when using the correct private key
     */
    virtual unsigned long int decrypt(paillier_ciphertext_t* ciphertext, paillier_prvkey_t* privateKey);



    /*
     * Takes a ciphertext to the power of a scalar.
     * This corresponds to a multiplication on the plaintext.
     * Note: This also frees the original ciphertext for convenience!
     */
    virtual paillier_ciphertext_t* pow(paillier_ciphertext_t* ciphertext, unsigned long int scalar);

    /*
     * Multiplies two ciphertexts.
     * This corresponds to an addition on the plaintext.
     * Note: This also frees the original ciphertexts for convenience!
     */
    virtual paillier_ciphertext_t* mul(paillier_ciphertext_t* cipher1, paillier_ciphertext_t* cipher2);

private:
    bool isInitialized;
    paillier_pubkey_t* publicKey;
};

#endif /* SRC_ENCRYPTION_HOMOMORPHICENCRYPTION_H_ */
