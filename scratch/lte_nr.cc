
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/quic-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/random-variable-stream.h"
#include <iostream>
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("wns3-mpquic-two-path");


void ThroughputMonitor2 (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, Ptr<OutputStreamWrapper> stream)
{
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classing->FindFlow(stats->first);

        // 檢查是否來自目標節點 4-5 或 6-7 的流量
        //Flow ID=1,3 ==> 4->5的流量

        if (stats->first == 5 || stats->first == 7)
        {
            // *stream->GetStream () 
            // << "FlowId: " << stats->first  
            // << "\tSource IP: " << t.sourceAddress 
            // << "\tSource Port: " << t.sourcePort
            // << "\tDestination IP: " << t.destinationAddress 
            // << "\ttDestination Port: " << t.destinationPort
            // << "\tTime: " << Simulator::Now().GetSeconds()
            // << "\tRxBytes: " << stats->second.rxBytes
            // << "\tRxPackets: " << stats->second.rxPackets 
            // << "\tLastDelay(ms): " << stats->second.lastDelay.GetMilliSeconds()
            // << "\tThroughput(Mbps): " 
            // << stats->second.rxBytes * 8 / 1024 / 1024 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstRxPacket.GetSeconds())
            // << std::endl;
            *stream->GetStream () << stats->first  << "\t" << Simulator::Now().GetSeconds()
            << "\t" << stats->second.rxBytes << "\t" << stats->second.rxPackets << "\t"
            << stats->second.lastDelay.GetMilliSeconds() << "\t" 
            << stats->second.rxBytes*8/1024/1024/(stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstRxPacket.GetSeconds())
            << std::endl;
   
        }
        
    }
    Simulator::Schedule(Seconds(0.05), &ThroughputMonitor2, fmhelper, flowMon, stream);
}

void
ModifyLinkRate(NetDeviceContainer *ptp, DataRate lr, Time delay) {
    StaticCast<PointToPointNetDevice>(ptp->Get(0))->SetDataRate(lr);
    StaticCast<PointToPointChannel>(StaticCast<PointToPointNetDevice>(ptp->Get(0))->GetChannel())->SetAttribute("Delay", TimeValue(delay));
}

void SetPosition(Ptr<Node> node, double x, double y, double z) {
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
    if (!mobility) {
        mobility = CreateObject<ConstantPositionMobilityModel>();
        node->AggregateObject(mobility);
    }
    mobility->SetPosition(Vector(x, y, z));
};

int
main (int argc, char *argv[])
{
    int schedulerType = MpQuicScheduler::ROUND_ROBIN;
    
    string myRandomNo = "5242880";
    string lossrate = "0.0000";

    double rate0a = 5.0;
    double rate1a = 10.0;
    double delay0a = 10.0;
    double delay1a = 50.0;
    double rate0b = 5.0;
    double rate1b = 10.0;
    double delay0b = 50.0;
    double delay1b = 10.0;    

    int bVar = 2;
    int bLambda = 100;
    int mrate = 52428800;
    int ccType = QuicSocketBase::OLIA;
    int mselect = 3;
    int seed = 1;
    TypeId ccTypeId = MpQuicCongestionOps::GetTypeId ();
    CommandLine cmd;


    cmd.AddValue ("SchedulerType", "in use scheduler type (0 - ROUND_ROBIN, 1 - MIN_RTT, 2 - BLEST, 3 - ECF, 4 - Peekaboo", schedulerType);
    cmd.AddValue ("BVar", "e.g. 100", bVar);
    cmd.AddValue ("BLambda", "e.g. 100", bLambda);
    cmd.AddValue ("MabRate", "e.g. 100", mrate);
    cmd.AddValue ("Rate0a", "e.g. 5Mbps", rate0a);
    cmd.AddValue ("Rate1a", "e.g. 50Mbps", rate1a);
    cmd.AddValue ("Delay0a", "e.g. 80ms", delay0a);
    cmd.AddValue ("Delay1a", "e.g. 20ms", delay1a);
    cmd.AddValue ("Rate0b", "e.g. 5Mbps", rate0b);
    cmd.AddValue ("Rate1b", "e.g. 50Mbps", rate1b);
    cmd.AddValue ("Delay0b", "e.g. 80ms", delay0b);
    cmd.AddValue ("Delay1b", "e.g. 20ms", delay1b);
    cmd.AddValue ("Size", "e.g. 80", myRandomNo);
    cmd.AddValue ("Seed", "e.g. 80", seed);
    cmd.AddValue ("LossRate", "e.g. 0.0001", lossrate);
    cmd.AddValue ("Select", "e.g. 0.0001", mselect);
    cmd.AddValue ("CcType", "in use congestion control type (0 - QuicNewReno, 1 - OLIA)", ccType);
    cmd.Parse (argc, argv);

    NS_LOG_INFO("\n\n#################### SIMULATION SET-UP ####################\n\n\n");
    
    LogLevel log_precision = LOG_LEVEL_LOGIC;
    Time::SetResolution (Time::NS);
    LogComponentEnableAll (LOG_PREFIX_TIME);
    LogComponentEnableAll (LOG_PREFIX_FUNC);
    LogComponentEnableAll (LOG_PREFIX_NODE);
    LogComponentEnable ("wns3-mpquic-two-path", log_precision);

    RngSeedManager::SetSeed (seed);  

    if (ccType == QuicSocketBase::OLIA){
        ccTypeId = MpQuicCongestionOps::GetTypeId ();
    }
    if(ccType == QuicSocketBase::QuicNewReno){
        ccTypeId = QuicCongestionOps::GetTypeId ();
    }

    Config::SetDefault ("ns3::QuicSocketBase::SocketSndBufSize",UintegerValue (40000000));
    Config::SetDefault ("ns3::QuicStreamBase::StreamSndBufSize",UintegerValue (40000000));
    Config::SetDefault ("ns3::QuicSocketBase::SocketRcvBufSize",UintegerValue (40000000));
    Config::SetDefault ("ns3::QuicStreamBase::StreamRcvBufSize",UintegerValue (40000000));


    Config::SetDefault ("ns3::QuicSocketBase::EnableMultipath",BooleanValue(true));
    Config::SetDefault ("ns3::QuicSocketBase::CcType",IntegerValue(ccType));
    Config::SetDefault ("ns3::QuicL4Protocol::SocketType",TypeIdValue (ccTypeId));
    Config::SetDefault ("ns3::MpQuicScheduler::SchedulerType", IntegerValue(schedulerType));   
    Config::SetDefault ("ns3::MpQuicScheduler::BlestVar", UintegerValue(bVar));   
    Config::SetDefault ("ns3::MpQuicScheduler::BlestLambda", UintegerValue(bLambda));     
    Config::SetDefault ("ns3::MpQuicScheduler::MabRate", UintegerValue(mrate)); 
    Config::SetDefault ("ns3::MpQuicScheduler::Select", UintegerValue(mselect)); 

    
    Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> (
    "RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
    "ErrorRate", DoubleValue (stod(lossrate)));

    Ptr<UniformRandomVariable> rateVal0 = CreateObject<UniformRandomVariable> ();
    rateVal0->SetAttribute ("Min", DoubleValue (rate0a));
    rateVal0->SetAttribute ("Max", DoubleValue (rate0b));

    Ptr<UniformRandomVariable> rateVal1 = CreateObject<UniformRandomVariable> ();
    rateVal1->SetAttribute ("Min", DoubleValue (rate1a));
    rateVal1->SetAttribute ("Max", DoubleValue (rate1b));

    Ptr<UniformRandomVariable> delayVal0 = CreateObject<UniformRandomVariable> ();
    delayVal0->SetAttribute ("Min", DoubleValue (delay0a));
    delayVal0->SetAttribute ("Max", DoubleValue (delay0b));

    Ptr<UniformRandomVariable> delayVal1 = CreateObject<UniformRandomVariable> ();
    delayVal1->SetAttribute ("Min", DoubleValue (delay1a));
    delayVal1->SetAttribute ("Max", DoubleValue (delay1b));


    int simulationEndTime = 25;
    int start_time = 1;

    uint32_t maxBytes = stoi(myRandomNo);
    
    NS_LOG_INFO ("Create nodes.");
    NodeContainer c;
    c.Create (10);
    NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
    NodeContainer n1n8 = NodeContainer (c.Get (1), c.Get (8));
    NodeContainer n8n2 = NodeContainer (c.Get (8), c.Get (2));
    
    NodeContainer n3n6 = NodeContainer (c.Get (3), c.Get (6));
    NodeContainer n6n9 = NodeContainer (c.Get (6), c.Get (9));
    NodeContainer n9n7 = NodeContainer (c.Get (9), c.Get (7));

    NodeContainer n4n1 = NodeContainer (c.Get (4), c.Get (1));
    NodeContainer n8n5 = NodeContainer (c.Get (8), c.Get (5));

    NodeContainer n4n6 = NodeContainer (c.Get (4), c.Get (6));
    NodeContainer n9n5 = NodeContainer (c.Get (9), c.Get (5));

 
    InternetStackHelper internet;
    internet.Install (c.Get (1));
    internet.Install (c.Get (6));
    internet.Install (c.Get (8));
    internet.Install (c.Get (9));

    QuicHelper stack;
    stack.InstallQuic (c.Get (4));
    stack.InstallQuic (c.Get (5));
    stack.InstallQuic (c.Get (0));
    stack.InstallQuic (c.Get (2));
    stack.InstallQuic (c.Get (7)); 
    stack.InstallQuic (c.Get (3));


    // We create the channels first without any IP addressing information
    NS_LOG_INFO ("Create channels.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue (std::to_string(rateVal0->GetValue())+"Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue (std::to_string(delayVal0->GetValue())+"ms"));
    NetDeviceContainer d1d8 = p2p.Install (n1n8);
    d1d8.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
    cout<<std::to_string(delayVal0->GetValue())+"ms"<<endl;
    cout<<std::to_string(delayVal1->GetValue())+"ms"<<endl;
    p2p.SetDeviceAttribute ("DataRate", StringValue (std::to_string(rateVal1->GetValue())+"Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue (std::to_string(delayVal1->GetValue())+"ms"));
    NetDeviceContainer d6d9 = p2p.Install (n6n9);
    d6d9.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

    p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
    NetDeviceContainer d4d1 = p2p.Install (n4n1);
    NetDeviceContainer d0d1 = p2p.Install (n0n1);
    NetDeviceContainer d8d5 = p2p.Install (n8n5);
    NetDeviceContainer d4d6 = p2p.Install (n4n6);
    NetDeviceContainer d9d5 = p2p.Install (n9n5);
    NetDeviceContainer d8d2 = p2p.Install (n8n2);
    NetDeviceContainer d3d6 = p2p.Install (n3n6);
    NetDeviceContainer d9d7 = p2p.Install (n9n7);
    
    // Later, we add IP addresses.
    NS_LOG_INFO ("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer i4i1 = ipv4.Assign (d4d1);

    ipv4.SetBase ("10.1.9.0", "255.255.255.0");
    Ipv4InterfaceContainer i1i8 = ipv4.Assign (d1d8);

    ipv4.SetBase ("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer i8i5 = ipv4.Assign (d8d5);
    
    ipv4.SetBase ("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer i4i6 = ipv4.Assign (d4d6);

    ipv4.SetBase ("10.1.10.0", "255.255.255.0");
    Ipv4InterfaceContainer i6i9 = ipv4.Assign (d6d9);

    ipv4.SetBase ("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer i9i5 = ipv4.Assign (d9d5);
    
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);

    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i8i2 = ipv4.Assign (d8d2);
    
    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i3i6 = ipv4.Assign (d3d6);

    ipv4.SetBase ("10.1.8.0", "255.255.255.0");
    Ipv4InterfaceContainer i9i7 = ipv4.Assign (d9d7);


    Ptr<Ipv4> ipv4_n4 = c.Get(4)->GetObject<Ipv4> ();
    Ipv4StaticRoutingHelper ipv4RoutingHelper; 
    Ptr<Ipv4StaticRouting> staticRouting_n4 = ipv4RoutingHelper.GetStaticRouting (ipv4_n4); 
    staticRouting_n4->AddHostRouteTo (Ipv4Address ("10.1.5.2"), Ipv4Address ("10.1.9.2") ,1); 
    staticRouting_n4->AddHostRouteTo (Ipv4Address ("10.1.7.2"), Ipv4Address ("10.1.10.2") ,2); 
    Ptr<Ipv4> ipv4_n5 = c.Get(5)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> staticRouting_n5 = ipv4RoutingHelper.GetStaticRouting (ipv4_n5); 
    staticRouting_n5->AddHostRouteTo (Ipv4Address ("10.1.4.1"), Ipv4Address ("10.1.9.1") ,1); 
    staticRouting_n5->AddHostRouteTo (Ipv4Address ("10.1.6.1"), Ipv4Address ("10.1.10.1") ,2);

    Ptr<Ipv4> ipv4_n0 = c.Get(0)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> staticRouting_n0 = ipv4RoutingHelper.GetStaticRouting (ipv4_n0); 
    staticRouting_n0->AddHostRouteTo (Ipv4Address ("10.1.2.2"), Ipv4Address ("10.1.9.2") ,1); 
    Ptr<Ipv4> ipv4_n2 = c.Get(2)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> staticRouting_n2 = ipv4RoutingHelper.GetStaticRouting (ipv4_n2); 
    staticRouting_n2->AddHostRouteTo (Ipv4Address ("10.1.1.1"), Ipv4Address ("10.1.9.1") ,1); 

    Ptr<Ipv4> ipv4_n3 = c.Get(3)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> staticRouting_n3 = ipv4RoutingHelper.GetStaticRouting (ipv4_n3); 
    staticRouting_n3->AddHostRouteTo (Ipv4Address ("10.1.8.2"), Ipv4Address ("10.1.10.2") ,1); 
    Ptr<Ipv4> ipv4_n7 = c.Get(7)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> staticRouting_n7 = ipv4RoutingHelper.GetStaticRouting (ipv4_n7); 
    staticRouting_n7->AddHostRouteTo (Ipv4Address ("10.1.3.1"), Ipv4Address ("10.1.10.1") ,1); 



    // Create router nodes, initialize routing database and set up the routing
    // tables in the nodes.
    NS_LOG_INFO ("Create All node's routing table");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


    // ---------------設定 nr 節點與裝置---------------

    NS_LOG_INFO ("start setting NR");
    NodeContainer enbNodes;
    enbNodes.Create(1); // eNodeB 節點

    NodeContainer ueNodes;
    ueNodes.Add (c.Get (6)); // 節點 6 作為 UE
    ueNodes.Add (c.Get (9)); // 節點 9 作為 UE

    uint16_t gNbNum = 1;
    uint16_t ueNumPergNb = 2;


    // Simulation parameters. Please don't use double to indicate seconds; use
    // ns-3 Time values which use integers to avoid portability issues.
    Time simTime = MilliSeconds(1000);
    Time udpAppStartTime = MilliSeconds(400);

    // NR parameters (Reference: 3GPP TR 38.901 V17.0.0 (Release 17)
    // Table 7.8-1 for the power and BW).
    // In this example the BW has been split into two BWPs
    // We will take the input from the command line, and then we
    // will pass them inside the NR module.
    uint16_t numerologyBwp1 = 4;
    double centralFrequencyBand1 = 28e9;
    double bandwidthBand1 = 50e6;
    uint16_t numerologyBwp2 = 2;
    
    
    double totalTxPower = 35;

    NS_ABORT_IF(centralFrequencyBand1 < 0.5e9 && centralFrequencyBand1 > 100e9);

    //-----------------------------

    cout<<"now here1";

    // 配置 eNB 和 UE 的靜態位置
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(enbNodes);  // eNB 由 GridScenario 控制
    mobility.Install(ueNodes);   // UE 需要手動配置

    // 設定 UE 節點位置（避免和 eNB 重疊）
    SetPosition(ueNodes.Get(0), 100.0, 100.0, 0.0);
    SetPosition(ueNodes.Get(1), 200.0, 150.0, 0.0);




    //-----------------------------
    
    Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

    int64_t randomStream = 1;
 
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();

    // Put the pointers inside nrHelper
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(epcHelper);

    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1; // in this example, both bands have a single CC

    // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
    // a single BWP per CC
    CcBwpCreator::SimpleOperationBandConf bandConf1(centralFrequencyBand1,
                                                    bandwidthBand1,
                                                    numCcPerBand,
                                                    BandwidthPartInfo::UMi_StreetCanyon);


    // By using the configuration created, it is time to make the operation bands
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf1);


    /*
     * The configured spectrum division is:
     * ------------Band1--------------|--------------Band2-----------------
     * ------------CC1----------------|--------------CC2-------------------
     * ------------BWP1---------------|--------------BWP2------------------
     */

    /*
     * Attributes of ThreeGppChannelModel still cannot be set in our way.
     * TODO: Coordinate with Tommaso
     */
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

    /*
     * Initialize channel and pathloss, plus other things inside band1. If needed,
     * the band configuration can be done manually, but we leave it for more
     * sophisticated examples. For the moment, this method will take care
     * of all the spectrum initialization needs.
     */
    nrHelper->InitializeOperationBand(&band1);

    /*
     * Start to account for the bandwidth used by the example, as well as
     * the total power that has to be divided among the BWPs.
     */
    double x = pow(10, totalTxPower / 10);
    double totalBandwidth = bandwidthBand1;

    /*
     * if not single band simulation, initialize and setup power in the second band
     */

    allBwps = CcBwpCreator::GetAllBwps({band1});

    // Packet::EnableChecking();
    // Packet::EnablePrinting();

    /*
     *  Case (i): Attributes valid for all the nodes
     */
    // Beamforming method
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));

    // Core latency
    epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));

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

    uint32_t bwpIdForLowLat = 0;
    uint32_t bwpIdForVoice = 0;


    // gNb routing between Bearer and bandwidh part
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB",
                                                 UintegerValue(bwpIdForLowLat));
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("GBR_CONV_VOICE", UintegerValue(bwpIdForVoice));

    // Ue routing between Bearer and bandwidth part
    nrHelper->SetUeBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB", UintegerValue(bwpIdForLowLat));
    nrHelper->SetUeBwpManagerAlgorithmAttribute("GBR_CONV_VOICE", UintegerValue(bwpIdForVoice));

    /*
     * We miss many other parameters. By default, not configuring them is equivalent
     * to use the default values. Please, have a look at the documentation to see
     * what are the default values for all the attributes you are not seeing here.
     */

    /*
     * Case (ii): Attributes valid for a subset of the nodes
     */

    // NOT PRESENT IN THIS SIMPLE EXAMPLE

    /*
     * We have configured the attributes we needed. Now, install and get the pointers
     * to the NetDevices, which contains all the NR stack:
     */

    NetDeviceContainer enbNetDev =
        nrHelper->InstallGnbDevice(enbNodes, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueNodes, allBwps);


    randomStream += nrHelper->AssignStreams(enbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);

    /*
     * Case (iii): Go node for node and change the attributes we have to setup
     * per-node.
     */

    // Get the first netdevice (enbNetDev.Get (0)) and the first bandwidth part (0)
    // and set the attribute.
    nrHelper->GetGnbPhy(enbNetDev.Get(0), 0)
        ->SetAttribute("Numerology", UintegerValue(numerologyBwp1));
    nrHelper->GetGnbPhy(enbNetDev.Get(0), 0)
        ->SetAttribute("TxPower", DoubleValue(10 * log10((bandwidthBand1 / totalBandwidth) * x)));

    // When all the configuration is done, explicitly call UpdateConfig ()

    for (auto it = enbNetDev.Begin(); it != enbNetDev.End(); ++it)
    {
        DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig();
    }

    for (auto it = ueNetDev.Begin(); it != ueNetDev.End(); ++it)
    {
        DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
    }



    // attach UEs to the closest eNB
    nrHelper->AttachToClosestEnb(ueNetDev, enbNetDev);

    //=============== NR SETTING DONE ======================


    // ---------------設定 LTE 節點與裝置---------------
    NS_LOG_INFO ("start setting LTE");
    NodeContainer enbNodes2;
    enbNodes2.Create(1); // eNodeB 節點

    NodeContainer ueNodes2;
    ueNodes2.Add (c.Get (1)); // 節點 1 作為 UE
    ueNodes2.Add (c.Get (8)); // 節點 8 作為 UE

    // 配置移動模型
    MobilityHelper mobility2;
    mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility2.Install (enbNodes2);
    mobility2.Install (ueNodes2);

    // lteHelper setting 
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    NetDeviceContainer enbDevs2 = lteHelper->InstallEnbDevice (enbNodes2);
    NetDeviceContainer ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);
    NS_ASSERT_MSG (enbDevs2.GetN() > 0, "eNodeB devices not installed correctly!");
    NS_ASSERT_MSG (ueDevs2.GetN() > 0, "UE devices not installed correctly!");

    // Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    // lteHelper->SetEpcHelper(epcHelper);

    NS_LOG_INFO("pair to enode and UE");
    lteHelper->Attach (ueDevs2.Get (0), enbDevs2.Get (0)); // 節點 6 連接到 eNodeB
    lteHelper->Attach (ueDevs2.Get (1), enbDevs2.Get (0)); // 節點 9 連接到 eNodeB
    
    //=============== LTE SETTING DONE ======================

    // UE's IP
    std::cout << "UE 1 IP Address: " << i1i8.GetAddress(0) << std::endl;
    std::cout << "UE 8 IP Address: " << i1i8.GetAddress(1) << std::endl;
     // 設置應用程序 (n4 -> n5)
    uint16_t port3 = 11; // 通訊埠
    MpquicBulkSendHelper sender3("ns3::QuicSocketFactory", InetSocketAddress(i8i5.GetAddress (1), port3));
    sender3.SetAttribute("MaxBytes", UintegerValue(maxBytes)); // 10 MB
    ApplicationContainer appSender3 = sender3.Install(c.Get(4));
    appSender3.Start(Seconds(start_time));
    appSender3.Stop(Seconds(simulationEndTime));

    PacketSinkHelper receiver3("ns3::QuicSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port3));
    ApplicationContainer appReceiver3 = receiver3.Install(c.Get(5));
    appReceiver3.Start(Seconds(0.0));
    appReceiver3.Stop(Seconds(simulationEndTime));
        
    // 設置應用程序 (n0 -> n2)
    uint16_t port1 = 9; // 通訊埠
    MpquicBulkSendHelper sender1("ns3::QuicSocketFactory", InetSocketAddress(i8i2.GetAddress (1), port1));
    sender1.SetAttribute("MaxBytes", UintegerValue(maxBytes/2)); // 5 MB
    ApplicationContainer appSender1 = sender1.Install(c.Get(0));
    appSender1.Start(Seconds(2.0));
    appSender1.Stop(Seconds(simulationEndTime));
    

    PacketSinkHelper receiver1("ns3::QuicSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port1));
    ApplicationContainer appReceiver1 = receiver1.Install(c.Get(2));
    appReceiver1.Start(Seconds(1.0));
    appReceiver1.Stop(Seconds(simulationEndTime));

    NS_LOG_INFO("Sender1 is sending to " << i8i2.GetAddress (1));

    // 設置應用程序 (n3 -> n7)
    uint16_t port2 = 10; // 通訊埠
    MpquicBulkSendHelper sender2("ns3::QuicSocketFactory", InetSocketAddress(i9i7.GetAddress (1), port2));
    sender2.SetAttribute("MaxBytes", UintegerValue(maxBytes/2)); // 10 MB
    ApplicationContainer appSender2 = sender2.Install(c.Get(3));
    appSender2.Start(Seconds(2));
    appSender2.Stop(Seconds(simulationEndTime));

    PacketSinkHelper receiver2("ns3::QuicSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port2));
    ApplicationContainer appReceiver2 = receiver2.Install(c.Get(7));
    appReceiver2.Start(Seconds(1.0));
    appReceiver2.Stop(Seconds(simulationEndTime));

   
Ptr<Node> sender1Node = c.Get(0);  // sender1 的節點
for (uint32_t i = 0; i < sender1Node->GetNDevices(); i++) {
    Ptr<NetDevice> device = sender1Node->GetDevice(i);
    NS_LOG_INFO("Sender1 has NetDevice ID=" << device->GetIfIndex());
}

    std::ostringstream file;
    file<<"./scheduler" << schedulerType;

    AsciiTraceHelper asciiTraceHelper;
    std::ostringstream fileName;
    fileName <<  "./scheduler" << schedulerType << "-rx-ltenr" << ".txt";
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName.str ());
  

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
    // ThroughputMonitor(&flowmon, monitor, stream); 
    ThroughputMonitor2(&flowmon, monitor, stream);
    

    for (double i = 1; i < simulationEndTime; i = i+0.1){
        Simulator::Schedule (Seconds (i), &ModifyLinkRate, &d1d8, DataRate(std::to_string(rateVal0->GetValue())+"Mbps"),  Time::FromInteger(delayVal0->GetValue(), Time::MS));
        Simulator::Schedule (Seconds (i), &ModifyLinkRate, &d6d9, DataRate(std::to_string(rateVal1->GetValue())+"Mbps"),Time::FromInteger(delayVal1->GetValue(), Time::MS));
    }


    Simulator::Stop (Seconds(simulationEndTime));
    NS_LOG_INFO("\n\n#################### STARTING RUN ####################\n\n");
    Simulator::Run ();
    // Ptr<LteEnbNetDevice> enb = enbDevs2.Get(0)->GetObject<LteEnbNetDevice>();

    // Ptr<Ipv4> ipv4_n6 = c.Get(6)->GetObject<Ipv4> ();

    // std::cout << "Node 6 IP Addresses: " << std::endl;
    // for (uint32_t i = 0; i < ipv4_n6->GetNInterfaces(); i++) {
    //     for (uint32_t j = 0; j < ipv4_n6->GetNAddresses(i); j++) {
    //         std::cout << "  Interface " << i << ": " 
    //                 << ipv4_n6->GetAddress(i, j).GetLocal() << std::endl;
    //     }
    // }
        

    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        // if (i->first == 5 || i->first == 7)
        {

        NS_LOG_INFO("Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")"
        << "\n Last rx Seconds: " << i->second.timeLastRxPacket.GetSeconds()
        << "\n Rx Bytes: " << i->second.rxBytes
        << "\n DelaySum(s): " << i->second.delaySum.GetSeconds()
        << "\n rxPackets: " << i->second.rxPackets);
        }
        
    }

    NS_LOG_INFO("\nfile size: "<<maxBytes<< " Bytes, scheduler type: " <<schedulerType<<
                "\npath 0: rate "<< rate0a <<", delay "<< delay0a << 
                "\npath 1: rate " << rate1a << ", delay " << delay1a );

    Simulator::Destroy ();

    return 0;
}

