import pandas as pd
import matplotlib.pyplot as plt
import argparse

def plot_ue_positions(file_path):
    """
    繪製 UE 位置的散點圖，文件中的第二和第三列應為 x 和 y 座標。

    :param file_path: 包含 UE 位置數據的文件路徑
    """
    # 讀取數據，只選取第二和第三列作為 x 和 y 座標
    df = pd.read_csv(file_path, delim_whitespace=True, header=None, usecols=[0, 1], names=['x', 'y'])

    # 繪製散點圖
    plt.figure(figsize=(10, 8))
    plt.scatter(df['x'], df['y'], c='blue', marker='o', label='UE Position')
    plt.title("UE Positions Plot")
    plt.xlabel("X Coordinate")
    plt.ylabel("Y Coordinate")
    plt.grid(True)
    plt.legend()
    plt.show()

# 主程序入口
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot UE positions from a text file.")
    parser.add_argument("file_path", type=str, help="Path to the file containing UE positions")
    args = parser.parse_args()
    
    # 繪製 UE 位置
    plot_ue_positions(args.file_path)