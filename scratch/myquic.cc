/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/nr-module.h"
#include "ns3/quic-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/grid-scenario-helper.h"
#include "ns3/antenna-module.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/eps-bearer-tag.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RANWithQUICIntegration");

static bool g_rxPdcpCallbackCalled = false;
static bool g_rxRxRlcPDUCallbackCalled = false;

void
RxPdcpPDU(std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t pdcpDelay)
{
    std::cout << "\n Packet PDCP delay:" << pdcpDelay << "\n";
    g_rxPdcpCallbackCalled = true;
}

void
RxRlcPDU(std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t rlcDelay)
{
    std::cout << "\n\n Data received at RLC layer at:" << Simulator::Now() << std::endl;
    std::cout << "\n rnti:" << rnti << std::endl;
    std::cout << "\n lcid:" << (unsigned)lcid << std::endl;
    std::cout << "\n bytes :" << bytes << std::endl;
    std::cout << "\n delay :" << (rlcDelay / 1e6) << "ms"<<std::endl;
    g_rxRxRlcPDUCallbackCalled = true;
}

void
ConnectPdcpRlcTraces()
{
    Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/DataRadioBearerMap/1/LtePdcp/RxPDU",
                    MakeCallback(&RxPdcpPDU));

    Config::Connect("/NodeList/*/DeviceList/*/LteUeRrc/DataRadioBearerMap/1/LteRlc/RxPDU",
                    MakeCallback(&RxRlcPDU));
    
}

int Etag1 = 1;
int Etag2 = 1;
static void
SendPacket(Ptr<NetDevice> device, Address& addr, uint32_t packetSize)
{
    Ptr<Packet> pkt = Create<Packet>(packetSize);
    Ipv4Header ipv4Header;
    ipv4Header.SetProtocol(UdpL4Protocol::PROT_NUMBER);
    pkt->AddHeader(ipv4Header);
    //EpsBearerTag tag(Etag1++, Etag2++);
    EpsBearerTag tag(1, 1);
    pkt->AddPacketTag(tag);
    std::cout << "Packet tags before sending: ";
    pkt->PrintPacketTags(std::cout);
    std::cout << std::endl;
    device->Send(pkt, addr, Ipv4L3Protocol::PROT_NUMBER);
}



int main(int argc, char *argv[])
{
    uint16_t gNbNum = 1;
    uint16_t ueNumPergNb = 1;
    double centralFrequencyBand1 = 28e9;
    double bandwidthBand1 = 400e6;



      //LogComponentEnable("EpcPgwApplication", LOG_LEVEL_ALL);
      //LogComponentEnable("NrHelper", LOG_LEVEL_ALL);
    LogLevel log_precision = LOG_LEVEL_LOGIC;
    LogComponentEnableAll (LOG_PREFIX_TIME);
    LogComponentEnableAll (LOG_PREFIX_FUNC);
    LogComponentEnableAll (LOG_PREFIX_NODE);
    LogComponentEnable ("QuicEchoClientApplication", log_precision);
    LogComponentEnable ("QuicSocketBase", log_precision);
      //LogComponentEnable ("QuicEchoClientApplication", LOG_LEVEL_ALL);

    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create Grid Scenario
    GridScenarioHelper gridScenario;
    gridScenario.SetRows(1);
    gridScenario.SetColumns(gNbNum);
    gridScenario.SetHorizontalBsDistance(100.0);
    gridScenario.SetBsHeight(10.0);
    gridScenario.SetUtHeight(1.5);
    gridScenario.SetSectorization(GridScenarioHelper::SINGLE);
    gridScenario.SetBsNumber(gNbNum);
    gridScenario.SetUtNumber(ueNumPergNb * gNbNum);
    gridScenario.CreateScenario();

    // NR Helper Setup
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();

    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
     // EPC Setup should set before idealBeamformingHelper init
    cout<<"epchelper create"<<endl;
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    nrHelper->SetEpcHelper(epcHelper);
    cout<<"epchelper created"<<endl;

    // Define NR Bandwidth Part
    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1;
    CcBwpCreator::SimpleOperationBandConf bandConf1(centralFrequencyBand1, bandwidthBand1, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon_LoS);
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf1);

    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetSchedulerAttribute("FixedMcsDl", BooleanValue(true));
    nrHelper->SetSchedulerAttribute("StartingMcsDl", UintegerValue(28));
    nrHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

    nrHelper->InitializeOperationBand(&band1);
    allBwps = CcBwpCreator::GetAllBwps({band1});

    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));

    // Antennas for all the UEs
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
    nrHelper->SetUeAntennaAttribute("AntennaElement",
                                    PointerValue(CreateObject<IsotropicAntennaModel>()));

    // Antennas for all the gNbs
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(4));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                     PointerValue(CreateObject<IsotropicAntennaModel>()));                                     

    // Install NR Devices
    NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice(gridScenario.GetBaseStations(), allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(gridScenario.GetUserTerminals(), allBwps);

     // Set the attribute of the netdevice (enbNetDev.Get (0)) and bandwidth part (0)
    nrHelper->GetGnbPhy(enbNetDev.Get(0), 0)
        ->SetAttribute("Numerology", UintegerValue(0));

    for (auto it = enbNetDev.Begin(); it != enbNetDev.End(); ++it)
    {
        DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig();
    }

    for (auto it = ueNetDev.Begin(); it != ueNetDev.End(); ++it)
    {
        DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
    }

   

    // Internet Stack
    cout<<"InternetStackHelper create"<<endl;
    QuicHelper stack;
    stack.InstallQuic (gridScenario.GetUserTerminals());
    stack.InstallQuic (gridScenario.GetBaseStations());

    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // Attach UEs to gNBs
    cout<<"AttachToClosestEnb create"<<endl;
    nrHelper->AttachToClosestEnb(ueNetDev, enbNetDev);

    // Install QUIC Server
    cout<<"Install QUIC Server"<<endl;
    QuicEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(gridScenario.GetBaseStations().Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(15.0));

    // Install QUIC Client
    QuicEchoClientHelper echoClient(ueIpIface.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // Simulator::Schedule(Seconds(4.4),
    //                         &SendPacket,
    //                         enbNetDev.Get(0),
    //                         ueNetDev.Get(0)->GetAddress(),
    //                         1000);
 
    ApplicationContainer clientApps = echoClient.Install(gridScenario.GetUserTerminals().Get(0));
    echoClient.SetFill(clientApps.Get(0), "Hello World");
    cout<<"##################start transfer#########"<<endl;
    cout<<"server : "<<&echoServer<<" , client: "<<&echoClient<<endl;
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(20.0));

    Simulator::Schedule(Seconds(0.2), &ConnectPdcpRlcTraces);

    // Enable Traces
    nrHelper->EnableTraces();

    // Run Simulation
    
    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();
    cout<<"#############stop transfer##################"<<endl;

    return 0;
}
