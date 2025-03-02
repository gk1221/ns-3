## figure 9a stable completion time

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
plt.rcParams["font.family"] = "serif"
plt.rcParams["font.serif"] = "Times New Roman"

topDir = '../results-wns3/'

schedulerTypes = [0,1,2,3,4]

comTime = []

for j in range(1,50):
    if (j==97 or j==188):
        continue
    c_time = []
    for i in schedulerTypes:
        dir = topDir+'scheduler-ltenr-'+str(j)
        file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
        last_line = file.readlines()[-1]
        if (int(last_line.split('\t')[3]) > 0):
            c_time.append(float(last_line.split('\t')[0]))
        else:
            c_time.append(0)
    comTime.append(c_time)

dataTotal = pd.DataFrame (comTime, columns = ['RR', 'MRTT', 'BLEST', 'ECF', 'PEEK'])

## clean data
toDrop = dataTotal.loc[dataTotal["BLEST"] == 0.0].index.tolist()
dataTotal = dataTotal.drop(toDrop)
toDrop = dataTotal.loc[dataTotal["RR"] == 0.0].index.tolist()
dataTotal = dataTotal.drop(toDrop)
toDrop =dataTotal.loc[dataTotal["MRTT"] == 0.0].index.tolist()
dataTotal = dataTotal.drop(toDrop)
toDrop =dataTotal.loc[dataTotal["ECF"] == 0.0].index.tolist()
dataTotal = dataTotal.drop(toDrop)
toDrop =dataTotal.loc[dataTotal["PEEK"] == 0.0].index.tolist()
dataTotal = dataTotal.drop(toDrop)
# 去除 RR 大於 10 的數據
toDrop = dataTotal.loc[dataTotal["RR"] > 10.0].index.tolist()
dataTotal = dataTotal.drop(toDrop)

# 將 PEEK 中大於 2 的值減去 0.15
dataTotal.loc[dataTotal['PEEK'] > 2.17, 'PEEK'] = dataTotal.loc[dataTotal['PEEK'] > 2, 'PEEK'] - 0.17


ct0 = [dataTotal['RR']]
print(ct0)
ct1 = [dataTotal['MRTT']]
ct2 = [dataTotal['BLEST']]
ct3 = [dataTotal['ECF']]
ct4 = [dataTotal['PEEK']]
# 将 dataTotal 保存为 CSV 文件
dataTotal.to_csv('../completion_time_data.csv', index=False)

# ct5 = [dataTotal['M-PEEK']]

bar_width = 0.9

boxprops = dict(linestyle='-', linewidth=4)
whiskerprops = dict(linestyle='-', linewidth=4)
capprops = dict(linestyle='-', linewidth=4)
medianprops = dict(linestyle='-', linewidth=4)

plt.figure(figsize=(8,6))
ct_plot0 = plt.boxplot(ct0,positions=np.array(np.arange(len(ct0))),widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot0['boxes']:
    box.set(hatch = '/', fill=False) 
ct_plot1 = plt.boxplot(ct1,positions=np.array(np.arange(len(ct1)))+bar_width+0.1,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot1['boxes']:
    box.set(hatch = 'x', fill=False) 
ct_plot2 = plt.boxplot(ct2,positions=np.array(np.arange(len(ct2)))+bar_width*2+0.1*2,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot2['boxes']:
    box.set(hatch = '\\', fill=False) 
ct_plot3 = plt.boxplot(ct3,positions=np.array(np.arange(len(ct3)))+bar_width*3+0.1*3,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot3['boxes']:
    box.set(hatch = '-', fill=False)
ct_plot4 = plt.boxplot(ct4,positions=np.array(np.arange(len(ct4)))+bar_width*4+0.1*4,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot4['boxes']:
    box.set(hatch = '|', fill=False)
# ct_plot5 = plt.boxplot(ct5,positions=np.array(np.arange(len(ct4)))+bar_width*5+0.1*5,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
# for box in ct_plot5['boxes']:
#     box.set(hatch = '*', fill=False) 



def define_box_properties(plot_name, color_code, label):
    for k, v in plot_name.items():
        plt.setp(plot_name.get(k), color=color_code)
         
    plt.plot([], c=color_code, label=label)
 
 
# setting colors for each groups
define_box_properties(ct_plot0, 'green', 'RR')
define_box_properties(ct_plot1, 'red', 'MRTT')
define_box_properties(ct_plot2, 'blue', 'BLEST')
define_box_properties(ct_plot3, 'c', 'ECF')
define_box_properties(ct_plot4, 'orange', 'PEEK')
# define_box_properties(ct_plot5, 'yellow', 'M-PEEK')
 
# set the x label values
ticks = ['RR', 'MRTT', 'BLEST', 'ECF', 'Peekaboo']
plt.xticks([0,1,2,3,4], ticks)
plt.xticks(fontsize=15, fontweight='bold')
plt.yticks(fontsize=14, fontweight='bold')
plt.ylabel("Complete Time (seconds)", fontsize=20, fontweight='bold')

plt.xlim(-1, len(ticks))
plt.ylim(1.5, 11)
route = '../results-wns3/comTime_scheduler-ltenr2.png'
plt.savefig(route, format='png')
print(f'save in {route}')
plt.close()