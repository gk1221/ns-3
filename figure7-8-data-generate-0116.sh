## generate data for figure 7

echo "...generating data for figure 7"
for i in {1..50}
do
   echo "DOING ${i}"
   ./exp-wns3-two-path-0116.sh $i

done
echo "...figure 7 data generated in results-wns3"

