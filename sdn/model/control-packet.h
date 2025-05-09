#ifndef CONTROL_PACKET_H
#define CONTROL_PACKET_H

namespace ns3 {
namespace sdn {

enum controlType {
  ARPPacket,            // Handle ARP resolution
  IPPacket,             // Forwarding decision for general IP packets
  FlowAdd,              // Add a new flow rule to a switch
  FlowDelete,           // Remove a flow rule
  FlowModify,           // Modify an existing flow rule
  LinkStateUpdate,      // Topology update notification
  StatsRequest,         // Controller asking switch for stats
  StatsReply,           // Switch replies with flow/table stats
  PacketIn,             // Packet arrived at switch with no matching rule
  PacketOut,            // Controller sends packet out a port
  SwitchJoin,           // A switch joins the network
  SwitchLeave,          // A switch disconnects or fails
  Hello,                // Initial handshake (e.g., between switch and controller)
  BarrierRequest,       // Synchronization barrier (for ordered message handling)
  BarrierReply,         // Response to BarrierRequest
  Error                 // Incomplete control packet
};

struct controlPacket {
    controlType type;
    Ipv4Address srcIp;
    Ipv4Address dstIp;
    uint8_t protocolNumber;
    uint16_t srcPort;
    uint16_t dstPort;
    Address srcMac;
    Address dstMac;
    FlowEntry* flow;
  };

} // namespace sdn
} // namespace ns3

#endif // CONTROL_PACKET_H
