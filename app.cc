#include "ns3/internet-module.h"
#include "ns3/seq-ts-header.h"

#include "app.h"

TypeId Sender::GetTypeId(void)
{
	static TypeId tid = TypeId ("Sender")
		.SetParent<Application> ()
		.AddConstructor<Sender> ();

	return tid;
}


Sender::Sender() :
	m_pktSize(64),
	m_destAddr("10.1.1.255"),
	m_destPort(APP_PORT),
	m_interval(0.5),
	m_socket(0),
	m_count(0),
	m_samples(12),
	m_vlan("vlan0")
{
	//m_socket = 0;
}

Sender::~Sender()
{
	//NS_LOG_FUNCTION_NOARGS ();
}

void Sender::DoDispose(void)
{
	m_socket = 0;
	// chain up
	Application::DoDispose ();
}

void Sender::SetVlan(string vlan)
{
	if(vlan == "")
		return;
	this->m_vlan = vlan;
}

void Sender::SetDevId(string devId)
{
	if(devId == "")
		return;
	this->m_devId = devId;
}

void Sender::StartApplication(void)
{
	if (m_socket == 0) 
	{
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		m_socket = Socket::CreateSocket (GetNode (), tid);
		m_socket->SetAllowBroadcast(true);
		m_socket->Bind();
		m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_destAddr), m_destPort));
	}

	//m_socket->SetRecvCallback (MakeCallback (&UdpEchoClient::HandleRead, this));

	m_count = 0;

	//Simulator::Cancel (m_sendEvent);
	m_sendEvent = Simulator::ScheduleNow (&Sender::SendPacket, this);
}

void Sender::StopApplication(void)
{
	Simulator::Cancel (m_sendEvent);
}

void Sender::SetSampleNum(uint32_t num)
{
	m_samples = num;
}

void Sender::SetType(string type)
{
	if(type == "MMS")
	{
		m_pktSize = 15;
		m_interval = 0.01;
	}
	else if(type == "SV")
	{
		m_pktSize = 98 + 2*m_samples;
		m_interval = 0.00025;
	}
	else // default: GOOSE
	{
		m_pktSize = 6016;
		m_interval = 10;
	}
	this->type = type;
}
void Sender::SetRemote(string addr)
{
	m_destAddr= Ipv4Address(addr.c_str());
}

void Sender::SendPacket()
{
	Ptr<Packet> packet = Create<Packet>(m_pktSize);

	TimestampTag timestamp;
	timestamp.SetTimestamp(Simulator::Now());
	packet->AddByteTag(timestamp);

	DeviceNameTag devTag;
	devTag.SetDeviceName(this->m_devId);
	packet->AddPacketTag(devTag);

	DeviceNameTag vlanTag;
	vlanTag.SetDeviceName(this->m_vlan);
	packet->AddPacketTag(vlanTag);

	//m_socket->SendTo(packet, 0, InetSocketAddress(m_destAddr, m_destPort));
	m_socket->Send(packet);

	//m_txTrace(packet);
	
	m_sendEvent = Simulator::Schedule (Seconds (rng.GetValue(m_interval)), &Sender::SendPacket, this);
                                         
	//cout<<Simulator::Now()<<" : "<<this->type<<" send a packet."<<std::endl;
}







//Receiver class
TypeId Receiver::GetTypeId(void)
{
	static TypeId tid = TypeId("Receiver")
		.SetParent<Application>()
		.AddConstructor<Receiver>()
		;

	return tid;
}

Receiver::Receiver() :
	m_socket(0),
	m_port(APP_PORT),
	m_reportInterval(0.05),
	m_reportNum(0),
	m_vlan("vlan0")
{
	m_loss = new PacketLossCounter(128);
	m_size.Reset();
	m_delay.Reset();
	m_delay.Update(0);
}

Receiver::~Receiver()
{
	delete m_loss;
}

void Receiver::DoDispose(void)
{
	m_socket = 0;
	Application::DoDispose();
}

void Receiver::SetVlan(string vlan)
{
	if(vlan == "")
		return;
	this->m_vlan = vlan;
}

void Receiver::SetDevId(string devId)
{
	if(devId == "")
		return;
	this->m_devId = devId;
}

void Receiver::StartApplication()
{
	if(m_socket == 0)
	{
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		m_socket = Socket::CreateSocket (GetNode (), tid);
		InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
		m_socket->Bind (local);
		if (addressUtils::IsMulticast (m_local))
		{
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
			if (udpSocket)
			{
				// equivalent to setsockopt (MCAST_JOIN_GROUP)
				udpSocket->MulticastJoinGroup (0, m_local);
			}
			else
			{
				NS_FATAL_ERROR ("Error: Failed to join multicast group");
			}
		}
	}

	//Simulator::Cancel (m_event);
	m_event = Simulator::Schedule (Seconds (m_reportInterval), &Receiver::Report, this);

	m_socket->SetRecvCallback(MakeCallback(&Receiver::Receive, this));
	//cout<<"receive socket ok"<<std::endl;

}

void Receiver::StopApplication()
{
	if(m_socket != 0)
	{
		m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
	}
	Simulator::Cancel (m_event);
}

void Receiver::Receive( Ptr<Socket> socket)
{
	//cout<<"enter receive "<<Simulator::Now()<<std::endl;
	Ptr<Packet> packet;
	Address from;
	while(packet = socket->RecvFrom(from))
	{
		SeqTsHeader seqTs;
		packet->RemoveHeader (seqTs);
		uint32_t currentSequenceNumber = seqTs.GetSeq ();
		if(InetSocketAddress::IsMatchingType(from))
		{
		}

		if(from == Ipv4Address::GetAny ())
		{
			continue;
		}

		DeviceNameTag devTag;
		if(packet->PeekPacketTag(devTag))
		{
			if(devTag.GetDeviceName() == this->m_devId)
			{
				continue;
			}
		}

		DeviceNameTag vlanTag;
		if(packet->PeekPacketTag(vlanTag))
		{
			if(vlanTag.GetDeviceName() != this->m_vlan)
			{
				continue;
			}
		}
		else
		{
			continue;
		}

		TimestampTag time;
		if(packet->FindFirstMatchingByteTag(time))
		{
			Time tx = time.GetTimestamp();
			Time dly = Simulator::Now() - tx;
			m_delay.Update(dly.GetMicroSeconds());
			m_calc.Update();
			m_size.PacketUpdate("", packet);
			m_loss->NotifyReceived (currentSequenceNumber);
		}
		//cout<<"receive "<<Simulator::Now()<<std::endl;
		//socket->SendTo (packet, 0, from);
	}
}

void Receiver::Report()
{
	//cout<<"report"<<Simulator::Now()<<std::endl;
	vec_size.push_back(m_size.getSum());
	m_size.Reset();

	vec_delay.push_back(m_delay.getMean());
	m_delay.Reset();
	m_delay.Update(0);

	vec_loss.push_back(m_loss->GetLost()*1.0/m_calc.GetCount());
	delete m_loss;
	m_loss = new PacketLossCounter(128);

	m_event = Simulator::Schedule (Seconds (m_reportInterval), &Receiver::Report, this);
}

//TimestampTag class
TypeId TimestampTag::GetTypeId(void)
{
	static TypeId tid = TypeId ("TimestampTag")
		.SetParent<Tag> ()
		.AddConstructor<TimestampTag> ()
		.AddAttribute ("Timestamp",
				"Some momentous point in time!",
				EmptyAttributeValue (),
				MakeTimeAccessor (&TimestampTag::GetTimestamp),
				MakeTimeChecker ())
		;
	return tid;
}


TypeId TimestampTag::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}

uint32_t TimestampTag::GetSerializedSize (void) const
{
	return 8;
}

void TimestampTag::Serialize (TagBuffer i) const
{
	int64_t t = m_timestamp.GetNanoSeconds ();
	i.Write ((const uint8_t *)&t, 8);
}

void TimestampTag::Deserialize (TagBuffer i)
{
	int64_t t;
	i.Read ((uint8_t *)&t, 8);
	m_timestamp = NanoSeconds (t);
}


void TimestampTag::SetTimestamp (Time time)
{
	m_timestamp = time;
}

Time TimestampTag::GetTimestamp (void) const
{
	return m_timestamp;
}


void TimestampTag::Print (std::ostream &os) const
{
	os << "t=" << m_timestamp;
}
