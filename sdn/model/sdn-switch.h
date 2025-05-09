#ifndef SDN_SWITCH_H
#define SDN_SWITCH_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/net-device.h"
#include "sdn-flow-table.h"
#include "sdn-controller.h"
#include "control-packet.h"
// #include "sdn-control-channel.h"

namespace ns3 {
namespace sdn {

class SDNController;
class SDNFlowTable;

class SDNSwitch : public Object
{
public:
  static TypeId GetTypeId(void);
  SDNSwitch();
  virtual ~SDNSwitch();

  void SetFlowTable(Ptr<SDNFlowTable> table);
  void AddDevice(Ptr<NetDevice> device);
  controlPacket GenerateControlPacket(Ptr<Packet> packet, controlType type);

  bool ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address &src, const Address &dst,
    ns3::NetDevice::PacketType packetType); // Main entry point 

  FlowEntry* InstallFlowEntry(FlowEntry entry);
  bool LookupFlow(Ptr<Packet> packet, FlowEntry*& matched);

  void ForwardPacket(Ptr<Packet> packet, FlowEntry* flow);
  
  std::vector<Ptr<NetDevice>> m_devices;
  Ptr<SDNController> m_controller;
private:
  Ptr<SDNFlowTable> m_flowTable;
};

} // namespace sdn
} // namespace ns3

#endif // SDN_SWITCH_H
