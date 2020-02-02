struct paillier_pubkey_t;
struct paillier_ciphertext_t;
struct AcceptorData {
    // unsigned long int xPos, yPos;
    unsigned long* r;
    unsigned long* p;
    unsigned long v;
    // unsigned long int v_dist;
    paillier_ciphertext_t* s1;
    unsigned long s2;
    paillier_pubkey_t* pubKey;
};
