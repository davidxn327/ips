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
#include "ns3/stats-module.h"

#include "device.h"

#define LINEMAX 512 

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("ips");

class IPS_Simulation
{
private:
	// variables
	vector<IPS_Switch *> switches;
	vector<IPS_Device *> devices;
	//int swlinks[2000];
	vector<IPS_Switch *> swlinks;//switch* <-> switch*
	
public:
	void Config(string configfile);
	void Topology();
	void InstallApplication();
	void Run();
	void MakeStats();
	void Destroy();

	IPS_Simulation();
};

IPS_Simulation::IPS_Simulation()
{}

void IPS_Simulation::Config(string configfile)
{
	//string configfile("scratch/ips/demo.txt")

	//cout<<sizeof(swlinks)<<std::endl;
	char line[LINEMAX];
	string type;
	std::ifstream ifs;
	map<string, IPS_Switch *> map;
	IPS_Switch *cs;

	ifs.open (configfile.c_str(), std::ifstream::in);
	while(ifs.getline(line, LINEMAX))
	{
		if(line[0]=='\n' || line[0]=='#' || line[0]==';' || (line[0]=='/'&&line[1]=='/'))
			continue;

		type = strtok(line, "\t");
		if(type == "Switch")
		{
			//type	id	
			string id = strtok(NULL, "\t");
			IPS_Switch *nSw = new IPS_Switch();
			nSw->id = id;
			switches.push_back(nSw);
			map.insert(std::pair<string, IPS_Switch *>(nSw->id, nSw));
			cs = nSw;
		}
		else if(type == "Device")
		{
			//type dev id tongdao rateMbps vlan
			string devname = strtok(NULL, "\t");
			string id = strtok(NULL, "\t");
			string tongdao = strtok(NULL, "\t");
			string rate = strtok(NULL, "\t");
			string vlan = strtok(NULL, "\t");
			IPS_Device *dev = new IPS_Device();

			dev->id = id;
			dev->type = devname;
			dev->lushu = atoi(tongdao.c_str());
			dev->rate = atoi(rate.c_str());
			dev->vlan = vlan;
			dev->sw = cs;
			dev->start = 1;
			dev->end = 15;
			cs->AddDevice(dev);

			devices.push_back(dev);

			if(devname == "MU")
			{}
			else if(devname == "IT")
			{}
			else if(devname == "PC")
			{}
			else if(devname == "FC")
			{}
			else if(devname == "MN")
			{}

		}
		else if(type == "Link")
		{
			string s0 = strtok(NULL, "\t");
			string s1 = strtok(NULL, "\t");

			swlinks.push_back(map[s0]);
			swlinks.push_back(map[s1]);
		}	
	}
}

void IPS_Simulation::Topology()
{
	int n, m, i, j;
	BridgeHelper bridge;
	NetDeviceContainer allDevices;
	NodeContainer allDevNodes;

	CsmaHelper csma;
	csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds(200)));

	n = switches.size();// the number of swithes
	NetDeviceContainer swInterfaces[n]; 

	for(i=0; i<n; ++i)
	{
		IPS_Switch *sw = switches[i];
		Ptr<Node> swNode = sw->node;
		//sw->SetNs3Node(&swNode);
		sw->index = i;

		m = sw->GetDeviceNum();
		for(j=0; j<m; ++j)
		{
			//Ptr<Node> devNode= CreateObject<Node> ();
			IPS_Device *dev = sw->GetDevice(j);
			Ptr<Node> devNode = dev->node;
			//dev->SetNs3Node(&devNode);

			//csma.SetChannelAttribute ("DataRate", DataRateValue(DataRate("1000Mbps")));
			csma.SetChannelAttribute ("DataRate", DataRateValue(dev->rate*1000*1000));
			NetDeviceContainer link = csma.Install(NodeContainer(swNode, devNode));

			swInterfaces[i].Add(link.Get(0));
			allDevices.Add(link.Get(1));
			allDevNodes.Add(devNode);
		}
	}

	m = swlinks.size()/2 ; // number of links between switches
	for(i=0; i<m; ++i)
	{
		IPS_Switch *s0 = swlinks[2*i];
		IPS_Switch *s1 = swlinks[2*i+1];
		//Ptr<Node> n0 = *((Ptr<Node> *)s0->GetNs3Node());
		Ptr<Node> n0 = s0->node;
		//Ptr<Node> n1 = *((Ptr<Node> *)s1->GetNs3Node());
		Ptr<Node> n1 = s1->node;

		NetDeviceContainer link = csma.Install(NodeContainer(n0, n1));

		swInterfaces[s0->index].Add(link.Get(0));
		swInterfaces[s1->index].Add(link.Get(1));

	}

	for(i=0; i<n; ++i)
	{
		IPS_Switch *sw = switches[i];
		Ptr<Node> sn = sw->node;
		bridge.Install(sn, swInterfaces[sw->index]);
	}

	InternetStackHelper internet;
	internet.Install(allDevNodes);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0", "255.255.255.0");
	ipv4.Assign(allDevices);

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


}

void IPS_Simulation::InstallApplication()
{
	int i;
	int n = devices.size();
	for(i=0; i<n; ++i)
	{
		devices[i]->InstallApplication();
	}
}

void IPS_Simulation::Run()
{
	cout<<"running"<<std::endl;
	Simulator::Run ();
}

void IPS_Simulation::Destroy()
{
	cout<<"destroy"<<std::endl;
	Simulator::Destroy();
}

void IPS_Simulation::MakeStats()
{
	cout<<"report"<<std::endl;
	int i;
	int n = devices.size();

	string outputfile = "IPS_REPORT.txt";
	std::ofstream ofs;
	ofs.open (outputfile.c_str(), std::ofstream::out | std::ofstream::trunc);

	for(i=0; i<n; ++i)
	{
		double mean = devices[i]->GetStats();
		ofs<< devices[i]->id << "\t" << mean << std::endl;
	}
	ofs.close();
}

// todo: modify bridge/model/bridge-net-device.cc to support vlan
int main(int argc, char **argv)
{
	Time::SetResolution (Time::NS);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

	string configfile = argv[1];
	//CommandLine cmd;
	//cmd.AddValue ("config", "path of ns3 config file for ips", configfile);
	
	cout<<"start"<<std::endl;
	IPS_Simulation ips;
	ips.Config(configfile);
	ips.Topology();
	ips.InstallApplication();
	//Simulator::Run ();
	//Simulator::Destroy();
	ips.Run();
	ips.MakeStats();
	ips.Destroy();
	return 0;
}
