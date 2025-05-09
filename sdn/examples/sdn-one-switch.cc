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

n1 01 == 02 s1 04 == 03 n2

*/
int main(int argc, char *argv[])
{
    LogComponentEnable("SDNExample", LOG_LEVEL_INFO);
    LogComponentEnable("SDNController", LOG_LEVEL_INFO);
    LogComponentEnable("SDNSwitch", LOG_LEVEL_INFO);
    
    NodeContainer nodes;
    nodes.Create(2);

    NodeContainer switchNode;
    switchNode.Create(1);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer nodeDevices, switchDevices;
    for (uint32_t i = 0; i < nodes.GetN(); i++)
    {
        NetDeviceContainer link = csma.Install(NodeContainer(nodes.Get(i), switchNode.Get(0)));
        nodeDevices.Add(link.Get(0));
        switchDevices.Add(link.Get(1));
    }
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(nodeDevices);

    Ptr<SDNController> controller = CreateObject<SDNController>();
    controller->PopulateRoutingTable(nodes, switchNode, NetDeviceContainer(nodeDevices, switchDevices));
    controller->DisplayDetailedRoutingTable();
    Ptr<SDNSwitch> sdnSwitch = CreateObject<SDNSwitch>();
    Ptr<SDNFlowTable> flowTable = CreateObject<SDNFlowTable>();
    controller->AddSwitch(sdnSwitch);
    sdnSwitch->SetFlowTable(flowTable);
    for (uint32_t i = 0; i < switchDevices.GetN(); i++)
    {
        sdnSwitch->AddDevice(switchDevices.Get(i));
    }
    
    uint16_t port = 9;
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
    onoff.SetConstantRate(DataRate("5Kbps"));
    onoff.SetAttribute("StartTime", TimeValue(Seconds(1.0)));
    onoff.SetAttribute("StopTime", TimeValue(Seconds(3.0)));
    onoff.SetAttribute("PacketSize", UintegerValue(512));
    onoff.Install(nodes.Get(0));

    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    sink.Install(nodes.Get(1));

    csma.EnablePcapAll("sdn-one", true);
    Simulator::Stop(Seconds(6.0));
    Simulator::Run();
    Simulator::Destroy();
    
    flowTable->PrintFlowStats();
    return 0;
}