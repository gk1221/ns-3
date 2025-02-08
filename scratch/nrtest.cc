#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/nr-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  // 初始化命令列參數
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // 建立節點：1 個 gNB 和 2 個 UE
  NodeContainer gnbNodes;
  gnbNodes.Create (1);
  NodeContainer ueNodes;
  ueNodes.Create (2);

  // 建立 NR Helper 並設定 EPC（核心網路）
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  nrHelper->SetEpcHelper (epcHelper);

  // 安裝網際網路協定堆疊到 UE
  InternetStackHelper internet;
  internet.Install (ueNodes);

  // 設定 gNB 和 UE 的頻譜配置
//   nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (1)); // 對應 30 kHz 子載波間隔
//   nrHelper->SetGnbPhyAttribute ("Frequency", DoubleValue (28e9)); // 頻率設為 28 GHz
//   nrHelper->SetGnbPhyAttribute ("Bandwidth", UintegerValue (100)); // 頻寬設為 100 RBs

  // 安裝 NR 裝置到 gNB 和 UE
  BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
    // a single BWP per CC
    CcBwpCreator::SimpleOperationBandConf bandConf1(28e9,
                                                    50e6,
                                                    1,
                                                    BandwidthPartInfo::UMi_StreetCanyon);
    // By using the configuration created, it is time to make the operation bands
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf1);
    
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetSchedulerAttribute("FixedMcsDl", BooleanValue(true));
    nrHelper->SetSchedulerAttribute("StartingMcsDl", UintegerValue(28));
    nrHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    
    nrHelper->InitializeOperationBand(&band1);
    allBwps = CcBwpCreator::GetAllBwps({band1});

    // 設定移動模型
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (gnbNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);

  NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  // 為 UE 分配 IP 位址
  Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (ueNetDev);

  // 將 UE 附著到 gNB
  nrHelper->AttachToClosestEnb(ueNetDev, gnbNetDev);

  

  // 安裝應用程式到 UE
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (0))); // 第一個 UE 作為伺服器

  UdpClientHelper dlClient (ueIpIface.GetAddress (0), dlPort);
  dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds (10)));
  dlClient.SetAttribute ("MaxPackets", UintegerValue (1000));
  clientApps.Add (dlClient.Install (gnbNodes.Get (0))); // gNB 作為客戶端

  serverApps.Start (Seconds (0.1));
  clientApps.Start (Seconds (0.1));
  serverApps.Stop (Seconds (1.0));
  clientApps.Stop (Seconds (1.0));

  // 啟動模擬
  Simulator::Stop (Seconds (1.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}