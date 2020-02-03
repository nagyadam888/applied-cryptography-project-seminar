//
// Generated file, do not edit! Created by nedtool 5.6 from src/protocols/messages/Msg4.msg.
//

#ifndef __MSG4_M_H
#define __MSG4_M_H

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
 * Class generated from <tt>src/protocols/messages/Msg4.msg:16</tt> by nedtool.
 * <pre>
 * packet Msg4
 * {
 *     unsigned long result;
 * }
 * </pre>
 */
class Msg4 : public ::omnetpp::cPacket
{
  protected:
    unsigned long result;

  private:
    void copy(const Msg4& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Msg4&);

  public:
    Msg4(const char *name=nullptr, short kind=0);
    Msg4(const Msg4& other);
    virtual ~Msg4();
    Msg4& operator=(const Msg4& other);
    virtual Msg4 *dup() const override {return new Msg4(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual unsigned long getResult() const;
    virtual void setResult(unsigned long result);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const Msg4& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, Msg4& obj) {obj.parsimUnpack(b);}


#endif // ifndef __MSG4_M_H

