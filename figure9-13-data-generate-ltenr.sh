## generate data for figure 9 - 13
echo "...generating data for figure 9-13"

for i in {1..50}
do
    echo "Doing in ${i}"

    # 啟動第一個測試，並設定 60 秒超時
    ./exp-wns3-two-path-scheduler-ltenr.sh $i   # 背景執行
    # pid=$!  # 取得進程 ID
    # sleep 60 && kill -9 $pid 2>/dev/null && echo "Process scheduler ${i} timed out, moving to next..." &
    # wait $pid 2>/dev/null  # 等待測試完成或超時

    # 啟動第二個測試，並設定 60 秒超時
    ./exp-wns3-two-path-scheduler-unstable-ltenr.sh $i   # 背景執行
    # pid=$!  # 取得進程 ID
    # sleep 60 && kill -9 $pid 2>/dev/null && echo "Process scheduler-unstable ${i} timed out, moving to next..." &
    # wait $pid 2>/dev/null  # 等待測試完成或超時

done

echo "...figure 9-13 data generated"