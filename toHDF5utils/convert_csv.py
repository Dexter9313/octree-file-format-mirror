#!/usr/bin/env python3

import pandas as pd
import sys
import numpy as np
from h5py import File
import matplotlib.pyplot as plt


debug = False

if len(sys.argv) < 4:
    print("Usage :\n\tconvert_csv.py INPUT_CSV OUTPUT_HDF5 COLUMNS")
    print("\n\tCOLUMNS = x,y,z[,radius[,luminosities]] where x, y, z, radius and luminosities are column numbers, starting at 0.")
    print("\tradius can be the empty string if you only want luminosities (0,1,2,,3 for example).")
    quit()

csv_input = sys.argv[1]
hdf5_output = sys.argv[2]
columns = sys.argv[3].split(',')

for i in range(len(columns)):
    columns[i] = int(columns[i])

if len(columns) < 3:
    print("Error : not enough elements for COLUMNS (x, y and z are needed).")
    quit()

print("Input : " + csv_input)
print("Output : " + hdf5_output)

def extract_data(l):
    line = l.split('#')[0].split(',')
    # if line is too short, return nothing (probably a comment line or whatever)
    if len(line) <= columns[0] or len(line) <= columns[1] or len(line) <= columns[2]:
        return np.array([[], [], []]).transpose()
    result = [line[columns[0]], line[columns[1]], line[columns[2]]]
    if len(columns) >= 4 and columns[3] != "":
        result.append(line[columns[3]])
    if len(columns) >= 5 and columns[4] != "":
        result.append(line[columns[4]])
    return np.array([result])


print("Loading CSV...")
df = pd.read_csv(csv_input)
print("Finished loading...")

x=df.columns[columns[0]]
y=df.columns[columns[1]]
z=df.columns[columns[2]]

pos = np.array([df[x], df[y], df[z]]).transpose()
print("Read coordinates : ")
print(pos.shape)

# write the HDF5 file
f = File(hdf5_output, "w")

grp = f.create_group("PartType0")
grp.create_dataset("Coordinates", data=pos)
if len(columns) >= 4 and columns[3] != "":
    rad=df.columns[columns[3]]
    radius = np.array([df[rad]]).transpose()
    print("Read radius : ")
    print(radius.shape)
    grp.create_dataset("Radius", data=radius)
if len(columns) >= 5 and columns[4] != "":
    lum=df.columns[columns[4]]
    luminosity = np.array([df[lum]]).transpose()
    print("Read luminosity : ")
    print(luminosity.shape)
    grp.create_dataset("Luminosity", data=luminosity)
f.close()

# plot the data
if debug:
    plt.figure("XY")
    plt.plot(pos[:, 0], pos[:, 1], '.')

    plt.figure("XZ")
    plt.plot(pos[:, 0], pos[:, 2], '.')

    plt.figure("YZ")
    plt.plot(pos[:, 1], pos[:, 2], '.')

    plt.show()
