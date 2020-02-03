//
// Generated file, do not edit! Created by nedtool 5.6 from src/protocols/messages/Msg3.msg.
//

#ifndef __MSG3_M_H
#define __MSG3_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>src/protocols/messages/Msg3.msg:16</tt> by nedtool.
 * <pre>
 * packet Msg3
 * {
 *     unsigned long u;
 * }
 * </pre>
 */
class Msg3 : public ::omnetpp::cPacket
{
  protected:
    unsigned long u;

  private:
    void copy(const Msg3& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Msg3&);

  public:
    Msg3(const char *name=nullptr, short kind=0);
    Msg3(const Msg3& other);
    virtual ~Msg3();
    Msg3& operator=(const Msg3& other);
    virtual Msg3 *dup() const override {return new Msg3(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual unsigned long getU() const;
    virtual void setU(unsigned long u);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const Msg3& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, Msg3& obj) {obj.parsimUnpack(b);}


#endif // ifndef __MSG3_M_H

