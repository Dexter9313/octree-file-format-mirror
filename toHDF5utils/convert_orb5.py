#!/usr/bin/env python3

# Credit : Loïc Hausammann

import sys
from os import listdir
from scipy.io import FortranFile
from glob import glob
import numpy as np
from h5py import File
import matplotlib as mpl
import matplotlib.pyplot as plt


verbose = True
debug = False

max_nber_files = 100

# radius of the tokamak (center -> tore center)
tokamak_radius = 0.88
# radius of the tore (tore center -> boundary)
s_radius = 0.25

output = "orb5.hdf5"

# Get files
directory = sys.argv[-1]

files = listdir(directory)

# find all species
Nspecies = 0
species = []
for f in files:
    f = f[:-4]

    if f not in species:
        Nspecies += 1
        species.append(f)

if verbose:
    print("Found {} species: {}".format(Nspecies, species))


def toCart(s, chi, phi):
    """
    ORB5 follows magnetic coordinates.
    Here we are supposing a simpler geometry.
    """
    x = tokamak_radius + s * np.cos(chi)
    y = tokamak_radius + s * np.cos(chi)
    z = s_radius * s * np.sin(chi)
    
    return np.array([x, y, z]).transpose()
    
def readFile(f, nspecies=2, nl2w=False):
    """
    Read an ORB5 file and return the positions in cartesian
    coordinates.
    """
    f = FortranFile(f)
    a1 = f.read_record('2<i4,<f8,<i4,{}<i4,3<f8'.format(nspecies))
    nproc       = a1[0][0][0]
    npart       = a1[0][0][1]
    endtime     = a1[0][1]
    nspecies    = a1[0][2]
    iskin       = a1[0][3]

    # bookkeeping
    s_pic   = f.read_reals()
    chi_pic = f.read_reals()
    phi_pic = f.read_reals()

    return toCart(s_pic, chi_pic, phi_pic)


# Read all the positions
positions = []
for i in range(Nspecies):
    i = 0
    pos = np.array([[], [], []]).transpose()
    for f in glob(directory + "/{}*".format(species[i])):
        i += 1
        if i > max_nber_files:
            break
        if verbose:
            print("Reading file {}".format(f))
        data = readFile(f, Nspecies)

        pos = np.append(pos, data, axis=0)

    positions.append(pos)


# write the HDF5 file
f = File(output, "w")

for i in range(Nspecies):
    if verbose:
        print("Writing type {}".format(i))
    grp = f.create_group("PartType{}".format(i))
    grp.create_dataset("Coordinates", data=positions[i])

f.close()

# plot the data
if debug:
    for i in range(Nspecies):
        pos = positions[i]
        plt.figure("XY")
        plt.plot(pos[:, 0], pos[:, 1], '.')

        plt.figure("XZ")
        plt.plot(pos[:, 0], pos[:, 2], '.')

        plt.figure("YZ")
        plt.plot(pos[:, 1], pos[:, 2], '.')

    plt.show()
