# Qnixx

## Tapping Network Packets
Qnixx's network stack is not done yet, so some testing is required. Using Wireshark or tcpdump is suggested for viewing packet contents. Run the commands below and select `tap0` in Wireshark or `sudo tcpdump -i tap0`
```sh
sudo ip tuntap add user $USER mode tap
sudo ip link set dev tap0 up
```

## Prebuilt ISOs
Go to [the most recent job](https://github.com/Qnixx/src/actions) and Qnixx.iso will be located under Artifacts.

## Etc
Come hang out with us on [our discord!](https://discord.gg/2rtRsbm4Am)
