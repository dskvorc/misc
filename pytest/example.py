from pyroute2 import IPRoute
ip = IPRoute()
print([x.get_attr('IFLA_IFNAME') for x in ip.get_links()])
