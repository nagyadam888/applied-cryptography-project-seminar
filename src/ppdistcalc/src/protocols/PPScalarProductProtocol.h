#ifndef SRC_PPSCALARPRODUCTPROTOCOL_H_
#define SRC_PPSCALARPRODUCTPROTOCOL_H_

#include <gmp.h>
#include <omnetpp.h>

enum PPSPNodeState {
    IDLE,
    SENT_MSG_1,
    RECEIVED_MSG_1,
    SENT_MSG_2,
    RECEIVED_MSG_2,
    SENT_MSG_3,
    RECEIVED_MSG_3
};

enum PPSPNodeRole {
    NONE,
    INITIATOR,
    ACCEPTOR
};

class HomomorphicEncryption;
class PPScalarProductProtocol {
public:
    static PPScalarProductProtocol* initiate(long* vector, unsigned int size, bool retrieveDistance);
    static PPScalarProductProtocol* accept(long* vector, unsigned int size, bool retrieveDistance);

    omnetpp::cMessage* processMessage(omnetpp::cMessage* receivedMessage);

    PPSPNodeRole getRole();
    bool isFinished();
    long getResult();

    virtual ~PPScalarProductProtocol();

private:
    PPScalarProductProtocol();

    bool calculateDistance;

    bool finished;
    long result;


    PPSPNodeRole role;

    PPSPNodeState state;

    unsigned int size;
    unsigned long* vector;
    void* protocolData;

    HomomorphicEncryption* encryptor;



    /*
     * Generates a random number using the GMP library
     */
    unsigned long int random();

    /*
     * Initializes GMP's RNG.
     * Adapted from the paillier library
     */
    void initRand(gmp_randstate_t rand, int bytes);

    bool randstateInitialized;
    gmp_randstate_t randstate;
};

#endif /* SRC_PPSCALARPRODUCTPROTOCOL_H_ */
