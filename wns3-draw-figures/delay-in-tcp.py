import pandas as pd
import matplotlib.pyplot as plt
import re
from pathlib import Path
from collections import defaultdict

cctype="OLIA08a"
base_dir = Path("/Users/chiyune/Desktop/project/ns-3/results-wns3")
queue_files = sorted(base_dir.glob(f"unfair-0008-tcp-all-flow-{cctype}-*/scheduler*-rx-ltenr-tcp.txt"))

records = []
for file in queue_files:
    df = pd.read_csv(file, sep="\t", header=None, names=[
        "flowId", "time", "rxBytes", "rxPackets", "lastDelay_ms", "throughput_Mbps"
    ])
    df = df[df["flowId"].isin([9, 10])]
    df["filename"] = file.name
    # 從檔名中提取scheduler編號
    scheduler_match = re.search(r"scheduler(\d+)-rx", file.name)
    if scheduler_match:
        scheduler_id = int(scheduler_match.group(1))
        df["scheduler"] = scheduler_id
    
    records.append(df)

# 3. 合併所有檔案資料
df_all = pd.concat(records, ignore_index=True)

# 4. 算每個檔案 + flow 的 delay 總和、平均
summary = df_all.groupby(["filename", "flowId"]).agg(
    total_delay_ms=("lastDelay_ms", "sum"),
    avg_delay_ms=("lastDelay_ms", "mean"),
    sample_count=("lastDelay_ms", "count")
).reset_index()

df_part = df_all[["flowId", "lastDelay_ms", "scheduler"]]
# 根據scheduler分組計算平均延遲
scheduler_summary = df_all.groupby("scheduler")["lastDelay_ms"].agg([
    ("平均延遲", "mean"),
    ("最大延遲", "max")
]).round(3)
print("各排程器的延遲統計:")
print(scheduler_summary)

# 畫 box plot, x軸為 scheduler 編號
plt.figure(figsize=(12, 6))
df_part.boxplot(column='lastDelay_ms', by='scheduler')
plt.title(f'Delay Distribution by Scheduler and Flow {cctype}')
plt.xlabel('(Scheduler, Flow)')
plt.ylabel('Delay (ms)')
plt.suptitle('')  # 移除自動產生的標題
plt.grid(True)
plt.xticks(rotation=0)
plt.tight_layout()
path=f'../Delay-tcp-interference-{cctype}.png'
plt.savefig(path, format='png')
print(f'save fig in {path}')
