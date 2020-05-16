from os import walk
from os.path import join
from os import popen
import numpy as np
items = walk("/mnt/DCIM")

shas = []
filenames = []
for root, dirs, files in items:
    for File in files:
        result = popen("sha1sum " + join(root, File))
        for line in result.readlines():
            sha, filename = line.split()
            shas.append(sha)
            filenames.append(filename.split("/")[-1])

m=np.array(shas)
np.save('shas.npy',m)
n=np.array(filenames)
np.save('filenames.npy',n)