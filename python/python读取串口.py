import datetime
import matplotlib.pyplot as plt
import serial
import struct
import binascii

now = datetime.datetime.now()
end1 = datetime.datetime.now()+datetime.timedelta(seconds=4)
end2 = datetime.datetime.now()+datetime.timedelta(seconds=6)
x=[]
y=[]
z=[]
data = [0,0,0]
num = [31,133,135,64,12]
def packData(data):
    num.pop(4)
    data.append(struct.unpack('<f', struct.pack('4B', *num))[0])
    num.append(0)
t = serial.Serial('com3',9600)

i=0
j=0
k=0
while(1):
    str = t.read()
    a = binascii.b2a_hex(str).decode()
    num[i] = eval('0x'+a)
    if(i==0 and num[0]!=0x51 and num[0]!=0x52 and num[0]!=0x53):
        pass
    else:
        i+=1
        if(i==5):
            i=0
            j+=1
            num.reverse()
            if(num[4]==0x51):
                packData(x)
            elif(num[4]==0x52):
                packData(y)
            else:
                packData(z)
            if(end1<=datetime.datetime.now()<=end2):
                break

# 打开画图窗口1，在三维空间中绘图
fig = plt.figure(1)
ax = fig.gca(projection='3d')
# 给出点（0，0，0）和（100，200，300）
# 将数组中的前两个点进行连线
figure = ax.plot(x, y, z, c='r')
plt.show()