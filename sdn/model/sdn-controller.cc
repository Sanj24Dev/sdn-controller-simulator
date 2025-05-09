#include "sdn-controller.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
extern "C" {
    #include "openflow/openflow.h"
}

namespace ns3{
namespace sdn{

NS_LOG_COMPONENT_DEFINE("SDNController");

TypeId SDNController::GetTypeId()
{
  static TypeId tid = TypeId("ns3::sdn::SDNController")
    .SetParent<ns3::ofi::Controller>()
    .SetGroupName("SDN")
    .AddConstructor<SDNController>();
  return tid;
}

SDNController::SDNController()
{
    NS_LOG_INFO("Custom SDNController created");
}

SDNController::~SDNController()
{
    NS_LOG_INFO("Custom SDNController destroyed");
}

// Shortest path computation using Dijkstra's algorithm
std::map<uint32_t, uint32_t> ComputeShortestPath(const std::map<graphNode, std::vector<Link>>& graph, uint32_t sourceNode) 
{    
    // Priority queue for Dijkstra's algorithm
    std::priority_queue<std::pair<int, uint32_t>,
                        std::vector<std::pair<int, uint32_t>>,
                        std::greater<std::pair<int, uint32_t>>> pq;
    
    // Distance and previous node maps
    std::map<uint32_t, uint32_t> distance;
    std::map<uint32_t, uint32_t> previous;
    
    // Initialize distances to infinity
    for (const auto& node : graph) {
        distance[node.first.id] = std::numeric_limits<uint32_t>::max();
        previous[node.first.id] = 0;  // 0 means no previous node
    }
    
    // Source node has distance 0
    distance[sourceNode] = 0;
    pq.push(std::make_pair(0, sourceNode));
    
    // Dijkstra's algorithm
    while (!pq.empty()) {
        uint32_t currentNode = pq.top().second;
        int currentDistance = pq.top().first;
        pq.pop();
        
        // Skip if we found a shorter path already
        if (currentDistance > distance[currentNode]) continue;
        
        // Find currentNode inside graph
        for (const auto& node : graph) {
            if (node.first.id == currentNode) {
                for (const auto& link : node.second) {
                    uint32_t neighbor = link.neighborId;
                    uint32_t weight = link.weight;
                    uint32_t distanceToNeighbor = currentDistance + weight;
                    
                    // If we found a shorter path to neighbor
                    if (distanceToNeighbor < distance[neighbor]) {
                        distance[neighbor] = distanceToNeighbor;
                        previous[neighbor] = currentNode;
                        pq.push(std::make_pair(distanceToNeighbor, neighbor));
                    }
                }
                break; // Exit once we find the matching node
            }
        }
    }
    
    return previous;
}


// Method definition for populating the routing table
void SDNController::PopulateRoutingTable(const NodeContainer& nodes, const NodeContainer& switches, const NetDeviceContainer& devices) 
{
    // Step 1: Populate nodes and switches into the graph
    for (uint32_t i = 0; i < switches.GetN(); i++) {
        Ptr<Node> swNode = switches.Get(i);
        for (uint32_t d = 0; d < swNode->GetNDevices(); d++) {
            Ptr<NetDevice> dev = swNode->GetDevice(d);

            graphNode switchGraphNode;
            switchGraphNode.id = swNode->GetId();
            switchGraphNode.device = dev;
            switchGraphNode.macAddr = dev->GetAddress();
            switchGraphNode.ipAddr = Ipv4Address("0.0.0.0");  // No IP

            networkTopologyGraph[switchGraphNode] = std::vector<Link>();
        }
    }

    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        Ptr<Node> n = nodes.Get(i);
        for (uint32_t d = 0; d < n->GetNDevices(); d++) {
            Ptr<NetDevice> dev = n->GetDevice(d);

            graphNode nodeGraphNode;
            nodeGraphNode.id = n->GetId();
            nodeGraphNode.device = dev;
            nodeGraphNode.macAddr = dev->GetAddress();

            Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
            int iface = ipv4->GetInterfaceForDevice(dev);
            nodeGraphNode.ipAddr = ipv4->GetAddress(iface, 0).GetLocal();

            networkTopologyGraph[nodeGraphNode] = std::vector<Link>();
            arpTable[nodeGraphNode.ipAddr] = nodeGraphNode.macAddr;
        }
    }

    // Step 2: Add edges (links) with NetDevice (outgoing port) info
    for (uint32_t i = 0; i < devices.GetN(); i++) {
        Ptr<NetDevice> dev = devices.Get(i);
        Ptr<Node> node = dev->GetNode();
        Ptr<Channel> channel = dev->GetChannel();

        if (channel == nullptr) continue;

        graphNode currentNode;
        currentNode.id = node->GetId();
        currentNode.device = dev;
        currentNode.macAddr = dev->GetAddress();

        // Check if this node is already present (should be)
        if (networkTopologyGraph.find(currentNode) == networkTopologyGraph.end()) {
            continue;  // Skip if not in graph
        }

        for (uint32_t j = 0; j < channel->GetNDevices(); j++) {
            Ptr<NetDevice> otherDev = channel->GetDevice(j);
            if (otherDev == dev) continue;

            Ptr<Node> otherNode = otherDev->GetNode();

            graphNode neighborNode;
            neighborNode.id = otherNode->GetId();
            neighborNode.device = otherDev;
            neighborNode.macAddr = otherDev->GetAddress();

            // Add the outgoing port used to reach the neighbor
            networkTopologyGraph[currentNode].push_back({neighborNode.id, dev, 1});
        }
    }

    // Step 3: Compute shortest paths and fill routing table
    for (const auto& source : networkTopologyGraph) {
        uint32_t sourceId = source.first.id;

        std::map<uint32_t, uint32_t> previous = ComputeShortestPath(networkTopologyGraph, sourceId);

        for (const auto& dest : networkTopologyGraph) {
            uint32_t destId = dest.first.id;

            if (sourceId == destId) continue;

            uint32_t nextHop = destId;
            uint32_t current = destId;

            while (previous[current] != sourceId && previous[current] != 0) {
                nextHop = current;
                current = previous[current];
            }

            if (previous[current] == sourceId) {
                nextHop = current;
            }

            // Find outgoing NetDevice (port) from source to nextHop
            Ptr<NetDevice> outDev = nullptr;
            for (const auto& link : source.second) {
                if (link.neighborId == nextHop) {
                    outDev = link.outgoingPort;
                    break;
                }
            }

            if (outDev) {
                m_routingTable[std::make_pair(sourceId, destId)] = outDev;
            } else {
                NS_LOG_WARN("No outgoing device found from " << sourceId << " to " << nextHop);
            }
        }
    }
}


// Method to look up next hop for a given source and destination
// Method to look up next hop for a given source and destination (with logging)
Ptr<NetDevice> SDNController::GetNextHop(Ptr<NetDevice> currDevice, Ipv4Address dstIp)
{
    int srcNodeId = -1;
    int dstNodeId = -1;

    // Find node IDs corresponding to srcIp and dstIp
    for (const auto& entry : networkTopologyGraph) {
        if (entry.first.device == currDevice) {
            srcNodeId = entry.first.id;
        }
        if (entry.first.ipAddr == dstIp) {
            dstNodeId = entry.first.id;
        }
    }

    if (srcNodeId == -1 || dstNodeId == -1) {
        NS_LOG_WARN("SDNController: Could not find node IDs for given IPs!");
        return nullptr; // Cannot find nodes
    }

    auto key = std::make_pair(srcNodeId, dstNodeId);
    if (m_routingTable.find(key) != m_routingTable.end()) {
        return m_routingTable[key];
    } else {
        NS_LOG_WARN("SDNController: No route from node " << srcNodeId << " to node " << dstNodeId);
        return nullptr; // No route found
    }
}


void SDNController::DisplayDetailedRoutingTable() const {
    std::cout << "\n========== SDN Controller Detailed Routing Information ==========\n";
    
    // If we have the general pairwise routing table
    if (!m_routingTable.empty()) {
        std::cout << "Full Routing Table (Source → Destination → Next Hop):\n";
        std::cout << "--------------------------------------------------------\n";
        
        // Group by source for better readability
        std::map<uint32_t, std::vector<std::pair<uint32_t, Ptr<NetDevice>>>> sourceGroups;
        for (const auto& entry : m_routingTable) {
            sourceGroups[entry.first.first].push_back({entry.first.second, entry.second});
        }
        
        for (const auto& group : sourceGroups) {
            std::cout << "From Node " << group.first << ":\n";
            for (const auto& route : group.second) {
                Ptr<Node> node = route.second->GetNode();
                int nextHop = node->GetId();
                std::cout << "  → To Node " << route.first 
                          << ": Next hop is Node " << nextHop << "\n";
            }
            std::cout << "\n";
        }
    }
    
    std::cout << "=============================================================\n" << std::endl;
}

void SDNController::DisplayARPResolution()
{
    std::cout << "\n========== SDN Controller Detailed ARP Information ==========\n";

    for (const auto& entry : arpTable)
    {
        std::cout << "Device " << entry.second << " is linked to IP " << entry.first << std::endl;
    }
}

void SDNController::AddSwitch(Ptr<SDNSwitch> swtch)
{
    m_switches.push_back(swtch);
    swtch->m_controller = this;
}

Address SDNController::AddressResolution(Ipv4Address ipAddr)
{
    return arpTable[ipAddr];
}

void SDNController::HandlePacketIn (Ptr<SDNSwitch> swtch, controlPacket* ctrl, Ptr<NetDevice> device)
{
    if (ctrl->type == ARPPacket)
    {
        ctrl->dstMac = AddressResolution(ctrl->dstIp);
        NS_LOG_INFO("SDNController: Control packet updated for ARP handling");
    }
    else if (ctrl->type == FlowAdd)
    {
        FlowEntry entry;
        entry.srcIp = ctrl->srcIp;
        entry.dstIp = ctrl->dstIp;
        entry.srcPort = ctrl->srcPort;
        entry.dstPort = ctrl->dstPort;
        entry.protocol = ctrl->protocolNumber;
        entry.outputDevice = GetNextHop(device, ctrl->dstIp);  // Change appropriately
        entry.packetCount = 0;
        entry.byteCount = 0;

        NS_LOG_INFO(entry.outputDevice->GetAddress());
        FlowEntry* newFlow = swtch->InstallFlowEntry(entry);
        NS_LOG_INFO("SDNController: Flow entry installed for src=" << ctrl->srcIp << " dst=" << ctrl->dstIp);
        ctrl->flow = newFlow;
        NS_LOG_INFO("SDNController: Control packet updated for flow");
    }
}


void SDNController::SendPacketOut(Ptr<SDNSwitch> swtch, Ptr<Packet> packet, Ptr<NetDevice> dev)
{
    if (swtch)
    {
        Address addr = dev->GetAddress();
 
        Ipv4Header ipv4Header;
        TcpHeader tcpHeader;
        UdpHeader udpHeader;
        uint16_t protocol;
 
        // Copy the packet to avoid modifying the original
        Ptr<Packet> copy = packet->Copy();
 
        // Extract the IP header
        if (!copy->RemoveHeader(ipv4Header)) return;
        protocol = ipv4Header.GetProtocol();
       
        NS_LOG_INFO("SDNController: Sending packet to switch");
        swtch->ReceivePacket(dev, packet, protocol, addr, addr, ns3::NetDevice::PacketType::PACKET_HOST); // Or ReceiveControlMessage if separate
    }
}

}
}