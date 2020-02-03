Introduction
============

The idea of this project is based on a previous project of mine, which requires to calculate distances between participants in a peer-2-peer network. The resulting distance is necessary in order to decide whether two participants are allowed to communicate with each other or not. As this said project is supposed to operate in public, however, participants may feel more comfortable staying anonymous rather than giving away there exact location to strangers. Note, that the distance has to be calculated even before two participants can decide to interact with each other. Therefore *this* project implements two methods to calculate distances between two parties without revealing their actual geo-location to one another. The respective methods differ from their accuracy and efficiency. E.g. on a flat 2D surface this means that the other participant could be anywhere on a circle around you (with radius d).


Mathematical background
=======================

The main concept relies on a privacy-preserving scalar product calculation. Specifically this means that one can compute \(P*Q, with P = (p1, ..., pn), Q = (q1, ..., qn) \) without revealing the vectors individual elements to the other party. For this, we use an additive homomorphic encryption scheme, e.g. \(E(m1 + m2) = E(m1) * E(m2)\). \(E(a * m) = E(m)^a\) holds additionally. To retrieve the scalar product (and distance) between P and Q, we base our implementation on the paper denoted in the materials folder.


Flat-earth-model
-----------------

When the participants are relatively near to each other (e.g. in the same city), it is often sufficient to represent the earth's surface as a flat plane. This way, we can use cartesian 2D coordinates as the vectors P and Q, respectively, and compute their distance to each other using the mentioned method. We usually optain GPS coordinates, though, and we therefore have to convert them into meters. GPS coordinates means, we have a longitude and a latitude given in degrees. Also, the longitude spans different amounts of meters depending on the latitude. Therefore, we get:

```
double flatEarthDistance(double lng1, double lat1, double lng2, double lat2) {

    p1 = rad(lat1);
    p2 = rad(lng1) * cos(lat1);

    q1 = rad(lat2);
    q2 = rad(lng2) * cos(lat2);

    |PQ| = pp_dist_protocol(...); // privacy-preserving distance calculation protocol. multiple messages between participants

    return |PQ| * DEGREES_TO_METERS;
}
```


Haversine-model
---------------------

When the participants are relatively far apart from each other (e.g. different countries or even continents), the flat earth model is often not sufficient enough anymore. The haversine model includes the earth's curvature into its calculations. Its pseudo-code looks like this:

```
    public double haversineDistance(double lng1, double lat1, double lng2, double lat2) {
        latDistance = rad(lat2 - lat1);
        lonDistance = rad(lng2 - lng1);

        tmpSinLatDist = Math.sin(latDistance / 2);
        tmpSinLonDist = Math.sin(lonDistance / 2);

        a = (tmpSinLatDist * tmpSinLatDist) +
            (cos(rad(lat1)) * cos(rad(lat2)) *
             tmpSinLonDist * tmpSinLonDist);

        c = 2 * atan2(sqrt(a), sqrt(1 - a));

        distSquared = pow(EARTH_RADIUS_IN_METERS * c, 2);

        return sqrt(distance);
    }
```

We see, that the two geo-locations are linked to each other in the calculations, which is a problem, as this destroys the whole purpose of this project. However, by analysing the operations we can seperate them and calculate the variable a by using a scalar product:

1) The first two lines are converting the subtracted geo-locations to radians. Luckily, this operation can be seperated: \(rad(x-y) = rad(x) - rad(y)\).
2) Same goes for the division by two inside of the sinus function: \((x-y) / 2 = x/2 - y/2 \).
3) Now, the actual sinus function has a convenient identity: \(sin(x-y) = sin(x)*cos(y) - cos(x)*sin(y)\).
4) By putting everything together, we obtain: 
 a = sin²(lat2)*cos²(lat1) - 2*sin(lat2)*cos(lat1)*cos(lat2)*sin(lat1) + cos²(lat2)*sin²(lat1) + sin²(lng2)*cos²(lng1)-2*sin(lng2)*cos(lng1)*cos(lng2)*sin(lng1) + cos²(lng2) * sin²(lng1) * cos(lat1)*cos(lat2)\)

This can be rewritten as a scalar product with 6-dimensional vectors:

 P = (c²(lat1),  
      c(lat1)s(lat1)sqrt(2), 
      s²(lat1), 
      c²(lng1)c(lat1),  
      c(lng1)s(lng1)c(lat1)sqrt(2), 
      s²(lng1)c(lat1))
      
 Q = (c²(lat2), 
      -c(lat2)s(lat2)sqrt(2), 
      s²(lat2), 
      c²(lng2)c(lat2), 
      -c(lng2)s(lng2)c(lat2)sqrt(2), 
      s²(lng2)c(lat2))

Hence, we can retrieve a with the privacy-preserving scalar product calculation protocol. The following calculations of the haversine function can be then computed seperately on each client device.



Implementation
=========================================

To implement this project, we used the OmNet++ framework, which is a powerful network simulator. It enables to write custom code on top of the managed eco-system using C++. It also offers a very powerful additional library called INET, which is able to simulate the entire ISO-OSI protocol stack, including technologies like WiFi 802.11. It can also emulate mobile participants. Even thouh these technologies were not used, they can be easily implemented to further test this implementation in a real-like environment.


testing_env.ned
----------------------------------------
.ned (network descriptor) files represent network to emulate (in a GUI). In this particular one, the project sets up a very basic testing network consisting of two participants that are directly connected to each other to test the two protocols. 


PPDistNode class
-----------------------------------------
This is the main entry point of the project. Each instance of this class represents a participant in the previously mentioned network. It extends omnetpp's cSimpleModule class, which act as entry points for the underlying framework using the method initialize() and handleMessage(cMessage* msg). The latter one is the heartbeat of the program and gets called every time a new network packet arrives to this particular client. Worth mentioning is also omnetpp's send(...) method to transmit a new packet over a particular connection.
To change the distance calculation method, simply change the classes static parameter called "calcMethod" in the classes header file. Valid values are FLAT and HAVERSINE.


protocols/PPScalarProductProtocol class
-----------------------------------------
This is the class, which implements the actual privacy-preserving scalar product calculation protocol. Note that its constructor is private. Instead one should use the two factory functions called 'initiate' and/or 'accept'. The first one creates an instance used for the client that initiates the protocol (Alice). The latter one creates an instance representing the client that accepts the protocol on the other side (Bob). The class wraps the entire protocol's message exchanges and internal state, so that the PPDistNode class can just conveniently call the processMessage(...) function whenever a new network packet arrives. The different network messages are defined in the /messages subdirectory and are represented by omnetpp's .msg files. This class also uses the homomorphic encryption scheme using an instance of the HomomorphicEncryption class, which gets covered in the following.


encryption/HomomorphicEncryption class
-----------------------------------------
This is a wrapper class, that eases the usage of the underlying paillier encryption and GMP libraries, as these are only offer quite atomic and small operations only, which one has to call over and over again.


encryption/paillier.h and encryption/paillier.cc
-----------------------------------------
This is the main encryption library. It implements various methods and structures to effectively use the paillier encryption scheme. It has functionality to generate public/private key-pairs and for encrypting/decrypting data with a specific public/private key. It also manages the conversion of library specific types into serializable representations of data (e.g. void* and char*). Internally, this uses the very powerful GMP library, which offers a rich set of integer number arithmetics, random number generation and, very importantely, big numbers.



Notes
-----------------------------------------
Unfortunately, there was a deeply nested problem, which I had no time to solve. There was a lot of undefined behaviour when the vector dimensions at the scalar product calculations exceeded 3. The program threw various different memory allocation/corruption/leakage errors and it was very hard to debug them. It even threw different errors when restarting the program without changing anything. The behaviour also varied depending on whether I've used the DEBUG or RELEASE build. I am still unsure about the root of these problems, but I guess it has something to do with the Paillier encryption / GMP libraries. As both of them are implemented in C instead of C++, they also use C-specific (and rather unsafe) memory management functions, e.g. malloc(...) and free(...) instead of new and delete. These two different methods can be incompatible when used on the same memory. If that is in fact the cause for the errors, it can either come from my side of the project, but also from omnetpp's underlying framework and its  possible incompatibility with C-style dependencies (or bad project settings from my side).

Workaround:
To still be able to compute the 6 dimensional scalar product used in the haversine formula, I simply run the protocol twice with 3D vectors each and add the results together afterwards. This leads to the same outcome.



