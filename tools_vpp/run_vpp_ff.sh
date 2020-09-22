#!/bin/bash

sudo killall vpp_main
sleep 2

echo "Starting VPP"
sudo $BINS/vpp `cat $STARTUP_CONF` &

sleep 8
echo "Creating loopback interface"
sudo -E $BINS/vppctl -s /run/vpp/cli.sock loop create
sudo -E $BINS/vppctl -s /run/vpp/cli.sock set int state loop0 up

echo "Installing FlowFight plugin"
sudo -E $BINS/vppctl -s /run/vpp/cli.sock hll start bits $2 mode 4 multi $1 loop0

#sudo -E $BINS/vppctl -s /run/vpp/cli.sock exec $FF_DIR/tools_vpp/vpp_script_2streams
#sudo -E $BINS/vppctl -s /run/vpp/cli.sock exec $FF_DIR/tools_vpp/vpp_script_pcap
#sudo -E $BINS/vppctl -s /run/vpp/cli.sock hll count
#sudo -E $BINS/vppctl -s /run/vpp/cli.sock 
