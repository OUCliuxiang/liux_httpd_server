import socket
import struct
import time

def send_data(client, data):
    cmd = 1
    data_len = len(data)
    data = struct.pack(f'@{data_len}s', data.encode('utf-8'))
    # data = struct.pack(f"8sII{data_len}s", data)
    # data = bytes(data, encoding="utf-8")
    client.send(data)  # 发送TCP数据
    info = client.recv(1024).decode()
    print(info)


if __name__ == '__main__':

    host = '127.0.0.1'
    port = 8899

    client = socket.socket()  # 创建TCP/IP套接字
    client.connect((host, port))  # 主动初始化TCP服务器连接
    send_data(client, 'hello python')
    send_data(client, 'so far so good!')
    time.sleep(1)
    client.close()
