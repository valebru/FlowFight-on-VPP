#!/bin/bash
dt=$(date '+%d-%m_%H-%M');

echo "VPP packet generator startup"
sudo -E $BINS/vppctl -s /run/vpp/cli.sock exec $FF_DIR/tools_vpp/vpp_script_2streams
#sudo -E $BINS/vppctl -s /run/vpp/cli.sock exec $FF_DIR/tools_vpp/vpp_script_pcap

filename="${EXP_RES}/Basetest_res_${dt}.txt"
echo "saving results: ${filename}"
sudo -E $BINS/vppctl -s /run/vpp/cli.sock hll count > ${filename}

cat ${filename}


#sudo -E $BINS/vppctl -s /run/vpp/cli.sock 
