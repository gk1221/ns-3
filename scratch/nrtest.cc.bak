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
#include "ns3/point-to-point-epc-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RANWithQUICIntegration");

int main(int argc, char *argv[]) {
    uint16_t gNbNum = 1;
    uint16_t ueNumPergNb = 1;
    double centralFrequencyBand1 = 28e9;
    double bandwidthBand1 = 400e6;

    LogComponentEnable("EpcPgwApplication", LOG_LEVEL_ALL);

    // Parse command line
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

    // EPC Helper Setup
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    nrHelper->SetEpcHelper(epcHelper);

    // Install devices on gNB and UE
    NetDeviceContainer enbDevices = nrHelper->InstallGnbDevice(gridScenario.GetBaseStations());
    NetDeviceContainer ueDevices = nrHelper->InstallUeDevice(gridScenario.GetUserTerminals());

    // Attach UE to gNB
    for (uint32_t i = 0; i < ueDevices.GetN(); ++i) {
        nrHelper->Attach(ueDevices.Get(i), enbDevices.Get(0));
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
