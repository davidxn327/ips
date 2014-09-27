#ifndef __IPS_DEVICE_H__
#define __IPS_DEVICE_H__

#include <iostream>
#include <vector>

#include "ns3/core-module.h"
#include "ns3/network-module.h"


using namespace std;
using namespace ns3;

class IPS_Switch;

class IPS_Device
{
private:
	// variables
	//void *node;
	//void *recApp;
	Ptr<Application> recApp;
public:
	int InstallApplication();
	double GetStats();

	int GetRate();

	//void *GetNs3Node();
	//int SetNs3Node(void *node); //&Ptr<>
	Ptr<Node> GetNs3Node();
	int SetNs3Node(Ptr<Node> node); //&Ptr<>

	IPS_Device();

	string id;
	string type;
	int lushu;//路数
	int rate;//Mbps
	string vlan;
	int start;//second
	int end;//sencond
	IPS_Switch *sw;

	//Ptr<Node> node = CreateObject<Node> ();
	Ptr<Node> node;
};

typedef struct _switchport
{
	string id;
	int rate; //Mbps
	IPS_Device *device;
}Switch_Port;

class IPS_Switch
{
private:
	// variables
	vector<Switch_Port *> ports;
public:
	int AddDevice(IPS_Device *device);
	IPS_Device *GetDevice(int num_port);
	int GetDeviceNum();

	//void *GetNs3Node();
	//int SetNs3Node(void *node); //&Ptr<>
	Ptr<Node> GetNs3Node();
	int SetNs3Node(Ptr<Node> node); //&Ptr<>

	IPS_Switch();
	string id;
	int index;

	//Ptr<Node> node = CreateObject<Node> ();
	Ptr<Node> node;
};


#endif//__IPS_DEVICE_H__
