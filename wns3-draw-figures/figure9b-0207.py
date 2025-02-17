## figure 9b stable instantaneous throughput
import pandas as pd
import matplotlib.pyplot as plt
plt.rcParams["font.family"] = "serif"
plt.rcParams["font.serif"] = "Times New Roman"

topDir = '../results-wns3/'

def help(i):
    file = open(topDir+'scheduler-0207-35'+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-4].split('\t')[0])
    file = open(topDir+'scheduler-0207-35'+'/scheduler'+str(i)+'-rx-0207.txt', 'r')
    lines = file.readlines()
    goodput = []
    c_time = []
    for line in lines:
        temp = line.split('\t')

        if (temp[0] == '5' and c_time == []):
            c_time.append(float(temp[1]))
            c_time.append(float(temp[5]))
        if (temp[0] == '7' and len(c_time) == 2):
            c_time[1] = c_time[1] + float(temp[5])
            c_time.append(i)
            goodput.append(c_time)
            if (c_time[0] > finish_time):
                break
            c_time = []
    dataTotal = pd.DataFrame (goodput, columns = ['Time', 'goodput', 'scheduler'])
    print(dataTotal)
    return dataTotal


rr = help(0)  
minrtt = help(1)
blest = help(2)  
ecf = help(3)
peek = help(4)

plt.figure(figsize=(8, 6))
plt.grid(linestyle="--")
ax = plt.gca()

plt.plot(rr['Time'], rr['goodput'], color="green", label="RR", linewidth=2)
plt.plot(minrtt['Time'], minrtt['goodput'], color="red", label="MRTT", linewidth=5)
plt.plot(blest['Time'], blest['goodput'], color="blue", label="BLEST", linewidth=2)
plt.plot(ecf['Time'], ecf['goodput'], color="c", label="ECF", linewidth=2)
plt.plot(peek['Time'], peek['goodput'], color="orange", label="Peekaboo", linewidth=2)

plt.xticks(fontsize=20, fontweight='bold')
plt.yticks(fontsize=20, fontweight='bold')
plt.ylabel("Instantaneous Throughput (Mbps)", fontsize=20, fontweight='bold')
plt.xlabel("Time (s)", fontsize=20, fontweight='bold')


plt.legend(loc=0, numpoints=1)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=20, fontweight='bold') 
route = '../results-wns3/ins_throughput_scheduler-0207.png'
plt.savefig(route, format='png') 
print(f'save in {route}')
plt.close()
