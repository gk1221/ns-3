/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*

 * n0                     n2
 *   \        TCP        /
 *    n1 -------------- n8
 *   /         P0        \
 * n4                     n5
 *   \         P1        /
 *    n6 -------------- n9
 *   /        TCP        \
 * n3                     n7

 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/quic-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-epc-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("mpquic-4G");

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, Ptr<OutputStreamWrapper> stream)
{
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
        //只紀錄flow=1(1->8), 3(6->9)
        //ID Time ReceiveBytes ReceivePackets Delay Throughput
        if (stats->first == 1 || stats->first == 3){
            *stream->GetStream () << stats->first  << "\t" << Simulator::Now().GetSeconds()/*->second.timeLastRxPacket.GetSeconds()*/ << "\t" << stats->second.rxBytes << "\t" << stats->second.rxPackets << "\t" << stats->second.lastDelay.GetMilliSeconds() << "\t" << stats->second.rxBytes*8/1024/1024/(stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstRxPacket.GetSeconds())  << std::endl;
        }
    }
    Simulator::Schedule(Seconds(0.05),&ThroughputMonitor, fmhelper, flowMon, stream);
}
//手動方式讓線路頻寬變動
void
ModifyLinkRate(NetDeviceContainer *ptp, DataRate lr, Time delay) {
    if (ptp == nullptr || ptp->GetN() == 0 || ptp->Get(0) == nullptr) {
        NS_LOG_ERROR("Null pointer encountered in ModifyLinkRate.");
        return;
    }NS_LOG_UNCOND("Setting DataRate to: " << lr);
    StaticCast<PointToPointNetDevice>(ptp->Get(0))->SetDataRate(lr);
    NS_LOG_UNCOND("Setting Delay to: " << delay);
    StaticCast<PointToPointChannel>(StaticCast<PointToPointNetDevice>(ptp->Get(0))->GetChannel())->SetAttribute("Delay", TimeValue(delay));
    }

int
main (int argc, char *argv[])
{
    int schedulerType = MpQuicScheduler::ROUND_ROBIN;
    
    string myRandomNo = "5242880";
    string lossrate = "0.0000";

    double rate0a = 5.0;
    double rate1a = 10.0;
    double delay0a = 50.0;
    double delay1a = 10.0;
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
    //輸出function call
    LogLevel log_precision = LOG_LEVEL_LOGIC;
    Time::SetResolution (Time::NS);
    LogComponentEnableAll (LOG_PREFIX_TIME);
    LogComponentEnableAll (LOG_PREFIX_FUNC);
    LogComponentEnableAll (LOG_PREFIX_NODE);
    LogComponentEnable ("mpquic-4G", log_precision);
    // LogComponentEnable ("QuicHelper", log_precision);
    // LogComponentEnable("QuicSocketBase", ns3::LOG_LEVEL_DEBUG);
    // LogComponentEnable("InetSocketAddress", ns3::LOG_LEVEL_DEBUG);

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


    int simulationEndTime = 30;
    int start_time = 1;

    uint32_t maxBytes = stoi(myRandomNo);
    
    NS_LOG_INFO ("Create nodes.");
    NodeContainer c;
    c.Create (10);
    NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
    NodeContainer n1n8 = NodeContainer (c.Get (1), c.Get (8));//path1
    NodeContainer n8n2 = NodeContainer (c.Get (8), c.Get (2));
    
    NodeContainer n3n6 = NodeContainer (c.Get (3), c.Get (6));
    NodeContainer n6n9 = NodeContainer (c.Get (6), c.Get (9));//path2
    NodeContainer n9n7 = NodeContainer (c.Get (9), c.Get (7));

    NodeContainer n4n1 = NodeContainer (c.Get (4), c.Get (1));
    NodeContainer n8n5 = NodeContainer (c.Get (8), c.Get (5));

    NodeContainer n4n6 = NodeContainer (c.Get (4), c.Get (6));
    NodeContainer n9n5 = NodeContainer (c.Get (9), c.Get (5));

    //其他節點都安裝Internetstack 4作為傳輸5作為接收所以接上quichelper
    InternetStackHelper internet;
    internet.Install (c.Get (0));
    internet.Install (c.Get (1));
    internet.Install (c.Get (2));
    internet.Install (c.Get (3));
    internet.Install (c.Get (6));
    internet.Install (c.Get (7));
    internet.Install (c.Get (8));
    internet.Install (c.Get (9));

    QuicHelper stack;
    stack.InstallQuic (c.Get (4));
    stack.InstallQuic (c.Get (5));


    //對1-8 之間定義傳輸channel的參數  5Mbps, 50ms
    NS_LOG_INFO ("Create channels.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue (std::to_string(rateVal0->GetValue())+"Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue (std::to_string(delayVal0->GetValue())+"ms"));
    NetDeviceContainer d1d8 = p2p.Install (n1n8);
    d1d8.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
    //對6-9之間定義傳輸channel的參數  10Mbps, 10ms
    p2p.SetDeviceAttribute ("DataRate", StringValue (std::to_string(rateVal1->GetValue())+"Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue (std::to_string(delayVal1->GetValue())+"ms"));
    NetDeviceContainer d6d9 = p2p.Install (n6n9);
    d6d9.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

    if (d1d8.GetN() == 0 || d1d8.Get(0) == nullptr) {
        NS_LOG_ERROR("d1d8 not properly initialized.");
    }
    if (d6d9.GetN() == 0 || d6d9.Get(0) == nullptr) {
        NS_LOG_ERROR("d6d9 not properly initialized.");
    }
    //其他線路之間的channel都定義為100Mbps, 0ms
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
    Ipv4InterfaceContainer i1i2 = ipv4.Assign (d8d2);
    
    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i3i6 = ipv4.Assign (d3d6);

    ipv4.SetBase ("10.1.8.0", "255.255.255.0");
    Ipv4InterfaceContainer i6i7 = ipv4.Assign (d9d7);

    //傳送節點4號 加上8、9號返回之路由
    Ptr<Ipv4> ipv4_n4 = c.Get(4)->GetObject<Ipv4> ();
    Ipv4StaticRoutingHelper ipv4RoutingHelper; 
    Ptr<Ipv4StaticRouting> staticRouting_n4 = ipv4RoutingHelper.GetStaticRouting (ipv4_n4); 
    staticRouting_n4->AddHostRouteTo (Ipv4Address ("10.1.5.2"), Ipv4Address ("10.1.9.2") ,1); 
    staticRouting_n4->AddHostRouteTo (Ipv4Address ("10.1.7.2"), Ipv4Address ("10.1.10.2") ,2); 
    //接收節點5號 加上1、6前送之路由
    Ptr<Ipv4> ipv4_n5 = c.Get(5)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> staticRouting_n5 = ipv4RoutingHelper.GetStaticRouting (ipv4_n5); 
    staticRouting_n5->AddHostRouteTo (Ipv4Address ("10.1.4.1"), Ipv4Address ("10.1.9.1") ,1); 
    staticRouting_n5->AddHostRouteTo (Ipv4Address ("10.1.6.1"), Ipv4Address ("10.1.10.1") ,2); 

    // Create router nodes, initialize routing database and set up the routing
    // tables in the nodes.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    
    uint16_t port2 = 9;  // well-known echo port number
    
    MpquicBulkSendHelper source ("ns3::QuicSocketFactory",
                            InetSocketAddress (i8i5.GetAddress (1), port2));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    //定義整個app的來源為4
    ApplicationContainer sourceApps = source.Install (c.Get (4));
    sourceApps.Start (Seconds (start_time));
    sourceApps.Stop (Seconds(simulationEndTime));
    //網路流目標為5
    PacketSinkHelper sink2 ("ns3::QuicSocketFactory",
                            InetSocketAddress (Ipv4Address::GetAny (), port2));
    ApplicationContainer sinkApps2 = sink2.Install (c.Get (5));
    sinkApps2.Start (Seconds (0.0));
    sinkApps2.Stop (Seconds(simulationEndTime));

    //輸出記錄檔案為scheduler{schedulerType}-rx-mpquic4G.txt
    std::ostringstream file;
    file<<"./scheduler" << schedulerType;

    AsciiTraceHelper asciiTraceHelper;
    std::ostringstream fileName;
    fileName <<  "./scheduler" << schedulerType << "-rx-mpquic4G" << ".txt";
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName.str ());
  
    //用額外的參數去定義要記錄哪一些
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
    ThroughputMonitor(&flowmon, monitor, stream); 
    
    //每兩秒交換一次頻寬&延遲
    for (double i = 1; i < simulationEndTime; i = i+2){
        Simulator::Schedule (Seconds (i), &ModifyLinkRate, &d1d8, DataRate(std::to_string(rateVal0->GetValue())+"Mbps"), Time::FromInteger(delayVal0->GetValue(), Time::MS));
        Simulator::Schedule (Seconds (i), &ModifyLinkRate, &d6d9, DataRate(std::to_string(rateVal1->GetValue())+"Mbps"), Time::FromInteger(delayVal1->GetValue(), Time::MS));
    }


    Simulator::Stop (Seconds(simulationEndTime));
    NS_LOG_INFO("\n\n#################### STARTING RUN ####################\n\n");
    NS_LOG_UNCOND("Starting simulation...");
    Simulator::Run();
    NS_LOG_UNCOND("Simulation finished.");
    //把flow1、3 輸出遺漏的封包
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        if (i->first == 1 || i->first == 3){

        NS_LOG_INFO("Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")"
        << "\n Last rx Seconds: " << i->second.timeLastRxPacket.GetSeconds()
        << "\n Rx Bytes: " << i->second.rxBytes
        << "\n DelaySum(s): " << i->second.delaySum.GetSeconds()
        << "\n rxPackets: " << i->second.rxPackets);
        }
        
    }
    //技術總結
    NS_LOG_INFO("\nfile size: "<<maxBytes/1024<< "MB, scheduler type " <<schedulerType<<
                "\npath 0: rate "<< rate0a <<", delay "<< delay0a << 
                "\npath 1: rate " << rate1a << ", delay " << delay1a );

    Simulator::Destroy ();

    return 0;
}

