setup_tap() {
  sudo ip tuntap add user $USER mode tap
  sudo ip address add 192.168.1.166 dev tap0
  sudo ip link set dev tap0 up
}

ethtool -i tap0 &> /dev/null || setup_tap
