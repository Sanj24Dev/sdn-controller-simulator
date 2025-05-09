#ifndef SDN_FLOW_TABLE_H
#define SDN_FLOW_TABLE_H

#include <map>
#include <vector>
#include <string>
#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"


namespace ns3 {
namespace sdn {

struct FlowEntry {
  Ipv4Address srcIp;
  Ipv4Address dstIp;
  uint16_t srcPort;
  uint16_t dstPort;
  uint8_t protocol;
  Ptr<NetDevice> outputDevice;
  uint64_t packetCount = 0;
  uint64_t byteCount;
};

class SDNFlowTable : public Object
{
public:
  SDNFlowTable();
  ~SDNFlowTable();

  FlowEntry* AddFlowEntry(const FlowEntry& entry);
  bool RemoveFlowEntry(const FlowEntry& entry);
  FlowEntry* FindMatchingFlow(const Ipv4Address& srcIp, const Ipv4Address& dstIp,
                               uint16_t srcPort, uint16_t dstPort);
  void UpdateFlowStats(FlowEntry* entry, uint64_t packets, uint64_t bytes);
  void PrintFlowStats() const;
  std::vector<FlowEntry> GetAllEntries() const;
  
private:
  std::vector<FlowEntry> m_flowTable;
  
};

}
} // namespace ns3

#endif // SDN_FLOW_TABLE_H
