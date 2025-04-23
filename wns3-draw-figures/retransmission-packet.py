import pandas as pd
import matplotlib.pyplot as plt
import re
from pathlib import Path
from collections import defaultdict

cctype='OLIA08a'
base_dir = Path("/Users/chiyune/Desktop/project/ns-3/results-wns3")
rx_files = sorted(base_dir.glob(f"unfair-0008-tcp-all-flow-{cctype}-*/scheduler*-rx-ltenr-tcp.txt"))

# 收集：每個 scheduler 對應多個 retrans 值
scheduler_retrans = defaultdict(list)

for file in rx_files:
    
    olia_match = re.search(r"OLIA08a-(\d+)", str(file))
    scheduler_match = re.search(r"scheduler(\d+)-rx", str(file))
    if olia_match and scheduler_match:
        scheduler_id = int(scheduler_match.group(1))

        lines = file.read_text().splitlines()
        retrans = 0
        for line in lines:
            if re.match(r"^\d+\t\d+\.\d+\t\d+\t\d+\t\d+\t\d+", line.strip()):
                parts = line.strip().split("\t")
                retrans += int(parts[4])
        scheduler_retrans[scheduler_id].append(retrans)

# 準備 DataFrame for boxplot
df_box = pd.DataFrame(dict(scheduler_retrans)).sort_index()

print(df_box)
# 根據scheduler分組計算平均延遲
# 計算每個scheduler的平均和最大重送數量
scheduler_summary = pd.DataFrame()
for col in df_box.columns:
    mean_retrans = df_box[col].mean()
    max_retrans = df_box[col].max() 
    scheduler_summary.loc[col, "平均重送數量"] = mean_retrans
    scheduler_summary.loc[col, "最大重送數量"] = max_retrans

print("\n各排程器的重送統計:")
print(scheduler_summary.round(3))


# 畫 box plot
plt.figure(figsize=(12, 6))
df_box.boxplot()
plt.title(f"Box Plot of Retransmissions per Scheduler across {cctype} Versions")
plt.xlabel("Scheduler ID")
plt.ylabel("Total Retransmissions")
plt.grid(True, axis='y')
plt.tight_layout()
route = f'../results-wns3/retransmission-{cctype}.png'
plt.savefig(route, format='png')
print(f'save fig in {route}')