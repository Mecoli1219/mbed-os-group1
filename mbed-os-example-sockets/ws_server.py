import socket
import json
import numpy as np
import matplotlib.pyplot as plt

HOST = '192.168.50.171'  # TODO: Update IP Address
PORT = 6531  # 监听的端口（使用端口> 1023）

plt.ion()  # 启用交互式模式
graph = []

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print("Starting server at: ", (HOST, PORT))
    conn, addr = s.accept()
    with conn:
        print("Connected at", addr)
        t = 0
        while True:
            data = conn.recv(1024)
            data = data.decode('utf-8')
            print("Received from socket server:", data)
            obj = json.loads(data)
            t = obj['sample_num']
            x = obj['x']
            y = obj['y']
            z = obj['z']
            graph.append([t, x, y, z])
            
            plt.clf()
            graph_np = np.array(graph)
            plt.plot(graph_np[:, 0], graph_np[:, 1], label='x')
            plt.plot(graph_np[:, 0], graph_np[:, 2], label='y')
            plt.plot(graph_np[:, 0], graph_np[:, 3], label='z')
            plt.xlabel("Sample num")
            plt.ylabel("Value")
            plt.legend()
            plt.pause(0.0001)
