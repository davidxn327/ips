#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

struct NodeST
{
	int id;
	char name[32];
	char type[8];
	int param;
	Ptr<Node> node;
	NetDeviceContainer *cards;
};

int main(int argc, char const *argv[])
{
	if (argc < 2)
	{
		exit(0);
	}

	std::ifstream ifs(argv[1]);
	char line[256];

	std::map<int, struct NodeST *> map;
	NodeContainer *terminals = new NodeContainer();
	//Ptr<NetDeviceContainer> terminalcards = CreateObject<NetDeviceContainer> ();
	NetDeviceContainer *terminalcards = new NetDeviceContainer();
	NodeContainer con;
	con.Create(1);
	while(1)
	{
		struct NodeST *node = (struct NodeST *)malloc(sizeof(*node));

		ifs.getline(line, 256);
		if (ifs.eof() || (line[0]=='-'))
		{
			break;
		}
		if (line[0]=='\n' || line[0]=='\r' || line[0]=='/' || line[0]==0)
		{
			continue;
		}
		sscanf(line, "%d %s %s %d", &node->id, node->name, node->type, &node->param);
		//std::cout << "id: " << node->id
		//	  << "; name: " << node->name
		//	  << "; type: " << node->type
		//	  << "; param: " << node->param 
		//	  << std::endl;

		Ptr<Node> objNode = CreateObject<Node>();
		node->node = objNode;

		if (strcmp(node->type, "SW")==0)
		{
			//std::cout << "this is switch." << std::endl;
			NetDeviceContainer *cards = new NetDeviceContainer();
			node->cards = cards;
		}
		else
		{
			//std::cout << "this is node: " << (node->node)->GetId() << std::endl;
			terminals->Add(node->node);
			node->cards = terminalcards;
		}
		//map.insert(node->id, node);
		map[node->id] = node;
	}

	CsmaHelper csma;
	csma.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (2)));
	
	LogComponentEnable("DropTailQueue", LOG_LEVEL_LOGIC);
	//ns3::CsmaNetDevice MacTxDrop
	//AsciiTraceHelper asciiTraceHelper;
	//Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("IPS.drop");
	//ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
	//void DropTraceFun(Ptr<const Packet>)
	//Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
	//  MakeCallback (&CourseChangeCallback));
	//
	//CsmaNetDevice::GetQueue
	//DropTailQueue
	//MaxPackets: DropTailQueue::m_maxPackets=100
	//MaxBytes: DropTailQueue::m_maxBytes=100*65535
	//csma.SetQueue("ns3::DropTailQueue",
	//		"MaxPackets", UintegerValue (300),
	//		"MaxBytes", UintegerValue (300 * 65535));
	while(1) {
		int id1, id2, rate;
		char vlan[64];

		ifs.getline(line, 256);
		if (ifs.eof())
		{
			break;
		}
		if (line[0]=='\n' || line[0]=='\r' || line[0]=='/' || line[0]==0)
		{
			continue;
		}
		sscanf(line, "%d %d %d %s", &id1, &id2, &rate, vlan);
		//std::cout << "id1: " << id1
		//	  << "; id2: " << id2 
		//	  << "; rate: " << rate 
		//	  << "; vlan: " << vlan 
		//	  << std::endl;

		//char strRate[32];
		//sprintf(strRate, "%dMb/s", rate);
		//csma.SetChannelAttribute ("DataRate", DataRate (strRate));//DataRateValue (rate*1000000));
		csma.SetChannelAttribute ("DataRate", DataRateValue (rate*1024*1024));

		//std::cout << "link 0 1:" << std::endl;
		//std::cout << id1 << " : " << (map[id1]->node)->GetId() << std::endl;
		//std::cout << id2 << " : " << (map[id2]->node)->GetId() << std::endl;

		NetDeviceContainer link = csma.Install ( NodeContainer(map[id1]->node, map[id2]->node) );
		map[id1]->cards->Add(link.Get(0));
		map[id2]->cards->Add(link.Get(1));
	}

	BridgeHelper bridge;
	for (std::map<int, struct NodeST *>::iterator it=map.begin(); it!=map.end(); ++it)
	{
		//std::cout << "map iteration: " << it->first << std::endl;
		if (strcmp(it->second->type, "SW")==0)
		{
			bridge.Install (it->second->node, *it->second->cards);
		}
	}

	std::cout << "terminal count: " << terminals->GetN() << std::endl;
	InternetStackHelper internet;
	internet.Install (*terminals);

	std::cout << "terminalcards count: " << terminalcards->GetN() << std::endl;
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	ipv4.Assign (*terminalcards);
	
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	int port = 1234;
	int endTime = 3;
	OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address ("10.1.1.255"), port)));
	PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	//onoff.Install(con.Get(0));
	//sink.Install(con.Get(0));

	for (std::map<int, struct NodeST *>::iterator it=map.begin(); it!=map.end(); ++it)
	{
		if (strcmp(it->second->type, "SW")==0)
		{
			continue;
		}
		else if (strcmp(it->second->type, "MU")==0)
		{
			std::cout << "install mu" << std::endl;
			int pktSize = 42 + 80 * it->second->param;//98 + 2*it->second->param;
			double rate = 8.0*pktSize/0.00025;
			std::cout << "mu  pktsize: " << pktSize << " rate: " << rate << std::endl;
			//onoff.SetAttribute("PacketSize", UintegerValue(pktSize));
			//onoff.SetConstantRate (DataRate ("500kb/s"));
			onoff.SetConstantRate (DataRate (rate), pktSize);

			ApplicationContainer app = onoff.Install (it->second->node);
			app.Start (Seconds (1.0));
			app.Stop (Seconds (endTime));
		}
		else if (strcmp(it->second->type, "IED")==0)
		{
			std::cout << "install ied" << std::endl;
			ApplicationContainer sink1 = sink.Install (it->second->node);
			sink1.Start (Seconds (1.0));
			sink1.Stop (Seconds (endTime));

			//GOOSE
			int pktSize = 6016;
			double rate = 8.0*pktSize/1;
			//double rate = 8.0*pktSize/5;
			std::cout << "mu  pktsize: " << pktSize << " rate: " << rate << std::endl;
			onoff.SetConstantRate (DataRate (rate), pktSize);

			ApplicationContainer app = onoff.Install (it->second->node);
			app.Start (Seconds (1.2));
			app.Stop (Seconds (endTime));

			if (0) {
				//fault happens
				pktSize = 6016;
				rate = 8.0*pktSize/0.002;
				std::cout << "mu  pktsize: " << pktSize << " rate: " << rate << std::endl;
				onoff.SetConstantRate (DataRate (rate), pktSize);

				onoff.SetAttribute("MaxBytes", UintegerValue(pktSize*4));

				app = onoff.Install (it->second->node);
				app.Start (Seconds (2.0));
				app.Stop (Seconds (endTime));

				onoff.SetAttribute("MaxBytes", UintegerValue(0));
			}

		}
		else
		{
			std::cout << "install pc" << std::endl;
			int pktSize = 512;
			double rate = it->second->param *1024*1024;
			std::cout << "pc  pktsize: " << pktSize << " rate: " << rate << std::endl;
			//onoff.SetAttribute("PacketSize", UintegerValue(pktSize));
			onoff.SetConstantRate (DataRate (rate), pktSize);

			ApplicationContainer app = onoff.Install (it->second->node);
			app.Start (Seconds (1.0));
			app.Stop (Seconds (endTime));
		}

		std::cout << it->second->name << ": "
		          << it->second->node->GetId() << ": "
			  << it->second->cards->Get(0)->GetAddress() << std::endl;

	}

	AsciiTraceHelper ascii;
	csma.EnableAsciiAll (ascii.CreateFileStream ("IPS.tr"));

	//-<nodeId>-<interfaceId>.pcap, by the "tcpdump -r" command (use "-tt")
	//csma.EnablePcapAll ("IPS", false);

	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}

