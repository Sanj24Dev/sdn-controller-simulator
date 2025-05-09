#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/openflow-module.h"
#include "ns3/applications-module.h"

#include "ns3/sdn-module.h"
using namespace ns3;
using namespace sdn;
NS_LOG_COMPONENT_DEFINE("SDNExample");

/*

n1-01 == 02-s1-03 == 04-s2-05 == 06-n2
                         |
                         07
                         ||
                         ||
                         08
                         |
                         n3
                         
*/
int main(int argc, char *argv[])
{
    LogComponentEnable("SDNExample", LOG_LEVEL_INFO);
    LogComponentEnable("SDNController", LOG_LEVEL_INFO);
    LogComponentEnable("SDNSwitch", LOG_LEVEL_INFO);
    
    NodeContainer nodes;
    nodes.Create(3);

    NodeContainer switchNodes;
    switchNodes.Create(2);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer nodeDevices, switchDevices;
    
    NetDeviceContainer link = csma.Install(NodeContainer(nodes.Get(0), switchNodes.Get(0)));
    nodeDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1));
    link = csma.Install(NodeContainer(switchNodes.Get(0), switchNodes.Get(1)));
    switchDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1)); 
    link = csma.Install(NodeContainer(switchNodes.Get(1), nodes.Get(1)));
    nodeDevices.Add(link.Get(1));
    switchDevices.Add(link.Get(0)); 
    link = csma.Install(NodeContainer(switchNodes.Get(1), nodes.Get(2)));
    nodeDevices.Add(link.Get(1));
    switchDevices.Add(link.Get(0));
    
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(nodeDevices);

    Ptr<SDNController> controller = CreateObject<SDNController>();
    controller->PopulateRoutingTable(nodes, switchNodes, NetDeviceContainer(nodeDevices, switchDevices));
    controller->DisplayDetailedRoutingTable();
    controller->DisplayARPResolution();
    Ptr<SDNSwitch> sdnSwitch1 = CreateObject<SDNSwitch>();
    Ptr<SDNSwitch> sdnSwitch2 = CreateObject<SDNSwitch>();
    Ptr<SDNFlowTable> flowTable1 = CreateObject<SDNFlowTable>();
    Ptr<SDNFlowTable> flowTable2 = CreateObject<SDNFlowTable>();
    controller->AddSwitch(sdnSwitch1);
    controller->AddSwitch(sdnSwitch2);
    sdnSwitch1->SetFlowTable(flowTable1);
    sdnSwitch2->SetFlowTable(flowTable2);
    sdnSwitch1->AddDevice(switchDevices.Get(0));
    sdnSwitch1->AddDevice(switchDevices.Get(1));
    sdnSwitch2->AddDevice(switchDevices.Get(2));
    sdnSwitch2->AddDevice(switchDevices.Get(3));
    sdnSwitch2->AddDevice(switchDevices.Get(4));
    
    uint16_t port = 9;
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("10.1.1.3"), port)));
    onoff.SetConstantRate(DataRate("5Kbps"));
    onoff.SetAttribute("StartTime", TimeValue(Seconds(1.0)));
    onoff.SetAttribute("StopTime", TimeValue(Seconds(2.0)));
    onoff.SetAttribute("PacketSize", UintegerValue(512));
    onoff.Install(nodes.Get(0));

    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    sink.Install(nodes.Get(2));

    csma.EnablePcapAll("sdn-two", true);
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();
    
    flowTable1->PrintFlowStats();
    flowTable2->PrintFlowStats();
    return 0;
}