## figure 7a completion time

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
plt.rcParams["font.family"] = "serif"
plt.rcParams["font.serif"] = "Times New Roman"

comTime = []
topDir = '../results-wns3/'

for j in range(1,50):
    c_time = []

    # one
    dir = 'one-'+str(j)
    file = open(topDir+dir+'/scheduler0-queue.txt', 'r')
    lines = file.readlines()
    last_line = lines[-1] 
    thp1 = float(last_line.split('\t')[0])
    c_time.append(thp1)
    
    # two
    dir = 'two-'+str(j)
    file = open(topDir+dir+'/scheduler0-queue.txt', 'r')
    lines = file.readlines()
    last_line = lines[-1]  
    thp2 = float(last_line.split('\t')[0])
    c_time.append(thp2)
    
    # four
    dir = 'four-'+str(j)
    file = open(topDir+dir+'/scheduler0-queue.txt', 'r')
    lines = file.readlines()
    last_line = lines[-1] 
    thp3 = float(last_line.split('\t')[0])
    c_time.append(thp3)
    

    
    # 0116-3
    dir = 'two-0116-3-'+str(j)
    file = open(topDir+dir+'/scheduler0-queue.txt', 'r')
    lines = file.readlines()
    last_line = lines[-1] 
    thp5 = float(last_line.split('\t')[0])
    c_time.append(thp5)
    
    # 0128
    dir = 'two-0128-'+str(j)
    file = open(topDir+dir+'/scheduler0-queue.txt', 'r')
    lines = file.readlines()
    last_line = lines[-2] 
    thp8 = float(last_line.split('\t')[0])
    c_time.append(thp8)
    
    # 0207
    dir = 'two-0207-'+str(j)
    file = open(topDir+dir+'/scheduler0-queue.txt', 'r')
    lines = file.readlines()
    last_line = lines[-2] 
    thp9 = float(last_line.split('\t')[0])
    c_time.append(thp9)
    
    comTime.append(c_time)

dataTotal = pd.DataFrame (comTime, columns = ['one', 'two', 'four',  '0116-3', '0128', '0207'])

print(dataTotal)

ct0 = [dataTotal['one']]
ct1 = [dataTotal['two']]
ct2 = [dataTotal['four']]
#ct3 = [dataTotal['0103']]
#ct4 = [dataTotal['0116-2']]
ct5 = [dataTotal['0116-3']]
ct7 = [dataTotal['0128']]
ct8 = [dataTotal['0207']]

ticks = ['one', 'two', 'four',   '0116-3', '0128', '0207']

bar_width = 0.9

boxprops = dict(linestyle='-', linewidth=5)
whiskerprops = dict(linestyle='-', linewidth=5)
capprops = dict(linestyle='-', linewidth=5)
medianprops = dict(linestyle='-', linewidth=5)

plt.figure(figsize=(10,8))
ct_plot0 = plt.boxplot(ct0,positions=np.array(np.arange(len(ct0))),widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot0['boxes']:
    box.set(hatch = '/', fill=False) 
ct_plot1 = plt.boxplot(ct1,positions=np.array(np.arange(len(ct1)))+bar_width+0.1,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot1['boxes']:
    box.set(hatch = 'x', fill=False) 
ct_plot2 = plt.boxplot(ct2,positions=np.array(np.arange(len(ct2)))+bar_width*2+0.1*2,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot2['boxes']:
    box.set(hatch = '\\', fill=False) 
# ct_plot3 = plt.boxplot(ct3,positions=np.array(np.arange(len(ct3)))+bar_width*3+0.1*3,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
# for box in ct_plot3['boxes']:
#     box.set(hatch = '\\', fill=False)
#ct_plot4 = plt.boxplot(ct4,positions=np.array(np.arange(len(ct4)))+bar_width*4+0.1*4,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
#for box in ct_plot4['boxes']:
#    box.set(hatch = '\\', fill=False) 
ct_plot5 = plt.boxplot(ct5,positions=np.array(np.arange(len(ct5)))+bar_width*3+0.1*3,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot5['boxes']:
    box.set(hatch = '\\', fill=False)

ct_plot7 = plt.boxplot(ct7,positions=np.array(np.arange(len(ct7)))+bar_width*4+0.1*4,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot7['boxes']:
    box.set(hatch = 'X', fill=False)
ct_plot8 = plt.boxplot(ct8,positions=np.array(np.arange(len(ct8)))+bar_width*5+0.1*5,widths=bar_width, patch_artist=True, boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops = medianprops)
for box in ct_plot8['boxes']:
    box.set(hatch = 'O', fill=False)
 


def define_box_properties(plot_name, color_code, label):
    for k, v in plot_name.items():
        plt.setp(plot_name.get(k), color=color_code)
         
    plt.plot([], c=color_code, label=label)
 
 
# setting colors for each groups
define_box_properties(ct_plot0, 'green', 'one')
define_box_properties(ct_plot1, 'red', 'two')
define_box_properties(ct_plot2, 'blue', 'four')
# define_box_properties(ct_plot3, 'c', '0103')
#define_box_properties(ct_plot4, 'black', '0116-2')
define_box_properties(ct_plot5, '#44dd22', '0116-3')

define_box_properties(ct_plot7, '#d233c2', '0128')
define_box_properties(ct_plot8, 'black', '0207')

ticks = ['one', 'two', 'four',   '0116-3', '0128', '0207']
plt.xticks([0,1,2, 3,4, 5], ticks)
plt.xticks(fontsize=24, fontweight='bold')
plt.yticks(fontsize=24, fontweight='bold')
plt.ylabel("Completion Time (Seconds)", fontsize=28, fontweight='bold')
plt.xlabel("Path(s)", fontsize=28, fontweight='bold')
plt.xlim(-1, len(ticks))
route = '../results-wns3/scalable_comtime_0207.png'
plt.savefig(route, format='png')
print(f"save in {route}")
plt.close()