#include "device.h"
#include "app.h"
#include "ns3/applications-module.h"

// IPS_Switch begin 

IPS_Switch::IPS_Switch():
	id("unregisted")
{
	node = CreateObject<Node> ();
}

int IPS_Switch::AddDevice(IPS_Device *device)
{
	Switch_Port *port = new Switch_Port();
	port->id = "";
	port->rate = device->GetRate();
	port->device = device;
	this->ports.push_back(port);
	return 0;
}

IPS_Device *IPS_Switch::GetDevice(int num_port)
{
	return this->ports[num_port]->device;
}

int IPS_Switch::GetDeviceNum()
{
	return this->ports.size();
}
/*
void *IPS_Switch::GetNs3Node()
{
	return this->node;
}

int IPS_Switch::SetNs3Node(void *node)
{
	this->node = node;
	return 0;
}
*/
Ptr<Node> IPS_Switch::GetNs3Node()
{
	return this->node;
}

int IPS_Switch::SetNs3Node(Ptr<Node> node)
{
	this->node = node;
	return 0;
}

// IPS_Switch end



// IPS_Device begin

IPS_Device::IPS_Device():
	recApp(NULL),
	id("unregisted device"),
	type("unknown type"),
	lushu(1),
	rate(100),
	start(0),
	end(10),
	sw(NULL)
{
	node = CreateObject<Node> ();
}

int IPS_Device::GetRate()
{
	return this->rate;
}

int IPS_Device::InstallApplication()
{
	//Ptr<Node> node= *((Ptr<Node> *)this->GetNs3Node());
	Ptr<Node> node= this->node;
if(0)
{
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (node);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
	this->recApp = serverApps.Get(0);

  UdpEchoClientHelper echoClient (Ipv4Address("10.1.1.3"), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (node);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
	return 0;
}
	int goose=0, sv=0, mms=0;

	if(type == "MU") //合并单元
	{
		goose = 1;
		sv = 1;
	}
	else if(type == "IT") //智能终端
	{
		goose = 1;
	}
	else if(type == "PC") //保测
	{
		mms = 1;
		goose = 1;
	}
	else if(type == "FC") //多功能测控
	{
		mms = 1;
		goose = 1;
		sv = 1;
	}
	else //if(type == "MN") //采集装置
	{
		mms = 1;
	}
	
	// sender
	if (mms)
	{
		Ptr<Sender> sender = CreateObject<Sender>();
		sender->SetDevId(this->id);
		sender->SetVlan(this->vlan);
		sender->SetType("MMS");
		node->AddApplication (sender);
		sender->SetStartTime (Seconds (this->start));
		sender->SetStopTime (Seconds (this->end));
	}

	if (goose)
	{
		Ptr<Sender> sender = CreateObject<Sender>();
		sender->SetDevId(this->id);
		sender->SetVlan(this->vlan);
		sender->SetType("GOOSE");
		node->AddApplication (sender);
		sender->SetStartTime (Seconds (this->start));
		sender->SetStopTime (Seconds (this->end));
	}

	if (sv)
	{
		Ptr<Sender> sender = CreateObject<Sender>();
		sender->SetDevId(this->id);
		sender->SetVlan(this->vlan);
		sender->SetType("SV");
		node->AddApplication (sender);
		sender->SetStartTime (Seconds (this->start));
		sender->SetStopTime (Seconds (this->end));
	}

	// receiver
	Ptr<Receiver> receiver = CreateObject<Receiver>();
	receiver->SetDevId(this->id);
	receiver->SetVlan(this->vlan);
	node->AddApplication (receiver);
	receiver->SetStartTime (Seconds (0));
	receiver->SetStopTime (Seconds (this->end+0.001));

	this->recApp = receiver;

	return 0;
}

double IPS_Device::GetStats()
{
	Ptr<Receiver> app = DynamicCast<Receiver>(this->recApp);
	int n = app->vec_delay.size();
	int i;

	string outputfile = "IPS_"+this->id+".txt";
	std::ofstream ofs;
	ofs.open (outputfile.c_str(), std::ofstream::out | std::ofstream::trunc);
	cout<<this->id<<":"<<std::endl;
	
	double sum = 0;
	for(i=0; i<n; ++i)
	{
		if(i%10 == 0)
			cout<<i<<"~"<<i+9<<":\t";
		//cout<<app->vec_size[i]<<"|";
		cout<<app->vec_loss[i]<<"|";
		cout<<app->vec_delay[i]<<"\t";
		if(i%10 == 9)
			cout<<std::endl;

		//ofs << i << "\t" << app->vec_delay[i] << std::endl; 
		ofs << app->vec_delay[i] << std::endl; 
		sum += app->vec_delay[i];
	}
	ofs.close();
	cout<<std::endl;

	return sum/i;
}
/*
void *IPS_Device::GetNs3Node()
{
	return this->node;
}

int IPS_Device::SetNs3Node(void *node)
{
	this->node = node;
	return 0;
}
*/
Ptr<Node> IPS_Device::GetNs3Node()
{
	return this->node;
}

int IPS_Device::SetNs3Node(Ptr<Node> node)
{
	this->node = node;
	return 0;
}


// IPS_Device end 
