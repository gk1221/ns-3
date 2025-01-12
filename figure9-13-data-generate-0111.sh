
## generate data for figure 9 - 13
echo "...generating data for figure 9-13"
for i in {1..100}
do
    echo "Doing in ${i}"
    ./exp-wns3-two-path-scheduler-0103.sh $i
    ./exp-wns3-two-path-scheduler-unstable-0103.sh $i
done

# ## set seed=2
# ./exp-wns3-two-path-scheduler-flip.sh 2
# echo "...figure 9-13 data generated in results-wns3"