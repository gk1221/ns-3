
from ns import ns

# Create nodes
n0 = ns.Node()
n1 = ns.Node()
n2 = ns.Node()
n4 = ns.Node()
n5 = ns.Node()
n8 = ns.Node()

print(n0)

# Install internet stack
internet = ns.InternetStackHelper()
internet.Install(n0)
internet.Install(n1)
internet.Install(n2)
internet.Install(n4)
internet.Install(n5)
internet.Install(n8)

# Create point-to-point channels
p2p = ns.PointToPointHelper()
p2p.SetDeviceAttribute("DataRate", ns.core.StringValue("5Mbps"))
p2p.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))

devices_n1_n8 = p2p.Install(n1, n8)

# Assign IP addresses
address = ns.internet.Ipv4AddressHelper()
address.SetBase(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"))

interfaces_n1_n8 = address.Assign(devices_n1_n8)

# Set up the QUIC server and client applications
quic_server_helper = ns.quic.QuicServerHelper(ns.core.Address(ns.network.InetSocketAddress(interfaces_n1_n8.GetAddress(1), 9000)))
server_apps = quic_server_helper.Install(n8)

quic_client_helper = ns.quic.QuicClientHelper(ns.core.Address(ns.network.InetSocketAddress(interfaces_n1_n8.GetAddress(1), 9000)))
client_apps = quic_client_helper.Install(n1)

# Start applications
server_apps.Start(ns.core.Seconds(1.0))
client_apps.Start(ns.core.Seconds(2.0))

# Run simulation
ns.core.Simulator.Run()
ns.core.Simulator.Destroy()
