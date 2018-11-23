
from sys import exit, stderr
import argparse

from math import sqrt


parser = argparse.ArgumentParser(description='Generate a polygonal mesh of a platonic solid.')
parser.add_argument('n_faces', metavar='N', type=int, help='number of faces (4, 6, 8, 12 or 20)')
parser.add_argument('-e', '--edge', dest='l_edge', metavar='L', type=float, default=1.0, help='edge length (default: 1)')


def tetrahedron(l_edge):
	# https://en.wikipedia.org/wiki/Tetrahedron
	h_side = (sqrt(3) / 2) * l_edge
	h = sqrt(6) / 3 * l_edge

	# In rright triangle
	y_top = sqrt(h_side ** 2 - h ** 2)

	vx = (
		(-l_edge / 2, 0, 0),
		(0, h_side, 0),
		(l_edge / 2, 0, 0),
		(0, y_top, h),
	)

	faces = (
		(2, 1, 0),
		(0, 1, 3),
		(1, 2, 3),
		(2, 0, 3),
	)

	return vx, faces


def hexahedron(l_edge):
	vx = (
		(-l_edge / 2, -l_edge / 2, 0),
		(-l_edge / 2, l_edge / 2, 0),
		(l_edge / 2, l_edge / 2, 0),
		(l_edge / 2, -l_edge / 2, 0),
		(-l_edge / 2, -l_edge / 2, l_edge),
		(-l_edge / 2, l_edge / 2, l_edge),
		(l_edge / 2, l_edge / 2, l_edge),
		(l_edge / 2, -l_edge / 2, l_edge),
	)

	faces = (
		(3, 2, 1, 0),
		(0, 1, 5, 4),
		(1, 2, 6, 5),
		(2, 3, 7, 6),
		(3, 0, 4, 7),
		(4, 5, 6, 7)
	)

	return vx, faces


def octahedron(l_edge):
	# https://en.wikipedia.org/wiki/Octahedron#Cartesian_coordinates
	coord = l_edge / sqrt(2)

	vx = (
		(0, 0, -coord),
		(-coord, 0, 0),
		(0, coord, 0),
		(coord, 0, 0),
		(0, -coord, 0),
		(0, 0, coord),
	)

	faces = (
		(0, 2, 1),
		(0, 3, 2),
		(0, 4, 3),
		(0, 1, 4),
		(1, 2, 5),
		(2, 3, 5),
		(3, 4, 5),
		(4, 1, 5),
	)

	return vx, faces


def dodecahedron(l_edge):
	# https://en.wikipedia.org/wiki/Regular_dodecahedron#Cartesian_coordinates
	phi = (1.0 + sqrt(5)) / 2

	big = phi * l_edge / (2.0 / phi)
	med = l_edge / (2.0 / phi)
	small = l_edge / phi / (2.0 / phi)

	vx = (
		(-small, 0, -big),
		(small, 0, -big),
		(-med, -med, -med),
		(-med, med, -med),
		(med, med, -med),
		(med, -med, -med),
		(0, -big, -small),
		(0, big, -small),
		(-big, -small, 0),
		(-big, small, 0),
		(big, small, 0),
		(big, -small, 0),
		(0, -big, small),
		(0, big, small),
		(-med, -med, med),
		(-med, med, med),
		(med, med, med),
		(med, -med, med),
		(-small, 0, big),
		(small, 0, big),
	)

	faces = (
		(1, 0, 2, 6, 5),
		(0, 1, 4, 7, 3),
		(0, 3, 9, 8, 2),
		(1, 5, 11, 10, 4),
		(2, 8, 14, 12, 6),
		(3, 7, 13, 15, 9),
		(4, 10, 16, 13, 7),
		(5, 6, 12, 17, 11),
		(8, 9, 15, 18, 14),
		(10, 11, 17, 19, 16),
		(12, 14, 18, 19, 17),
		(13, 16, 19, 18, 15),
	)

	return vx, faces


def icosahedron(l_edge):
	# https://en.wikipedia.org/wiki/Regular_icosahedron#Cartesian_coordinates
	phi = (1.0 + sqrt(5)) / 2

	big = phi * l_edge / 2
	reg = l_edge / 2.0

	vx = (
		(0, -reg, -big),
		(0, reg, -big),
		(-big, 0, -reg),
		(big, 0, -reg),
		(-reg, -big, 0),
		(-reg, big, 0),
		(reg, big, 0),
		(reg, -big, 0),
		(-big, 0, reg),
		(big, 0, reg),
		(0, -reg, big),
		(0, reg, big),
	)

	faces = (
		(1, 0, 3),
		(0, 1, 2),
		(0, 4, 7),
		(1, 6, 5),
		(0, 2, 4),
		(1, 5, 2),
		(1, 3, 6),
		(0, 7, 3),
		(2, 8, 4),
		(2, 5, 8),
		(3, 9, 6),
		(3, 7, 9),
		(4, 10, 7),
		(5, 6, 11),
		(7, 10, 9),
		(4, 8, 10),
		(5, 11, 8),
		(6, 9, 11),
		(9, 10, 11),
		(8, 11, 10),
	)

	return vx, faces


def norm(xs, ys):
	return sqrt(sum([(x - y) ** 2 for (x, y) in zip(xs, ys)]))


def edge_norms(vx, faces):
	edges = set()

	for face_vx in faces:
		for a, b in zip(face_vx, face_vx[1:] + (face_vx[0], )):
			edges.add((a, b, ) if a < b else (b, a, ))

	return [norm(vx[a], vx[b]) for (a, b) in edges]


def stddev(l_expected, ls):
	return sqrt(sum((l - l_expected) ** 2 for l in ls))


def print_obj(name, l_edge, vx, faces):
	print "o {}\n".format(name)

	for x, y, z in vx:
		print "v {} {} {}".format(x, y, z)
	print

	for face_vx in faces:
		print 'f', ' '.join([str(v + 1) for v in face_vx])
	print

	lengths_edges = edge_norms(vx, faces)
	stderr.write("Total number of edges: \x1b[1m{:d}\x1b[0m.\n".format(len(lengths_edges)))

	total_err = sum([abs(l - l_edge) for l in lengths_edges])
	stderr.write("Total error in edge lengths: \x1b[1m{:f}\x1b[0m.\n".format(total_err))

if __name__ == '__main__':
	args = parser.parse_args()

	n_faces = args.n_faces
	l_edge = args.l_edge

	if n_faces == 4:
		print_obj('Tetrahedron', l_edge, *tetrahedron(l_edge))
	elif n_faces == 6:
		print_obj('Hexahedron', l_edge, *hexahedron(l_edge))
	elif n_faces == 8:
		print_obj('Octahedron', l_edge, *octahedron(l_edge))
	elif n_faces == 12:
		print_obj('Dodecahedron', l_edge, *dodecahedron(l_edge))
	elif n_faces == 20:
		print_obj('Icosahedron', l_edge, *icosahedron(l_edge))
	else:
		stderr.write("\x1b[1;31mError\x1b[0m: no platonic solids with {} faces exist, LOL\n".format(n_faces))
		exit(1)
