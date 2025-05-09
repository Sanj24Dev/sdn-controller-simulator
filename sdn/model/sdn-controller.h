#ifndef SDN_CONTROLLER_H
#define SDN_CONTROLLER_H
#include "ns3/openflow-interface.h"
#include "ns3/openflow-switch-net-device.h"
#include "ns3/sdn-switch.h"
#include "control-packet.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/packet.h"
#include <vector>
#include <map>
#include <queue>
namespace ns3 {
namespace sdn {

class SDNSwitch;

// Structure to represent network links
struct Link {
    uint32_t neighborId;
    Ptr<NetDevice> outgoingPort;
    uint32_t weight;  // Can be hop count (1) or another metric
};

struct graphNode {
    uint32_t id;
    Ptr<NetDevice> device;  // Device on source node for this link
    Address macAddr;
    Ipv4Address ipAddr;

    bool operator<(const graphNode& other) const {
        return id < other.id;  // Compare by node ID (extend if needed)
    }
};

class SDNController : public Object //public ns3::ofi::Controller
{
public:
    SDNController();
    virtual ~SDNController();

    void PopulateRoutingTable(const NodeContainer& nodes, const NodeContainer& switches, const NetDeviceContainer& devices);
    Ptr<NetDevice> GetNextHop(Ptr<NetDevice> currDevice, Ipv4Address dstIp);
    
    void DisplayDetailedRoutingTable() const;
    void DisplayARPResolution();

    void AddSwitch(Ptr<SDNSwitch> swtch);

    Address AddressResolution(Ipv4Address ipAddr);
    void HandlePacketIn (Ptr<SDNSwitch> m_switch, controlPacket* ctrl, Ptr<NetDevice> device);
    void SendPacketOut (Ptr<SDNSwitch> swtch, Ptr<Packet> packet, Ptr<NetDevice> dev);

    static ns3::TypeId GetTypeId();
    
private:
    std::vector<Ptr<SDNSwitch>> m_switches;
    std::map<graphNode, std::vector<Link>> networkTopologyGraph;
    std::map<std::pair<uint32_t, uint32_t>, Ptr<NetDevice>> m_routingTable;
    std::map<Ipv4Address, Address> arpTable;
};

}
}
#endif


// the graph will have node and its links information
// the routing table maps the src and dest ip to the net dev