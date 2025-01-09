#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/quic-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("epc-quic");

int main(int argc, char* argv[])
{
    LogLevel log_precision = LOG_LEVEL_LOGIC;
    Time::SetResolution (Time::NS);
    LogComponentEnableAll (LOG_PREFIX_TIME);
    LogComponentEnableAll (LOG_PREFIX_FUNC);
    LogComponentEnableAll (LOG_PREFIX_NODE);
    LogComponentEnable ("epc-quic", log_precision);
    LogComponentEnable("SocketFactory", log_precision);

    uint16_t numNodePairs = 1;
    Time simTime = MilliSeconds(1100);
    double distance = 60.0;
    Time interPacketInterval = MilliSeconds(100);

    // Command line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("numNodePairs", "Number of eNodeBs + UE pairs", numNodePairs);
    cmd.AddValue("simTime", "Total duration of the simulation", simTime);
    cmd.AddValue("distance", "Distance between eNBs [m]", distance);
    cmd.AddValue("interPacketInterval", "Inter packet interval", interPacketInterval);
    cmd.Parse(argc, argv);
    

    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults();
    cmd.Parse(argc, argv);

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    // Create a single RemoteHost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    // Create the Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2ph.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create(numNodePairs);
    ueNodes.Create(numNodePairs);

    // Install Mobility Model
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (uint16_t i = 0; i < numNodePairs; i++)
    {
        positionAlloc->Add(Vector(distance * i, 0, 0));
    }
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(enbNodes);
    mobility.Install(ueNodes);

    // Install LTE Devices to the nodes
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

    // Install the IP stack on the UEs
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
        Ptr<Node> ueNode = ueNodes.Get(u);
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }

    // Attach one UE per eNodeB
    for (uint16_t i = 0; i < numNodePairs; i++)
    {
        lteHelper->Attach(ueLteDevs.Get(i), enbLteDevs.Get(i));
    }

    // Install and start QUIC applications
    uint16_t dlPort = 1100;
    ApplicationContainer clientApps;
    ApplicationContainer serverApps;
    cout<<"adding quichelper"<<endl;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
        // Set up QUIC server on the remote host
        cout<<"adding quicserverhelper"<<endl;
        QuicServerHelper quicServerHelper(dlPort);
        serverApps.Add(quicServerHelper.Install(remoteHost));
        cout<<"adding quicclienthelper"<<endl;
        // Set up QUIC client on UEs
        QuicClientHelper quicClientHelper(ueIpIface.GetAddress(u), dlPort);
        quicClientHelper.SetAttribute("Interval", TimeValue(interPacketInterval));
        quicClientHelper.SetAttribute("MaxPackets", UintegerValue(1000000));
        quicClientHelper.SetAttribute ("PacketSize", UintegerValue(1000));
        clientApps.Add(quicClientHelper.Install(ueNodes.Get(u)));
    }
    

    serverApps.Start(MilliSeconds(500));
    clientApps.Start(MilliSeconds(500));

    lteHelper->EnableTraces();

    Simulator::Stop(simTime);
    cout<<"server start "<<endl;
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
