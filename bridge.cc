
#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

#include "app.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmaBridgeOneHopExample");

int 
main0123 (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  //LogComponentEnable ("CsmaBridgeOneHopExample", LOG_LEVEL_ALL);

  NS_LOG_INFO ("Create nodes.");

  Ptr<Node> n0 = CreateObject<Node> ();
  Ptr<Node> n1 = CreateObject<Node> ();
  Ptr<Node> n2 = CreateObject<Node> ();
  Ptr<Node> n3 = CreateObject<Node> ();
  Ptr<Node> n4 = CreateObject<Node> ();

  Ptr<Node> bridge1 = CreateObject<Node> ();
  Ptr<Node> bridge2 = CreateObject<Node> ();

  NS_LOG_INFO ("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  NetDeviceContainer lan;

  NetDeviceContainer topLanDevices;
  NetDeviceContainer topBridgeDevices;

  NodeContainer topLan (n0, n1);

  for (int i = 0; i < 2; i++)
    {
      // install a csma channel between the ith toplan node and the bridge node
      NetDeviceContainer link = csma.Install (NodeContainer (topLan.Get (i), bridge1));
      topLanDevices.Add (link.Get (0));
      topBridgeDevices.Add (link.Get (1));
      lan.Add (link.Get (0));
    }
  NetDeviceContainer blink = csma.Install(NodeContainer(bridge1, bridge2));
  topBridgeDevices.Add(blink.Get(0));

  BridgeHelper bridge;
  bridge.Install (bridge1, topBridgeDevices);

  // Add internet stack to the router nodes
  NodeContainer routerNodes (n0, n1, n3, n4);
  InternetStackHelper internet;
  internet.Install (routerNodes);

  // Repeat for bottom bridged LAN
  NetDeviceContainer bottomLanDevices;
  NetDeviceContainer bottomBridgeDevices;
  bottomBridgeDevices.Add(blink.Get(1));
  NodeContainer bottomLan (n3, n4);
  for (int i = 0; i < 2; i++)
    {
      NetDeviceContainer link = csma.Install (NodeContainer (bottomLan.Get (i), bridge2));
      bottomLanDevices.Add (link.Get (0));
      bottomBridgeDevices.Add (link.Get (1));
      lan.Add (link.Get (0));
    }
  bridge.Install (bridge2, bottomBridgeDevices);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  //ipv4.Assign (lan);
  Ipv4InterfaceContainer interfaces = ipv4.Assign (lan);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  if(0)
  {
	  UdpEchoServerHelper echoServer (9);

	  ApplicationContainer serverApps = echoServer.Install (n4);
	  serverApps.Start (Seconds (1.0));
	  serverApps.Stop (Seconds (10.0));

	  //UdpEchoClientHelper echoClient (interfaces.GetAddress (3), 9);
	  UdpEchoClientHelper echoClient (Ipv4Address("10.1.1.255"), 9);
	  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
	  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

	  ApplicationContainer clientApps = echoClient.Install (n0);
	  clientApps.Start (Seconds (2.0));
	  clientApps.Stop (Seconds (10.0));
  }
  else
  {
	  Ptr<Sender> sender = CreateObject<Sender>();
	  sender->SetType("GOOSE");
	  sender->SetRemote("10.1.1.255");
	  n0->AddApplication (sender);
	  sender->SetStartTime (Seconds (1));
	  sender->SetStopTime (Seconds (10));

	  Ptr<Receiver> receiver = CreateObject<Receiver>();
	  n4->AddApplication (receiver);
	  receiver->SetStartTime (Seconds (0));
	  receiver->SetStopTime (Seconds (10));
  }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
