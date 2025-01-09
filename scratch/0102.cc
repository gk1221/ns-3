#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MpquicLteIntegration");

int main(int argc, char *argv[]) {
    // Simulation parameters
    double rateP2P = 10.0; // P2P rate in Mbps
    double delayP2P = 50.0; // P2P delay in ms
    uint16_t numUePairs = 1; // Number of LTE UE-eNodeB pairs
    Time simTime = Seconds(30);

    // Command line arguments
    CommandLine cmd;
    cmd.AddValue("rateP2P", "Rate for P2P link in Mbps", rateP2P);
    cmd.AddValue("delayP2P", "Delay for P2P link in ms", delayP2P);
    cmd.Parse(argc, argv);

    // Create nodes
    NodeContainer p2pNodes;
    p2pNodes.Create(2);
    NodeContainer lteEnbNodes;
    lteEnbNodes.Create(numUePairs);
    NodeContainer lteUeNodes;
    lteUeNodes.Create(numUePairs);

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install(p2pNodes);
    internet.Install(lteUeNodes);

    // Point-to-Point link setup
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(rateP2P) + "Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue(std::to_string(delayP2P) + "ms"));
    NetDeviceContainer p2pDevices = p2p.Install(p2pNodes);

    // LTE setup
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);

    Ptr<Node> pgw = epcHelper->GetPgwNode();
    internet.Install(pgw);

    // Configure mobility for eNodeBs and UEs
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(lteEnbNodes);
    mobility.Install(lteUeNodes);

    // Install LTE devices
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(lteEnbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(lteUeNodes);

    // Assign IP addresses to UEs
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
    for (uint32_t i = 0; i < lteUeNodes.GetN(); ++i) {
        Ptr<Node> ueNode = lteUeNodes.Get(i);
        Ptr<Ipv4StaticRouting> ueStaticRouting = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(ueNode->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }

    // Attach UEs to eNodeBs
    for (uint16_t i = 0; i < numUePairs; ++i) {
        lteHelper->Attach(ueLteDevs.Get(i), enbLteDevs.Get(i));
    }

    // Assign IP addresses for P2P nodes
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = ipv4.Assign(p2pDevices);

    // Applications: MPQUIC sender and receiver setup
    uint16_t quicPort = 9;

    // P2P Sender
    MpquicBulkSendHelper p2pSource("ns3::QuicSocketFactory", InetSocketAddress(p2pInterfaces.GetAddress(1), quicPort));
    p2pSource.SetAttribute("MaxBytes", UintegerValue(5242880));
    ApplicationContainer p2pSourceApps = p2pSource.Install(p2pNodes.Get(0));
    p2pSourceApps.Start(Seconds(1.0));
    p2pSourceApps.Stop(simTime);

    // LTE Receiver
    PacketSinkHelper lteSink("ns3::QuicSocketFactory", InetSocketAddress(ueIpIface.GetAddress(0), quicPort));
    ApplicationContainer lteSinkApps = lteSink.Install(lteUeNodes.Get(0));
    lteSinkApps.Start(Seconds(1.0));
    lteSinkApps.Stop(simTime);

    // Start simulation
    Simulator::Stop(simTime);
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
