import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
import pandas as pd

plt.rcParams["font.family"] = "serif"
plt.rcParams["font.serif"] = "Times New Roman"

topDir = '../results-wns3/'

def help(i):
    file = open(topDir+'schedulerU-ltenr-15'+'/scheduler'+str(i)+'-queue.txt', 'r')
    finish_time = float(file.readlines()[-1].split('\t')[0])
    file = open(topDir+'schedulerU-ltenr-15'+'/scheduler'+str(i)+'-rx-ltenr.txt', 'r')
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
    return dataTotal

def generate_image(value):
    """根據滑軸的數值生成對應的吞吐量圖像"""
    rr = help(0)  
    minrtt = help(1)
    blest = help(2)  
    ecf = help(3)
    peek = help(4)
    
    fig, ax = plt.subplots(figsize=(8, 6))
    plt.grid(linestyle="--")
    
    plt.plot(rr['Time'], rr['goodput'], color="green", label="RR", linewidth=4)
    plt.plot(minrtt['Time'], minrtt['goodput'], color="red", label="MRTT", linewidth=4)
    plt.plot(blest['Time'], blest['goodput'], color="blue", label="BLEST", linewidth=4)
    plt.plot(ecf['Time'], ecf['goodput'], color="c", label="ECF", linewidth=4)
    plt.plot(peek['Time'], peek['goodput'], color="orange", label="Peekaboo", linewidth=4)
    
    plt.xticks(fontsize=20, fontweight='bold')
    plt.yticks(fontsize=20, fontweight='bold')
    plt.ylabel("Instantaneous Throughput (Mbps)", fontsize=20, fontweight='bold')
    plt.xlabel("Time (s)", fontsize=20, fontweight='bold')
    plt.legend(loc=0, numpoints=1)
    
    return fig

def update_image(event):
    """更新圖片"""
    value = slider.get()
    fig = generate_image(value)
    
    # 清除舊的圖片並繪製新的
    for widget in frame.winfo_children():
        widget.destroy()
    
    canvas = FigureCanvasTkAgg(fig, master=frame)
    canvas.draw()
    canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

# 創建主窗口
root = tk.Tk()
root.title("Slider Image Generator")
root.geometry("800x600")

# 創建滑軸
slider = ttk.Scale(root, from_=1, to=50, orient='horizontal', command=update_image)
slider.pack(fill=tk.X, padx=20, pady=10)

# 顯示圖片的框架
frame = tk.Frame(root)
frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)

# 初始化顯示第一張圖
update_image(None)

# 運行應用
root.mainloop()
