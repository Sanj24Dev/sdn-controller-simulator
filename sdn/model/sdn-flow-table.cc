#include "sdn-flow-table.h"

namespace ns3 {
namespace sdn {

NS_LOG_COMPONENT_DEFINE("SDNFlowTable");

SDNFlowTable::SDNFlowTable() 
{
	NS_LOG_INFO("SDNFlowTable: SDNFlowTable created");
}

SDNFlowTable::~SDNFlowTable()
{
	NS_LOG_INFO("SDNFlowTable: SDNFlowTable destroyed");
}

FlowEntry* SDNFlowTable::AddFlowEntry(const FlowEntry& entry) 
{
	// Store internal NS3 flow
	m_flowTable.push_back(entry);

	return &m_flowTable.back();
}

bool SDNFlowTable::RemoveFlowEntry(const FlowEntry& entry) 
{
	for (auto it = m_flowTable.begin(); it != m_flowTable.end(); ++it) 
	{
		if (it->srcIp == entry.srcIp && it->dstIp == entry.dstIp &&
			it->srcPort == entry.srcPort && it->dstPort == entry.dstPort &&
			it->protocol == entry.protocol) 
		{
			m_flowTable.erase(it);
			return true;
		}
	}
  	return false;
}

FlowEntry* SDNFlowTable::FindMatchingFlow(const Ipv4Address& srcIp, const Ipv4Address& dstIp,
											  uint16_t srcPort, uint16_t dstPort) 
{
	for (auto& entry : m_flowTable) 
	{
		if (entry.srcIp == srcIp && entry.dstIp == dstIp &&
			entry.srcPort == srcPort && entry.dstPort == dstPort) 
		{
			return &entry;
		}
	}
	return nullptr;
}

void SDNFlowTable::UpdateFlowStats(FlowEntry* entry, uint64_t packets, uint64_t bytes) 
{
	if (entry) 
	{
		entry->packetCount += packets;
		entry->byteCount += bytes;
	}
}

void SDNFlowTable::PrintFlowStats() const 
{
  std::cout << "=== SDN Flow Table Statistics ===\n";
  for (const auto& flow : m_flowTable) 
  {
    std::cout << "Flow: " 
              << flow.srcIp << ":" << flow.srcPort 
              << " -> " 
              << flow.dstIp << ":" << flow.dstPort 
			  << " through port device " << flow.outputDevice->GetAddress()
              << "\n  Packets: " << flow.packetCount 
              << "\n  Bytes:   " << flow.byteCount 
              << std::endl;
  }
}

std::vector<FlowEntry> SDNFlowTable::GetAllEntries() const 
{
  	return m_flowTable;
}

} // namespace sdn
} // namespace ns3
