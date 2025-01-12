## generate data for figure 7

echo "...generating data for 0103.cc"
for i in {1..50}
do
   echo "DOING ${i}"
   ./exp-0103.sh $i
   ./exp-0111.sh $i

done
echo "...0103.cc data generated in results-wns3"

