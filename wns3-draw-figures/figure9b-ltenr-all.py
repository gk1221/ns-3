import pandas as pd
import matplotlib.pyplot as plt

# 設定字體
plt.rcParams["font.family"] = "serif"
plt.rcParams["font.serif"] = "Times New Roman"

# 設定資料夾
topDir = '../results-wns3/'

# 定義函式來讀取數據
def read_throughput_data(folder, scheduler_id):
    """ 讀取 scheduler-ltenr-X 的數據 (0~4 排程器) """
    file_path = f"{topDir}{folder}/scheduler{scheduler_id}-queue.txt"
    
    try:
        with open(file_path, 'r') as file:
            finish_time = float(file.readlines()[-1].split('\t')[0])

        file_path = f"{topDir}{folder}/scheduler{scheduler_id}-rx-ltenr-tcp.txt"
        with open(file_path, 'r') as file:
            lines = file.readlines()
        
        goodput = []
        c_time = []
        for line in lines:
            temp = line.split('\t')
            if temp[0] == '5' and not c_time:
                c_time.append(float(temp[1]))
                c_time.append(float(temp[5]))
            if temp[0] == '7' and len(c_time) == 2:
                c_time[1] += float(temp[5])
                c_time.append(scheduler_id)
                goodput.append(c_time)
                if c_time[0] > finish_time:
                    break
                c_time = []

        return pd.DataFrame(goodput, columns=['Time', 'goodput', 'scheduler'])
    
    except FileNotFoundError:
        print(f"⚠️ Warning: File not found in {folder} for scheduler {scheduler_id}")
        return pd.DataFrame(columns=['Time', 'goodput', 'scheduler'])

# 建立 5x2 子圖
fig, axes = plt.subplots(5, 2, figsize=(15, 20))  # 5 列 x 2 欄
fig.suptitle("Instantaneous Throughput for Different Schedulers", fontsize=20, fontweight="bold")

# 顏色列表 (對應 0~4 排程器)
colors = ["green", "red", "blue", "c", "orange"]
scheduler_labels = ["RR", "MRTT", "BLEST", "ECF", "Peekaboo"]

#34
index = 7
for idx in range(index*10+1, (index+1)*10+1):  # 17 
    folder_name = f"unfair-0008-tcp-all-flow-OLIA08a-{idx}"
    row, col = divmod(idx - (index*10+1), 2)  # 計算子圖位置
    ax = axes[row, col]
    
    # 讀取 0~4 排程器數據
    for scheduler_id in range(5):  # 0~4
        data = read_throughput_data(folder_name, scheduler_id)
        if not data.empty:
            ax.plot(data['Time'], data['goodput'], label=scheduler_labels[scheduler_id], linewidth=2, color=colors[scheduler_id])
    
    ax.set_title(folder_name, fontsize=16, fontweight="bold")
    ax.set_xlabel("Time (s)", fontsize=12)
    ax.set_ylabel("Throughput (Mbps)", fontsize=12)
    ax.grid(linestyle="--")
    ax.legend(fontsize=10)

# 調整子圖間距
plt.tight_layout(rect=[0, 0, 1, 0.96])  

# 儲存圖表
output_path = f"unfair-0008-tcp-all-flow-OLIA08a-throughput-all-schedulers.png"
plt.savefig(output_path, format='png')
print(f"✅ Save in {output_path}")

