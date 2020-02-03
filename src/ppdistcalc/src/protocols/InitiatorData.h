struct paillier_pubkey_t;
struct paillier_prvkey_t;
struct InitiatorData {
    //unsigned long int xPos, yPos;
    unsigned long* r;
    unsigned long* p;
    unsigned long u;
    //unsigned long int u_dist;
    paillier_pubkey_t* pubKey;
    paillier_prvkey_t* prvKey;
};
