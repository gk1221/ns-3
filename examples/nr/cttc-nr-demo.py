from ns import ns
def main(argv):
    # Default simulation parameters
    gNbNum = 1
    ueNumPergNb = 2
    logging = False
    doubleOperationalBand = True
    udpPacketSizeULL = 100
    udpPacketSizeBe = 1252
    lambdaULL = 10000
    lambdaBe = 10000
    simTime = ns.MilliSeconds(1000)
    udpAppStartTime = ns.MilliSeconds(400)

    # NR parameters (Ref: 3GPP TR 38.901 V17.0.0 (Release 17)
    numerologyBwp1 = 4
    centralFrequencyBand1 = 28e9
    bandwidthBand1 = 50e6
    numerologyBwp2 = 2
    centralFrequencyBand2 = 28.2e9
    bandwidthBand2 = 50e6
    totalTxPower = 35

    simTag = "default"
    outputDir = "./"

    # Parse command-line arguments (you may add more based on your needs)
    cmd = ns.CommandLine()
    cmd.AddValue("gNbNum", "The number of gNbs in multiple-ue topology", gNbNum)
    cmd.AddValue("ueNumPergNb", "The number of UE per gNb in multiple-ue topology", ueNumPergNb)
    cmd.AddValue("logging", "Enable logging", logging)
    cmd.AddValue("doubleOperationalBand",
                 "Simulate two operational bands", doubleOperationalBand)
    cmd.AddValue("packetSizeUll", "packet size in bytes for ultra low latency traffic", udpPacketSizeULL)
    cmd.AddValue("packetSizeBe", "packet size in bytes for best effort traffic", udpPacketSizeBe)
    cmd.AddValue("lambdaUll", "Number of UDP packets per second for ultra low latency traffic", lambdaULL)
    cmd.AddValue("lambdaBe", "Number of UDP packets per second for best effort traffic", lambdaBe)
    cmd.AddValue("simTime", "Simulation time", simTime)
    cmd.AddValue("numerologyBwp1", "Numerology for bandwidth part 1", numerologyBwp1)
    cmd.AddValue("centralFrequencyBand1", "System frequency for band 1", centralFrequencyBand1)
    cmd.AddValue("bandwidthBand1", "System bandwidth for band 1", bandwidthBand1)
    cmd.AddValue("numerologyBwp2", "Numerology for bandwidth part 2", numerologyBwp2)
    cmd.AddValue("centralFrequencyBand2", "System frequency for band 2", centralFrequencyBand2)
    cmd.AddValue("bandwidthBand2", "System bandwidth for band 2", bandwidthBand2)
    cmd.AddValue("totalTxPower", "Total transmission power", totalTxPower)
    cmd.AddValue("simTag", "Tag for output files", simTag)
    cmd.AddValue("outputDir", "Directory for output files", outputDir)
    cmd.Parse(argv)

    # Check frequency range
    ns.NS_ABORT_IF(centralFrequencyBand1 < 0.5e9 or centralFrequencyBand1 > 100e9)
    ns.NS_ABORT_IF(centralFrequencyBand2 < 0.5e9 or centralFrequencyBand2 > 100e9)

    if logging:
        ns.LogComponentEnable("UdpClient", ns.LOG_LEVEL_INFO)
        ns.LogComponentEnable("UdpServer", ns.LOG_LEVEL_INFO)
        ns.LogComponentEnable("LtePdcp", ns.LOG_LEVEL_INFO)

    # NR setup and grid scenario creation
    gridScenario = ns.nr.GridScenarioHelper()
    gridScenario.SetRows(1)
    gridScenario.SetColumns(gNbNum)
    gridScenario.SetHorizontalBsDistance(10.0)
    gridScenario.SetVerticalBsDistance(10.0)
    gridScenario.SetBsHeight(10)
    gridScenario.SetUtHeight(1.5)
    gridScenario.SetSectorization(ns.nr.GridScenarioHelper.SINGLE)
    gridScenario.SetBsNumber(gNbNum)
    gridScenario.SetUtNumber(ueNumPergNb * gNbNum)
    gridScenario.SetScenarioHeight(3)
    gridScenario.SetScenarioLength(3)
    gridScenario.CreateScenario()

    # Ue containers for low latency and voice
    ueLowLatContainer = ns.network.NodeContainer()
    ueVoiceContainer = ns.network.NodeContainer()

    for j in range(gridScenario.GetUserTerminals().GetN()):
        ue = gridScenario.GetUserTerminals().Get(j)
        if j % 2 == 0:
            ueLowLatContainer.Add(ue)
        else:
            ueVoiceContainer.Add(ue)

    # NR configuration helpers
    epcHelper = ns.nr.NrPointToPointEpcHelper()
    idealBeamformingHelper = ns.nr.IdealBeamformingHelper()
    nrHelper = ns.nr.NrHelper()

    nrHelper.SetBeamformingHelper(idealBeamformingHelper)
    nrHelper.SetEpcHelper(epcHelper)

    # Spectrum division
    ccBwpCreator = ns.nr.CcBwpCreator()
    bandConf1 = ns.nr.CcBwpCreator.SimpleOperationBandConf(
        centralFrequencyBand1, bandwidthBand1, 1, ns.nr.BandwidthPartInfo.UMi_StreetCanyon)
    bandConf2 = ns.nr.CcBwpCreator.SimpleOperationBandConf(
        centralFrequencyBand2, bandwidthBand2, 1, ns.nr.BandwidthPartInfo.UMi_StreetCanyon)

    band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf1)
    band2 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf2)

    if doubleOperationalBand:
        nrHelper.InitializeOperationBand(band1)
        nrHelper.InitializeOperationBand(band2)
        allBwps = ns.nr.CcBwpCreator.GetAllBwps([band1, band2])
    else:
        nrHelper.InitializeOperationBand(band1)
        allBwps = ns.nr.CcBwpCreator.GetAllBwps([band1])

    # Continue setting up the rest of the simulation...
    # This includes setting attributes for antennas, installing applications,
    # traffic generation, running the simulator, and collecting statistics.

    ns.Simulator.Stop(simTime)
    ns.Simulator.Run()
    ns.Simulator.Destroy()

if __name__ == '__main__':
    import sys
    main(sys.argv)
