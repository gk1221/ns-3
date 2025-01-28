## figure 7b instantaneous throughput

import pandas as pd
import matplotlib.pyplot as plt
plt.rcParams["font.family"] = "serif"
plt.rcParams["font.serif"] = "Times New Roman"

schedulerTypes = [0]

topDir = '../results-wns3/'

def helpone(num,i):
    dir = topDir+'one-'+str(num)
    file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(dir+'/scheduler'+str(i)+'-rx.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')
        if (temp[0] == '1' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])        
    return dataTotal

def helptwo(num,i):
    dir = topDir+'two-'+str(num)
    file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(dir+'/scheduler'+str(i)+'-rx.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')
        if (temp[0] == '1' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
        if (temp[0] == '3' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])        
    return dataTotal

def help0103(num,i):
    dir = topDir+'two-0103-'+str(num)
    file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(dir+'/scheduler'+str(i)+'-rx-0103.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')
        if (temp[0] == '1' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
        if (temp[0] == '3' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])        
    return dataTotal

def help0116_2(num,i):
    dir = topDir+'two-0116-2-'+str(num)
    file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(dir+'/scheduler'+str(i)+'-rx-0116-2.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')
        if (temp[0] == '1' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
        if (temp[0] == '3' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])        
    print(dataTotal)
    return dataTotal

def help0116_3(num,i):
    dir = topDir+'two-0116-3-'+str(num)
    file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(dir+'/scheduler'+str(i)+'-rx-0116-3.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')
        if (temp[0] == '1' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
        if (temp[0] == '3' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])        
    return dataTotal

def help0126(num,i):
    dir = topDir+'two-0126-'+str(num)
    file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(dir+'/scheduler'+str(i)+'-rx-0126.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')
        if (temp[0] == '1' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
        if (temp[0] == '3' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])        
    print(dataTotal)
    return dataTotal

def helpfour(num,i):
    dir = topDir+'four-'+str(num)
    file = open(dir+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(dir+'/scheduler'+str(i)+'-rx.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')
        if (temp[0] == '1' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
        if (temp[0] == '3' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
        if (temp[0] == '4' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
        if (temp[0] == '5' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])        
    return dataTotal



## take seed=39 as the example figure
j = 39
one = helpone(j,0)
two = helptwo(j,0)
two_0103 = help0103(j,0)
two_0116_3 = help0116_3(j,0)
two_0126 = help0126(j,0)
four = helpfour(j,0)  

plt.figure(figsize=(10, 8))
plt.grid(linestyle="--")  
ax = plt.gca()

plt.plot(one['Time'], one['goodput'], color="green", label="one path", linewidth=4)
plt.plot(two['Time'], two['goodput'], color="red", label="two paths", linewidth=4)
plt.plot(two_0103['Time'], two_0103['goodput'], color="c", label="two_0103 paths", linewidth=4)
plt.plot(two_0116_3['Time'], two_0116_3['goodput'], color="#44dd22", label="two_0116_3 paths", linewidth=4)
plt.plot(two_0126['Time'], two_0126['goodput'], color="black", label="two_0126 paths", linewidth=4)
plt.plot(four['Time'], four['goodput'], color="blue", label="four paths", linewidth=4)

plt.xticks(fontsize=24, fontweight='bold')  
plt.yticks(fontsize=24, fontweight='bold')

plt.ylabel("Instantaneous Throughput (Mbps)", fontsize=26, fontweight='bold')
plt.xlabel("Time (s)", fontsize=26, fontweight='bold')

plt.legend(loc=4, numpoints=2)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=16, fontweight='bold')
route = '../results-wns3/ins_throughput_scalable-0126.png'
plt.savefig(route, format='png')
print(f"save in {route}")
plt.close()
