#ifndef PP_DIST_NODE_H
#define PP_DIST_NODE_H

#include <omnetpp.h>

#include "protocols/PPScalarProductProtocol.h"

enum DistanceCalculationMethod {
    TEST,
    FLAT,
    HAVERSINE
};


class PPDistNode : public omnetpp::cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage* msg) override;


    void initiateFlatProtocol();
    void acceptFlatProtocol();
    long* createFlatCalcVector(double lat, double lng);


    void initiateHaversineProtocol();
    void acceptHaversineProtocol();


    // Helper
    static long int geoToInt(double geo);
    static double intToGeo(long integer);
    static double intSquaredToGeo(long integer);

private:
    static const DistanceCalculationMethod calcMethod = FLAT;
    static constexpr double DEGREE_TO_METERS = 110250.0;
    static constexpr double EARTH_RADIUS_IN_METERS = 6371000.0;

    double latitude, longitude;

    PPScalarProductProtocol* ppspp;


    // Helper variables for haversine distance calculation
    long* vec2;
    bool firstSPCalculated;
    long firsSPValue;
};

// Register module to OmNet
Define_Module(PPDistNode);

#endif // PP_DIST_NODE_H
