#ifndef __APP_H__
#define __APP_H__


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/application.h"

#include "ns3/stats-module.h"
#include "ns3/packet-loss-counter.h"

#define APP_PORT 1603

using namespace ns3;
using namespace std;

// loss, delay, throughput
class Sender : public Application
{
public:
	static TypeId GetTypeId(void);
	Sender();
	virtual ~Sender();

	void SetRemote(string addr);
	void SendPacket ();
	void SetType(string type);
	void SetSampleNum(uint32_t num);
	void SetVlan(string vlan);
	void SetDevId(string devId);

protected:
	virtual void DoDispose(void);

private:
	virtual void StartApplication (void);
	virtual void StopApplication (void);

	uint32_t        m_pktSize;
	Ipv4Address     m_destAddr;
	uint32_t        m_destPort;
	double		m_interval;
	ConstantRandomVariable rng;

	Ptr<Socket>     m_socket;
	EventId         m_sendEvent;

	string type;	
	uint32_t        m_count;
	uint32_t	m_samples;//采样通道数
	string		m_vlan;
	string 		m_devId;
};// end class Sender 


class Receiver : public Application 
{
public:
	static TypeId GetTypeId (void);
	Receiver();
	virtual ~Receiver();

	void SetCounter (Ptr<CounterCalculator<> > calc);
	void SetDelayTracker (Ptr<TimeMinMaxAvgTotalCalculator> delay);
	void SetVlan(string vlan);
	void SetDevId(string devId);

	std::vector<double> vec_size;  //throughput
	std::vector<double> vec_delay; //delay
	std::vector<double> vec_loss;  //loss

protected:
	virtual void DoDispose (void);

private:
	virtual void StartApplication (void);
	virtual void StopApplication (void);

	void Receive (Ptr<Socket> socket);
	void Report (void);

	Ptr<Socket>     m_socket;
	Address m_local; //for local multicast address

	uint32_t        m_port;
	EventId         m_event;

	CounterCalculator<> m_calc;
	PacketSizeMinMaxAvgTotalCalculator m_size;
	//TimeMinMaxAvgTotalCalculator m_delay;
	MinMaxAvgTotalCalculator<int64_t> m_delay;
	PacketLossCounter *m_loss;

	double		m_reportInterval;
	uint32_t	m_reportNum;
	string		m_vlan;
	string 		m_devId;

};// end class Receiver


class TimestampTag : public Tag 
{
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;

	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);

	// these are our accessors to our tag structure
	void SetTimestamp (Time time);
	Time GetTimestamp (void) const;

	void Print (std::ostream &os) const;

private:
	Time m_timestamp;

};// end class TimestampTag


#endif// __APP_H__
