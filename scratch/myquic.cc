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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RANWithQUICIntegration");



int main(int argc, char *argv[])
{
    uint16_t gNbNum = 1;
    uint16_t ueNumPergNb = 1;
    double centralFrequencyBand1 = 28e9;
    double bandwidthBand1 = 400e6;

      //LogComponentEnable("EpcPgwApplication", LOG_LEVEL_ALL);
      //LogComponentEnable("NrHelper", LOG_LEVEL_ALL);
      LogComponentEnable ("QuicEchoClientApplication", LOG_LEVEL_ALL);

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
    // InternetStackHelper internet;
    // internet.Install(gridScenario.GetUserTerminals());
    // internet.Install(gridScenario.GetBaseStations());
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
    serverApps.Stop(Seconds(120.0));

    // Install QUIC Client
    QuicEchoClientHelper echoClient(ueIpIface.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    cout<<"start trans"<<endl;
    ApplicationContainer clientApps = echoClient.Install(gridScenario.GetUserTerminals().Get(0));
    echoClient.SetFill(clientApps.Get(0), "Hello World");
    cout<<"start transfer"<<endl;
    cout<<"server : "<<&echoServer<<" , client: "<<&echoClient<<endl;
    clientApps.Start(Seconds(10.0));
    clientApps.Stop(Seconds(120.0));

    // Enable Traces
    nrHelper->EnableTraces();

    // Run Simulation
    cout<<"stop transfer"<<endl;
    Simulator::Stop(Seconds(120.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
