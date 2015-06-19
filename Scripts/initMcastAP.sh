
ifconfig $1 up 192.168.1.1 netmask 255.255.255.0
sleep 2

if [ "$(ps -e | grep dhcpd)" == "" ]; then
	dhcpd $1 &
fi

iptables --flush
iptables --table nat --flush
iptables --delete-chain
iptables --table nat --delete-chain
iptables --table nat --append POSTROUTING --out-interface $2 -j MASQUERADE
iptables --append FORWARD --in-interface $1 -j ACCEPT


sysctl -w net.ipv4.ip_forward=1

hostapd /etc/hostapd/hostapd.conf 
killall dhcpd	
