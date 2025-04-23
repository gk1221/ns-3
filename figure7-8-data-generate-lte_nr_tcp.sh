## generate data for figure 7

# echo "...generating data for figure 7"
# for i in {1..50}
# do
#    echo "DOING ${i}"
#    ./exp-wns3-two-path-lte_nr_tcp.sh $i

# done
# echo "...figure 7 data generated in results-wns3"


## generate data for figure 8
echo "...generating data for figure 8"
./exp-wns3-two-path-cwnd-ltenr-tcp.sh 0 1
./exp-wns3-two-path-cwnd-ltenr-tcp.sh 1 1
./exp-wns3-two-path-cwnd-ltenr-tcp.sh 2 1
./exp-wns3-two-path-cwnd-ltenr-tcp.sh 3 1
./exp-wns3-two-path-cwnd-ltenr-tcp.sh 4 1

echo "...figure 8 data generated in results-wns3"