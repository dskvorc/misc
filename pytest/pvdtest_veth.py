import sys
import socket
from netaddr import IPNetwork
from pyroute2 import IPRoute
from pyroute2 import netns


def getPvdIfName(pvdId):
    """
    Mock-up function to simulate the mapping of PvDs to interfaces
    """
    return pvdId


def getPvdNetnsName(pvdId):
    return 'pvd-' + pvdId;


def getPvdVethNames(pvdId):
    # Max length of veth name is 15
    return ('pvd-veth0-' + pvdId[0:5], 'pvd-veth1-' + pvdId[0:5])


def getPvdBridgeName(pvdId):
    # Max length of veth name is 15
    return ('pvd-br-' + pvdId[0:5])


def main(argv):
    if (len(argv) !=  2):
        print('Usage: python pvdtest_veth.py PVD_ID')
        print('PVD_ID := name of the interface that contains PvD-related network configuration')
        sys.exit()

    # Get user-entered PVD ID
    pvdId = sys.argv[1]
    
    # Create IPRoute object for manipulation with a default network namespace
    ipMain = IPRoute()

    # Create a PvD-related network namespace
    pvdNetnsName = getPvdNetnsName(pvdId);
    if (pvdNetnsName in netns.listnetns()):
        netns.remove(pvdNetnsName)
    netns.create(pvdNetnsName)

    # Create IPRoute object for manipulation with a PvD-related network namespace
    netns.setns(pvdNetnsName)
    ipPvd = IPRoute()

    # Activate loopback interface in a PvD-related network namespace
    loIndex = ipPvd.link_lookup(ifname='lo')[0]
    ipPvd.link_up(loIndex)

    # Get addresses from a PvD-related network interface
    pvdIfIndex = ipMain.link_lookup(ifname=getPvdIfName(pvdId))[0]
    pvdAddresses = ipMain.get_addr(index=pvdIfIndex)

    # Get current routes
    pvdRoutes = ipMain.get_routes()

    # Create bridge
    bridge = getPvdBridgeName(pvdId)
    ipMain.link_create(ifname=bridge, kind='bridge')

    # Create veth interface
    (veth0, veth1) = getPvdVethNames(pvdId)
    ipMain.link_create(ifname=veth0, kind='veth', peer=veth1)

    # Move one end of the veth interafce to a PvD-related network namespace
    veth1Index = ipMain.link_lookup(ifname=veth1)[0]
    ipMain.link('set', index=veth1Index, net_ns_fd=pvdNetnsName)

    # Shut down and remove addresses from the PvD interface before adding it to the bridge
    ipMain.link_down(pvdIfIndex)
    ipMain.flush_addr(pvdIfIndex)

    # Make a bridge between PvD interface and one end of veth interface
    veth0Index = ipMain.link_lookup(ifname=veth0)[0]
    bridgeIndex = ipMain.link_lookup(ifname=bridge)[0]
    ipMain.link('set', index=veth0Index, master=bridgeIndex)
    ipMain.link('set', index=pvdIfIndex, master=bridgeIndex)

    # Activate bridge and connected interfaces
    ipMain.link_up(pvdIfIndex)
    ipMain.link_up(veth0Index)
    ipMain.link_up(bridgeIndex)
    ipPvd.link_up(veth1Index)

    # Configure bridge and another end of the veth interface with PvD-related network parameters + add routes
    ipMain.flush_routes()
    ipPvd.flush_routes()
    for address in pvdAddresses:
        ipAddress = broadcastAddress = netmask = addrFamily = None
        for attr in address['attrs']:
            if attr[0] == 'IFA_ADDRESS':
                ipAddress = attr[1]
            if attr[0] == 'IFA_BROADCAST':
                broadcastAddress = attr[1]
        netmask = address['prefixlen']
        addrFamily = address['family']
        # Configure bridge
        try:
            ipMain.addr('add', index=bridgeIndex, family=addrFamily, address=ipAddress, broadcast=broadcastAddress, mask=netmask)
        except:
            pass
        # Configure veth
        try:
            ipPvd.addr('add', index=veth1Index, family=addrFamily, address=ipAddress, broadcast=broadcastAddress, mask=netmask)
        except:
            pass
        # Configure routes
        # Some routes are added during interface IP address/netmask configuration, skip them if already there
        ipNetwork = IPNetwork(ipAddress + '/' + str(netmask))
        try:
            ipMain.route('add', dst=str(ipNetwork.network), mask=netmask, oif=bridgeIndex, src=ipAddress, rtproto='RTPROT_STATIC', rtscope='RT_SCOPE_LINK')
        except:
            pass
        try:
            ipPvd.route('add', dst=str(ipNetwork.network), mask=netmask, oif=veth1Index, src=ipAddress, rtproto='RTPROT_STATIC', rtscope='RT_SCOPE_LINK')
        except:
            pass

    # Fing gateway(s) and add default routes
    defGateways = []
    for route in pvdRoutes:
        for attr in route['attrs']:
            if (attr[0] == 'RTA_GATEWAY' and attr[1] not in defGateways):
                defGateways.append(attr[1])
    if (len(defGateways) > 0):
        ipMain.route('add', dst='0.0.0.0', oif=bridgeIndex, gateway=defGateways[0], rtproto='RTPROT_STATIC')
        ipPvd.route('add', dst='0.0.0.0', oif=veth1Index, gateway=defGateways[0], rtproto='RTPROT_STATIC')


main(sys.argv)
