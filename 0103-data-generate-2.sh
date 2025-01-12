## generate data for figure 7

echo "...generating data for 0103-2.cc"
for i in {1..50}
do
   echo "DOING ${i}"
   ./exp-0103-2.sh $i

done
echo "...0103-2.cc data generated in results-wns3"

