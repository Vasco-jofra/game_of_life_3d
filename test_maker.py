#!/usr/bin/python
import sys
import random

USAGE = "./test_maker.py <cube_side_size> <initial_cell_count>"

PATH = "tests/"
PREFIX = "custom"

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print USAGE
        exit(0)

    side = int(sys.argv[1])
    initial_cell_count = int(sys.argv[2])

    if side > 10000:
        print "cube_side_size can't be larger than 10000"
        exit(0)

    if  initial_cell_count > side**3:
        print "This are probably too many cells for the cube size."
        exit(0)


    cells = []

    i = 0
    cnt = 0
    for c in random.sample(xrange(side**3), initial_cell_count):
        c_side = (c / side)
        x = (c_side / side)
        y = c_side % side
        z = c % side

        # if [x, y, z] in cells:
        #     print "FUUCK"

        cells.append([x, y, z])
        if i == 1000:
            cnt += 1
            print i*cnt
            i = 0

        i += 1

    print "HEY"
    file_name = PATH + PREFIX + "_s%de%d.in" % (side, initial_cell_count)
    with open(file_name, "w") as f:
        f.write("%d\n" % side)
        for c in cells:
            f.write("%d %d %d\n" % (c[0], c[1], c[2]))