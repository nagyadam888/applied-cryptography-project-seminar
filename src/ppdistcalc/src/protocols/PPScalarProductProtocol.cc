#include "PPScalarProductProtocol.h"

#include <climits>

#include <omnetpp.h>
#include <gmp.h>

#include "../encryption/paillier.h"
#include "../encryption/HomomorphicEncryption.h"
#include "InitiatorData.h"
#include "AcceptorData.h"

#include "messages/Msg1_m.h"
#include "messages/Msg2_m.h"
#include "messages/Msg3_m.h"
#include "messages/Msg4_m.h"

PPScalarProductProtocol* PPScalarProductProtocol::initiate(long* vector, unsigned int size, bool retrieveDistance) {
    PPScalarProductProtocol* ppspp = new PPScalarProductProtocol();
    ppspp->size = size;
    ppspp->vector = new unsigned long(size);
    for(unsigned int i = 0; i < size; ++i) {
        ppspp->vector[i] = (unsigned long) vector[i]; //(vector[i] - LLONG_MIN);
        //std::cout << "init vec: " << std::to_string(ppspp->vector[i]) << std::endl;
    }

    ppspp->protocolData = new InitiatorData();
    InitiatorData* data = (InitiatorData*) ppspp->protocolData;

    HomomorphicEncryption::generateKeyPair(&data->pubKey, &data->prvKey);
    //std::cout << "jojojo" << std::endl;
    if(data->pubKey == NULL && data->prvKey == NULL) {
        delete ppspp;
        return NULL;
    }

    ppspp->encryptor = new HomomorphicEncryption();
    ppspp->encryptor->initialize(data->pubKey);

    ppspp->role = PPSPNodeRole::INITIATOR;
    ppspp->state = PPSPNodeState::IDLE;

    ppspp->calculateDistance = retrieveDistance;

    return ppspp;
}

PPScalarProductProtocol* PPScalarProductProtocol::accept(long* vector, unsigned int size, bool retrieveDistance) {
    PPScalarProductProtocol* ppspp = new PPScalarProductProtocol();
    ppspp->size = size;
    ppspp->vector = new unsigned long(size);
    for(unsigned int i = 0; i < size; ++i) {
        ppspp->vector[i] = (unsigned long) vector[i]; //(vector[i] - LLONG_MIN);
    }

    ppspp->protocolData = new AcceptorData();
    // AcceptorData* data = (AcceptorData*) ppspp->protocolData;

    ppspp->encryptor = new HomomorphicEncryption();

    ppspp->role = PPSPNodeRole::ACCEPTOR;
    ppspp->state = PPSPNodeState::IDLE;

    ppspp->calculateDistance = retrieveDistance;

    // std::cout << "accept: vector: " << std::to_string(ppspp->vector[0]) << ", " << std::to_string(ppspp->vector[1]) << std::endl;

    return ppspp;
}



omnetpp::cMessage* PPScalarProductProtocol::processMessage(omnetpp::cMessage* receivedMessage) {
    if(this->role == PPSPNodeRole::INITIATOR) {
        InitiatorData* data = (InitiatorData*) this->protocolData;

        /*
         * Generate the initial message
         */
        if((this->state == PPSPNodeState::IDLE) && (receivedMessage == NULL)) {

            unsigned int i;

            // Calculate R = (r_1, ..., r_size)
            // -------------------------------------------------
            data->r = new unsigned long(this->size);
            for(i = 0; i < this->size; ++i) {
                data->r[i] = this->random();
            }

            // Calculate P = (vec_1 + r_1, ...)
            // -------------------------------------------------
            data->p = new unsigned long(this->size);
            for(i = 0; i < this->size; ++i) {
                data->p[i] = this->vector[i] + data->r[i];
            }


            // Homomorphic Encryption:  Enc(P)
            // -------------------------------------------------
            paillier_ciphertext_t** encP = new paillier_ciphertext_t*[this->size];
            for(i = 0; i < this->size; ++i) {
                encP[i] = this->encryptor->encrypt(data->p[i]);
            }

            Msg1* msg = new Msg1("Message1");
            msg->setRArraySize(this->size);
            for(i = 0; i < this->size; ++i) {
                msg->setR(i, data->r[i]);
            }

            msg->setEncPArraySize(this->size);
            for(i = 0; i < this->size; ++i) {
                msg->setEncP(i, encP[i]);
            }

            char* serializedPubKey = HomomorphicEncryption::createSerializedPublicKey(data->pubKey);
            msg->setPubKey(serializedPubKey); // <-- this creates a copy, so we can safely free memory after this
            delete[] serializedPubKey;

            this->state = PPSPNodeState::SENT_MSG_1;

            return msg;
        }

        else if((this->state == PPSPNodeState::SENT_MSG_1) && (dynamic_cast<Msg2*>(receivedMessage) != nullptr)) {
            Msg2* packet = (Msg2*) receivedMessage;

            data->u = this->encryptor->decrypt(packet->getS1(), data->prvKey) - packet->getS2();
            for(unsigned int i = 0; i < this->size; ++i) {
                data->u -= (this->vector[i] * packet->getR(i));
            }

            if(this->calculateDistance) {
                unsigned long sp = 0L;
                for(int i = 0; i < this->size; ++i) {
                    sp += (this->vector[i] * this->vector[i]);
                }
                data->u = (-2L * data->u) + sp;
            }

            Msg3* msg = new Msg3("Message3");
            msg->setU(data->u);

            this->state = PPSPNodeState::SENT_MSG_3;

            return msg;
        }

        else if((this->state == PPSPNodeState::SENT_MSG_3) && (dynamic_cast<Msg4*>(receivedMessage) != nullptr)) {
            Msg4* packet = (Msg4*) receivedMessage;

            this->result = packet->getResult();
            this->state = PPSPNodeState::IDLE;
            this->finished = true;

            return NULL;
        }
    }


    else if(this->role == PPSPNodeRole::ACCEPTOR) {
        AcceptorData* data = (AcceptorData*) this->protocolData;

        if((this->state == PPSPNodeState::IDLE) && (dynamic_cast<Msg1*>(receivedMessage) != nullptr)) {
            Msg1* packet = (Msg1*) receivedMessage;

            const char* serializedPubKey = packet->getPubKey(); // <-- this gets automatically freed when the message object gets destroyed ...
            data->pubKey = HomomorphicEncryption::createDeserializedPublicKey(serializedPubKey); // ... therefore this creates a *copy* while deserializing

            this->encryptor->initialize(data->pubKey);


            unsigned int i;
            // Calculate R = (r_1, ..., r_size)
            // -------------------------------------------------
            data->r = new unsigned long(this->size);
            for(i = 0; i < this->size; ++i) {
                data->r[i] = this->random();
            }

            // Calculate P = (vec_1 + r_1, ...)
            // -------------------------------------------------
            data->p = new unsigned long(this->size);
            for(i = 0; i < this->size; ++i) {
                data->p[i] = this->vector[i] + data->r[i];
            }

            // Calculate random v
            // -------------------------------------------------
            data->v = this->random();


            // Calculate s1 and s2
            // -------------------------------------------------
            data->s1 = this->encryptor->encrypt((data->v));
            for(i = 0; i < this->size; ++i) {
                data->s1 = this->encryptor->mul(data->s1, this->encryptor->pow(packet->getEncP(i), data->p[i]));
            }

            data->s2 = 0L;
            for(i = 0; i < this->size; ++i) {
                data->s2 += (packet->getR(i) * data->p[i]);
            }

            Msg2* msg = new Msg2("Message2");
            msg->setRArraySize(this->size);
            for(i = 0; i < this->size; ++i) {
                msg->setR(i, data->r[i]);
            }
            msg->setS1(data->s1);
            msg->setS2(data->s2);

            this->state = PPSPNodeState::SENT_MSG_2;

            return msg;
        }

        else if((this->state == PPSPNodeState::SENT_MSG_2) && (dynamic_cast<Msg3*>(receivedMessage) != nullptr)) {
            Msg3* packet = (Msg3*) receivedMessage;

            long scalarProduct;
            if(!this->calculateDistance) {
                scalarProduct = (long)(packet->getU() - data->v);
            }
            else {
                unsigned long v_dist = 0L;
                unsigned long sp = 0L;
                for(int i = 0; i < this->size; ++i) {
                    sp += (this->vector[i] * this->vector[i]);
                }
                v_dist = (2L * data->v) + sp;
                scalarProduct = (long)(packet->getU() + v_dist);
            }

            /*if(tmp > LLONG_MAX) {
                scalarProduct = ULLONG_MAX - tmp + 1;
            }
            else {
                scalarProduct = (long)tmp;
            }*/
            //unsigned long scalarProduct = ULLONG_MAX - (packet->getU() - data->v) + 1;
            //std::cout << "sp before: " << std::to_string(tmp) << std::endl;
            //std::cout << "sp after: " << std::to_string(scalarProduct) << std::endl;


            this->result = scalarProduct;
            this->state = PPSPNodeState::IDLE;
            this->finished = true;

            Msg4* msg = new Msg4("Message4");
            msg->setResult(scalarProduct);

            return msg;
        }
    }
}

PPSPNodeRole PPScalarProductProtocol::getRole() {
    return this->role;
}

bool PPScalarProductProtocol::isFinished() {
    return this->finished;
}

long PPScalarProductProtocol::getResult() {
    return this->result;
}


PPScalarProductProtocol::PPScalarProductProtocol() {
    this->finished = false;
    this->result = 0L;
    this->randstateInitialized = false;
}

PPScalarProductProtocol::~PPScalarProductProtocol() {
    if(this->role == PPSPNodeRole::INITIATOR) {
        InitiatorData* data = (InitiatorData*) this->protocolData;
        paillier_freepubkey(data->pubKey);
        paillier_freeprvkey(data->prvKey);
        delete[] data->r;
        delete[] data->p;

        delete data;
        delete this->vector;
        delete this->encryptor;
    }

    else if(this->role == PPSPNodeRole::ACCEPTOR) {
        AcceptorData* data = (AcceptorData*) this->protocolData;
        paillier_freepubkey(data->pubKey);
        delete[] data->r;
        delete[] data->p;

        delete data;
        delete this->vector;
        delete this->encryptor;
    }
}

unsigned long int PPScalarProductProtocol::random() {
    if(!this->randstateInitialized) {
        // gmp_randinit_default(this->randstate);
        this->initRand(this->randstate, HomomorphicEncryption::MODULUS_BITS);
        this->randstateInitialized = true;
    }

    mpz_t num;
    mpz_init(num);

    mpz_urandomb(num, this->randstate, (sizeof(unsigned long int) * 8) / 2);

    return mpz_get_ui(num);
}

void PPScalarProductProtocol::initRand(gmp_randstate_t rand, int bytes) {
    void* buf;
    mpz_t s;

    buf = malloc(bytes);
    paillier_get_rand_devurandom(buf, bytes);

    gmp_randinit_default(rand);
    mpz_init(s);
    mpz_import(s, bytes, 1, 1, 0, 0, buf);
    gmp_randseed(rand, s);
    mpz_clear(s);

    free(buf);
}

