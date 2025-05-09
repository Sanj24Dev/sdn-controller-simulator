#include "sdn-switch.h"
#include "ns3/log.h"

namespace ns3 {
namespace sdn {

NS_LOG_COMPONENT_DEFINE("SDNSwitch");

TypeId SDNSwitch::GetTypeId(void)
{
	static TypeId tid = TypeId("ns3::sdn::SDNSwitch")
		.SetParent<Object>()
		.SetGroupName("SDN")
		.AddConstructor<SDNSwitch>();
	return tid;
}

SDNSwitch::SDNSwitch() 
{
	NS_LOG_INFO("SDNSwitch: SDNSwitch created");
}

SDNSwitch::~SDNSwitch() 
{
	NS_LOG_INFO("SDNSwitch: Custom SDNSwitch destroyed");
}

void SDNSwitch::SetFlowTable(Ptr<SDNFlowTable> table)
{
	m_flowTable = table;
}

void SDNSwitch::AddDevice(Ptr<NetDevice> device) {
    m_devices.push_back(device);
	device->SetPromiscReceiveCallback(MakeCallback(&SDNSwitch::ReceivePacket, this));
}

controlPacket SDNSwitch::GenerateControlPacket(Ptr<Packet> packet, controlType type)
{
	controlPacket ctrl;
	if (type == ARPPacket)
	{
		Ptr<Packet> copy = packet->Copy();
		ArpHeader arpHeader;
		if (!copy->RemoveHeader(arpHeader))
		{
			NS_LOG_WARN("SDNSwitch: Failed to remove ARP header");
			ctrl.type = Error;
			return ctrl;
		}
		if (arpHeader.IsRequest())
    	{
			ctrl.srcIp = arpHeader.GetSourceIpv4Address();
			ctrl.dstIp = arpHeader.GetDestinationIpv4Address();
			ctrl.type = type;
		}
	}
	else if (type == FlowAdd)
	{
		Ipv4Header ipv4Header;
		if (!packet->RemoveHeader(ipv4Header))
		{
			NS_LOG_WARN("SDNSwitch: Failed to remove IPv4 header");
			ctrl.type = Error;
			return ctrl;
		}

		Ipv4Address srcIp = ipv4Header.GetSource();
		Ipv4Address dstIp = ipv4Header.GetDestination();
		uint8_t protocolNumber = ipv4Header.GetProtocol();

		uint16_t srcPort = 0;
		uint16_t dstPort = 0;

		if (protocolNumber == 6) // TCP
		{
			TcpHeader tcpHeader;
			if (!packet->RemoveHeader(tcpHeader))
			{
				NS_LOG_WARN("SDNController: Failed to remove TCP header");
				ctrl.type = Error;
				return ctrl;
			}
			srcPort = tcpHeader.GetSourcePort();
			dstPort = tcpHeader.GetDestinationPort();
		}
		else if (protocolNumber == 17) // UDP
		{
			UdpHeader udpHeader;
			if (!packet->RemoveHeader(udpHeader))
			{
				NS_LOG_WARN("SDNController: Failed to remove UDP header");
				ctrl.type = Error;
				return ctrl;
			}
			srcPort = udpHeader.GetSourcePort();
			dstPort = udpHeader.GetDestinationPort();
		}

		ctrl.type = type;
		ctrl.srcIp = srcIp;
		ctrl.dstIp = dstIp;
		ctrl.protocolNumber = protocolNumber;
		ctrl.srcPort = srcPort;
		ctrl.dstPort = dstPort;
	}

	return ctrl;
}


bool SDNSwitch::ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address &src, const Address &dst,
	ns3::NetDevice::PacketType packetType)
{
	NS_LOG_INFO("SDNSwitch: Packet received at SDN switch");
	if (protocol == 0x0806)		// ARP handling
	{
		Ptr<Packet> copy = packet->Copy();
		ArpHeader arpHeader;
		if (!copy->RemoveHeader(arpHeader))
		{
			NS_LOG_WARN("SDNSwitch: Failed to remove ARP header");
			return true;
		}
		if (arpHeader.IsRequest())
    	{
			
			Ipv4Address senderIp = arpHeader.GetSourceIpv4Address();
			Ipv4Address targetIp = arpHeader.GetDestinationIpv4Address();
			Address senderMac = src;

			NS_LOG_INFO("SDNSwitch: ARP Request from IP " << senderIp << " asking for " << targetIp);

			controlPacket ctrl = GenerateControlPacket(packet->Copy(), ARPPacket);
			if (ctrl.type == Error)
				exit(1);
			ctrl.srcMac = src;
			m_controller->HandlePacketIn(this, &ctrl, device);
			
			ArpHeader replyArp;
			replyArp.SetReply(ctrl.dstMac, targetIp, ctrl.srcMac, senderIp);
			EthernetHeader ethHeader;
			ethHeader.SetSource(Mac48Address::ConvertFrom(ctrl.dstMac));
			ethHeader.SetDestination(Mac48Address::ConvertFrom(ctrl.srcMac));
			ethHeader.SetLengthType(0x0806);

			Ptr<Packet> replyPacket = Create<Packet>();
			replyPacket->AddHeader(ethHeader);
			replyPacket->AddHeader(replyArp);
			
			device->Send(replyPacket, senderMac, protocol);
			NS_LOG_INFO("SDNSwitch: ARP Reply (" << ctrl.dstMac << ") sent to " << senderIp << " for target " << targetIp);
		}
		return true;
	}
	else		// FLow handling
	{
		FlowEntry *flow;
		if (LookupFlow(packet->Copy(), flow)) {
			NS_LOG_INFO("SDNSwitch: Flow matched. Forwarding...");
			ForwardPacket(packet->Copy(), flow);
		} else {
			NS_LOG_INFO("SDNSwitch: No flow match");
			controlPacket ctrl = GenerateControlPacket(packet->Copy(), FlowAdd);
			if (ctrl.type == Error)
				exit(1);
			m_controller->HandlePacketIn(this, &ctrl, device);
			ForwardPacket(packet->Copy(), ctrl.flow);
		}
	}
	
	return true;
}

FlowEntry* SDNSwitch::InstallFlowEntry(FlowEntry entry)
{
	FlowEntry *ptr = nullptr;
	if (m_flowTable) {
		ptr = m_flowTable->AddFlowEntry(entry);
	}
	return ptr;
}

bool SDNSwitch::LookupFlow(Ptr<Packet> packet, FlowEntry*& matched)
{
	if (!m_flowTable) return false;

	Ipv4Header ipv4Header;
	TcpHeader tcpHeader;
	UdpHeader udpHeader;
	std::string protocol;

	// Copy the packet to avoid modifying the original
	Ptr<Packet> copy = packet->Copy();

	// Extract the IP header
	if (!copy->RemoveHeader(ipv4Header)) return false;

	Ipv4Address srcIp = ipv4Header.GetSource();
	Ipv4Address dstIp = ipv4Header.GetDestination();

	uint16_t srcPort = 0;
	uint16_t dstPort = 0;

	switch (ipv4Header.GetProtocol()) {
		case 6: // TCP
			if (!copy->RemoveHeader(tcpHeader)) return false;
			srcPort = tcpHeader.GetSourcePort();
			dstPort = tcpHeader.GetDestinationPort();
			protocol = "TCP";
			break;
		case 17: // UDP
			if (!copy->RemoveHeader(udpHeader)) return false;
			srcPort = udpHeader.GetSourcePort();
			dstPort = udpHeader.GetDestinationPort();
			protocol = "UDP";
			break;
		default:
			return false; // Unsupported protocol
	}
	NS_LOG_INFO("SDNSwitch: Looking up: " << srcIp << ":" << srcPort << " -> " << dstIp << ":" << dstPort);
	FlowEntry* entry = m_flowTable->FindMatchingFlow(srcIp, dstIp, srcPort, dstPort);
	if (entry) {
		matched = entry;
		return true;
	}

	return false;
}

void SDNSwitch::ForwardPacket(Ptr<Packet> packet, FlowEntry* flow)
{
	Ptr<NetDevice> device = flow->outputDevice;
	Address addr = device->GetAddress();
	device->Send(packet, addr, 0x0800);
	m_flowTable->UpdateFlowStats(flow, 1, packet->GetSize());
	NS_LOG_INFO("SDNSwitch: Forwarding from " << flow->srcIp 
			<< " to " << flow->dstIp 
			<< " at output port " << addr);
}












} // namespace sdn
} // namespace ns3
