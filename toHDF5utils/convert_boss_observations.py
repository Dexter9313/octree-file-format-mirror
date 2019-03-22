#!/usr/bin/env python3
"""
This script converts the data from BOSS/EBOSS into hdf5.

You need to provide a list of fits files as arguments.

>>> python3 convert_boss_observations.py data/*/*/*.fits

Credit : Loïc Hausammann

"""

from astropy.io import fits
from astropy.cosmology import Planck15
from glob import glob
import numpy as np
from os.path import dirname
from h5py import File

import sys

filenames = sys.argv[1:]
if len(filenames) < 1:
    raise Exception("You need to provide at least one file")

filenames = glob(filenames)

output = "boss.hdf5"


def toCart(z, ra, dec):
    ra *= 2 * np.pi / 360
    dec *= 2 * np.pi / 360
    dist = Planck15.comoving_distance(z)
    x = np.cos(ra) * np.cos(dec) * dist
    y = np.sin(ra) * np.cos(dec) * dist
    z = np.sin(dec) * dist

    pos = np.array([x, y, z]).transpose()
    return pos


directories = []
positions = []

for f in filenames:
    print("Reading file {}".format(f))
    data = fits.open(f)[1].data

    dir_name = dirname(f)
    if dir_name not in directories:
        directories.append(dir_name)
        positions.append([])

    z = data['z']
    ra = data["ra"]
    dec = data["dec"]

    positions[-1].append(toCart(z, ra, dec))


output = File(output, "w")

# need to create boxsize
h = output.create_group("Header")
h.attrs["BoxSize"] = 0.

for i in range(len(positions)):
    print("Writing type {}".format(i))
    pos = positions[i][0]
    for j in range(len(positions[i])):
        if j == 0:
            continue
        pos = np.append(pos, positions[i][j], axis=0)

    grp = output.create_group("PartType{}".format(i))
    grp.attrs["Directory"] = directories[i]
    grp.create_dataset("Coordinates", data=pos)
