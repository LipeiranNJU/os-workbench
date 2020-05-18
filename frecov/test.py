import numpy as np
from os import popen
from os import system
system("make")
a = np.load('shas.npy')
shas = a.tolist()
b = np.load('filenames.npy')
filenames = b.tolist()
cmd = " ./frecov-64 ~/Downloads/M5-frecov.img"
lines = popen(cmd)
correctNumOfSha = 0
correctNumOfName = 0
for line in lines:
    shaToBeTested, filenameTobeTested = line.split()
    if shaToBeTested in shas:
        correctNumOfSha += 1
    if filenameTobeTested in filenames:
        correctNumOfName += 1
    
print("文件名正确率{:.3f}%\t校验码正确率{:3f}%".format(correctNumOfName/len(filenames)*100, correctNumOfSha/len(shas)*100))