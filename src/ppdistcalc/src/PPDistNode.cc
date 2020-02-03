#include "PPDistNode.h"

#include <cmath>

using namespace omnetpp;



void PPDistNode::initialize() {

    if (strcmp("alice", getName()) == 0) {

        // Budapest
        this->latitude = 47.498295;
        this->longitude = 19.047212;

        if(PPDistNode::calcMethod == DistanceCalculationMethod::FLAT) {
            this->initiateFlatProtocol();
        }

        else if(PPDistNode::calcMethod == DistanceCalculationMethod::HAVERSINE) {
            this->firstSPCalculated = false;
            this->initiateHaversineProtocol();
        }
    }

    else {

        // Munich
        this->latitude = 48.133814;
        this->longitude = 11.585436;

        if(PPDistNode::calcMethod == DistanceCalculationMethod::FLAT) {
            this->acceptFlatProtocol();
        }

        else if(PPDistNode::calcMethod == DistanceCalculationMethod::HAVERSINE) {
            this->firstSPCalculated = false;
            this->acceptHaversineProtocol();
        }
    }
}

void PPDistNode::handleMessage(cMessage* msg) {

    if(PPDistNode::calcMethod == DistanceCalculationMethod::FLAT) {
        if(this->ppspp != nullptr) {
            cMessage* response = this->ppspp->processMessage(msg);

            if(response != nullptr) {
                send(response, "out");
            }

            if(this->ppspp->isFinished()) {
                long result = this->ppspp->getResult();
                long sqrtResult = (long)(sqrt((double)(result)));
                double toDegrees = PPDistNode::intToGeo(sqrtResult);

                std::cout << "Flat distance between Munich and Budapest (in m): " << (PPDistNode::DEGREE_TO_METERS * toDegrees) << std::endl;
            }
        }
    }

    else if(PPDistNode::calcMethod == DistanceCalculationMethod::HAVERSINE) {
        if(this->ppspp != nullptr) {
            cMessage* response = this->ppspp->processMessage(msg);

            if(response != nullptr) {
                send(response, "out");
            }

            if(this->ppspp->isFinished()) {
                // only first half of vector is finished
                if(!this->firstSPCalculated) {
                    this->firsSPValue = this->ppspp->getResult();
                    if(this->ppspp->getRole() == PPSPNodeRole::INITIATOR) {
                        delete this->ppspp;
                        this->ppspp = PPScalarProductProtocol::initiate(this->vec2, 3, false);
                        delete this->vec2;

                        cMessage* initMessage = this->ppspp->processMessage(NULL);
                        send(initMessage, "out");
                    }
                    else if(this->ppspp->getRole() == PPSPNodeRole::ACCEPTOR) {
                        delete this->ppspp;
                        this->ppspp = PPScalarProductProtocol::accept(this->vec2, 3, false);
                        delete this->vec2;
                    }

                    std::cout << "first result: " << this->firsSPValue << std::endl;
                    this->firstSPCalculated = true;
                }

                // complete scalar product calculation is finished

                else {
                    std::cout << "second result: " << this->ppspp->getResult() << std::endl;
                    long scalarProduct = this->firsSPValue + this->ppspp->getResult();
                    delete this->ppspp;
                    double a = intSquaredToGeo(scalarProduct) / 2.0; // <--not sure about / 2.0, but it gives a better result..
                    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
                    double distSquared = pow(EARTH_RADIUS_IN_METERS * c, 2.0);
                    double distance = sqrt(distSquared);

                    std::cout << "distance (in m): " << (long)distance << std::endl;
                }
            }
        }
    }


    delete msg;
}

/*
    public double flatDistance(double lng1, double lat1, double lng2, double lat2){
        final double deglen = 110250.0; // length of a degree in meters
        double x = lat1 - lat2;
        double y = (lng1 - lng2) * Math.cos(lat2);
        return deglen * Math.sqrt(x*x + y*y);
    }

    this->latitude = 47.498295;
            this->longitude = 19.047212;


 this->latitude = 48.133814;
            this->longitude = 11.585436;
*/
void PPDistNode::initiateFlatProtocol() {
    long* vec = this->createFlatCalcVector(this->latitude, this->longitude);
    this->ppspp = PPScalarProductProtocol::initiate(vec, 2, true);
    delete[] vec;

    cMessage* request = this->ppspp->processMessage(NULL);
    send(request, "out");
}

void PPDistNode::acceptFlatProtocol() {
    long* vec = this->createFlatCalcVector(this->latitude, this->longitude);
    this->ppspp = PPScalarProductProtocol::accept(vec, 2, true);
    delete[] vec;
}

long* PPDistNode::createFlatCalcVector(double lat, double lng) {
    long* vec = new long[2];
    vec[0] = PPDistNode::geoToInt(lat);
    vec[1] = PPDistNode::geoToInt(lng * cos(lat * (PI / 180.0)));
    return vec;
}


//
// Haversine Distance calculation
// ==============================================================
// Original Jave code:
//
//public double haversineDistance(double lng1, double lat1, double lng2, double lat2) {
//    double latDistance = Math.toRadians(lat2 - lat1);
//    double lonDistance = Math.toRadians(lng2 - lng1);
//
//    double tmpSinLatDist = Math.sin(latDistance / 2.0);
//    double tmpSinLonDist = Math.sin(lonDistance / 2.0);
//
//    double a = tmpSinLatDist * tmpSinLatDist
//            + Math.cos(Math.toRadians(lat1)) * Math.cos(Math.toRadians(lat2))
//            * tmpSinLonDist * tmpSinLonDist;
//    double c = 2.0 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
//
//    double distance = Math.pow(EARTH_RADIUS_IN_METERS * c, 2);
//
//    return Math.sqrt(distance);
//}



void PPDistNode::initiateHaversineProtocol() {
    double latInRad = this->latitude * (PI / 180.0);
    double lngInRad = this->longitude * (PI / 180.0);

    double x = latInRad / 2.0;
    double y = lngInRad / 2.0;

    double cx = cos(x);
    double sx = sin(x);
    double cy = cos(y);
    double sy = sin(y);

    long* vec1 = new long[3];
    vec1[0] = geoToInt(cx * cx);
    vec1[1] = geoToInt(cx * sx * sqrt(2.0));
    vec1[2] = geoToInt(sx * sx);

    this->vec2 = new long[3];
    this->vec2[0] = geoToInt(cy * cy * cx);
    this->vec2[1] = geoToInt(cy * sy * cx * sqrt(2.0));
    this->vec2[2] = geoToInt(sy * sy * cx);

    std::cout << "Initiator vector: " << std::endl;
    std::cout << "[0]: " << vec1[0] << std::endl;
    std::cout << "[1]: " << vec1[1] << std::endl;
    std::cout << "[2]: " << vec1[2] << std::endl;
    std::cout << "[3]: " << this->vec2[0] << std::endl;
    std::cout << "[4]: " << this->vec2[1] << std::endl;
    std::cout << "[5]: " << this->vec2[2] << std::endl;

    this->ppspp = PPScalarProductProtocol::initiate(vec1, 3, false);

    delete[] vec1;

    cMessage* request = this->ppspp->processMessage(NULL);
    send(request, "out");
}

void PPDistNode::acceptHaversineProtocol() {
    double latInRad = this->latitude * (PI / 180.0);
    double lngInRad = this->longitude * (PI / 180.0);

    double x = latInRad / 2.0;
    double y = lngInRad / 2.0;

    double cx = cos(x);
    double sx = sin(x);
    double cy = cos(y);
    double sy = sin(y);

    long* vec1 = new long[3];
    vec1[0] = geoToInt(sx * sx);
    vec1[1] = geoToInt(-sx * cx * sqrt(2.0));
    vec1[2] = geoToInt(cx * cx);

    this->vec2 = new long[3];
    this->vec2[0] = geoToInt(sy * sy * cx);
    this->vec2[1] = geoToInt(-sy * cy * cx * sqrt(2.0));
    this->vec2[2] = geoToInt(cy * cy * cx);

    std::cout << "Acceptor vector: " << std::endl;
    std::cout << "[0]: " << vec1[0] << std::endl;
    std::cout << "[1]: " << vec1[1] << std::endl;
    std::cout << "[2]: " << vec1[2] << std::endl;
    std::cout << "[3]: " << this->vec2[0] << std::endl;
    std::cout << "[4]: " << this->vec2[1] << std::endl;
    std::cout << "[5]: " << this->vec2[2] << std::endl;

    this->ppspp = PPScalarProductProtocol::accept(vec1, 3, false);

    delete[] vec1;
}



long PPDistNode::geoToInt(double geo) {
    return (long)(100000000.0 * geo);
}

double PPDistNode::intToGeo(long integer) {
    return ((double)integer / (100000000.0));
}

double PPDistNode::intSquaredToGeo(long integer) {
    return ((double)integer / (100000000.0 * 100000000.0));
}








