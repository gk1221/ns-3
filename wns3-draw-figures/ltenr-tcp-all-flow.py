import glob
import pandas as pd
import matplotlib.pyplot as plt

# 取得所有符合路徑規則的檔案 (可調整成實際放置的路徑)
files = glob.glob('../results-wns3/unfair-0008-tcp-all-flow-OLIA08a-**/scheduler*-rx-ltenr-tcp.txt')

# 用來儲存每個 flowid 的「首度 packet > 20000」時間
flow_data = {}

for file_path in files:
    # 讀取檔案，假設共有6欄: flowid, time, packet, col4, col5, col6
    df = pd.read_csv(
        file_path, 
        sep='\s+', 
        header=None,
        names=['flowid','time','packet','col4','col5','col6']
    )
    
    # 篩選出 packet > 20000 的資料
    subset = df[df['packet'] > 2000000]
    
    #print(subset)
    if subset.empty:
        # 這個檔案沒有任何 packet > 20000，跳過
        continue
    
    # 依照 flowid 分組，找出在該 flowid 下「第三欄首度 > 20000」的最小 time
    # groupby('flowid') 之後，用 .time.min() 即可取得該 group 最早達到條件的時間
    first_times = subset.groupby('flowid')['time'].min()
    
    # 將這些最早時間存到 flow_data 的對應 flowid 列表裡
    for fid, t in first_times.items():
        if fid not in flow_data:
            flow_data[fid] = []
        flow_data[fid].append(t)

# 若希望以 flowid 為 X 軸標籤，繪製多個盒狀圖
sorted_flow_ids = sorted(flow_data.keys())
all_first_times = [flow_data[fid] for fid in sorted_flow_ids]
#print(all_first_times)
plt.boxplot(all_first_times, tick_labels=sorted_flow_ids, showfliers=True)
plt.xlabel('Flow ID')
plt.ylabel('First time where packet > 2000000')
plt.title('Boxplot of earliest times (packet > 2000000) for unfair flow OLIA08a')
path = '../results-wns3/unfair-0008-tcp-all-flow-OLIA08a.png'
print(f"figure save in {path}")
plt.savefig(path)