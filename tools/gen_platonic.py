
from sys import exit, stderr
import argparse

from math import sqrt

import numpy as np
import scipy.optimize


parser = argparse.ArgumentParser(description='Generate a polygonal mesh of a solid.')

main_sel = parser.add_mutually_exclusive_group(required=True)

main_sel.add_argument('-P', '--platonic', dest='n_faces_plato', metavar='N_FACES', type=int,
							help='create a Platonic solid (4, 6, 8, 12 or 20 faces)')
main_sel.add_argument('-K', '--kepler-poinsot', dest='id_kepler', metavar='ID', choices=['gD', 'gS', 'gI', 'sgD'], type=str,
							help='create a Kepler-Poinsot solid (gD, gS, gI or sgD -- Conway\'s abbreviations)')
main_sel.add_argument('-R', '--revolution', dest='id_revolution', metavar='ID', choices=['sphere'], type=str,
							help='create a surface of revolution (sphere only for now)')

solid_size = parser.add_mutually_exclusive_group()

solid_size.add_argument('-s', '--size', dest='h_total', metavar='H', type=float, default=10.0, help='total height (default: 10)')
solid_size.add_argument('-e', '--edge', dest='l_edge', metavar='L', type=float, default=None, help='edge length')


def tetrahedron(l_edge=None, h_total=None):
	if l_edge is None:
		l_edge = 3 / sqrt(6) * h_total
	else:
		h_total = sqrt(6) / 3 * l_edge

	# https://en.wikipedia.org/wiki/Tetrahedron
	h_side = (sqrt(3) / 2) * l_edge

	# In right triangle
	y_top = sqrt(h_side ** 2 - h_total ** 2)

	vx = (
		(-l_edge / 2, 0, 0),
		(0, h_side, 0),
		(l_edge / 2, 0, 0),
		(0, y_top, h_total),
	)

	faces = (
		(2, 0, 1),
		(0, 3, 1),
		(1, 3, 2),
		(2, 3, 0),
	)

	return np.array(vx), faces


def hexahedron(l_edge=None, h_total=None):
	if l_edge is None:
		l_edge = h_total
	else:
		h_total = l_edge

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
		(0, 1, 2, 3),
		(4, 5, 1, 0),
		(5, 6, 2, 1),
		(6, 7, 3, 2),
		(7, 4, 0, 3),
		(7, 6, 5, 4),
	)

	return np.array(vx), faces


def octahedron(l_edge=None, h_total=None):
	if l_edge is None:
		l_edge = h_total / sqrt(2)
	else:
		h_total = sqrt(2) * l_edge

	# https://en.wikipedia.org/wiki/Octahedron#Cartesian_coordinates
	coord = h_total / 2

	vx = (
		(0, 0, -coord),
		(-coord, 0, 0),
		(0, coord, 0),
		(coord, 0, 0),
		(0, -coord, 0),
		(0, 0, coord),
	)

	faces = (
		(0, 1, 2),
		(0, 2, 3),
		(0, 3, 4),
		(0, 4, 1),
		(1, 5, 2),
		(2, 5, 3),
		(3, 5, 4),
		(4, 5, 1),
	)

	return np.array(vx), faces


def dodecahedron(l_edge=None, h_total=None):
	# https://en.wikipedia.org/wiki/Regular_dodecahedron#Cartesian_coordinates
	phi = (1.0 + sqrt(5)) / 2

	if l_edge is None:
		l_edge = h_total / phi**2
	else:
		h_total = phi**2 * l_edge

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

	return np.array(vx), tuple(tuple(reversed(face_vx)) for face_vx in faces)


def icosahedron(l_edge=None, h_total=None):
	phi = (1.0 + sqrt(5)) / 2

	if l_edge is None:
		l_edge = h_total / phi
	else:
		h_total = phi * l_edge

	# 1. Start with an equilatral pentagon at origin with side length `l_edge`. Its side is also a chord of the
	#    circumscribed circle, which gives us
	#      l_edge = 2 R sin(alpha / 2)
	#    and lets solve for R.
	r = l_edge / np.sin(np.pi / 5) / 2

	vx_middle = []
	for angle in [np.pi * (0.5 + 0.2 * i) for i in xrange(10)]:
		vx_middle.append(np.array([r * np.cos(angle), r * np.sin(angle), 0]))

	# 2. Given that the pyramid's side faces are equilateral, find its height
	z_top = scipy.optimize.bisect(lambda z: np.linalg.norm(vx_middle[0] - [0, 0, z]) - l_edge, 0, l_edge)
	# 3. Given that the middle "band" is formed of equilateral triangles, find its width
	z_band = scipy.optimize.bisect(lambda z: np.linalg.norm(vx_middle[0] - vx_middle[1] - [0, 0, z]) - l_edge, 0, l_edge)

	vx = np.vstack(([[0, 0, z_top]], vx_middle, [[0, 0, -(z_top + z_band)]]))
	vx[2::2] -= [0, 0, z_band]

	faces = []
	for i in xrange(2, 11, 2):
		faces.append(((i + 1) % 10, 0, i - 1))
		faces.append(((i + 1) % 10, i - 1, i))

		faces.append((i, (i + 1) % 10 + 1, (i + 1) % 10))
		faces.append((i, 11, i % 10 + 2))

	return vx, tuple(faces)


def norm(xs, ys):
	return sqrt(sum([(x - y) ** 2 for (x, y) in zip(xs, ys)]))


def edge_norms(vx, faces):
	edges = set()

	for face_vx in faces:
		for a, b in zip(face_vx, face_vx[1:] + (face_vx[0], )):
			edges.add((a, b, ) if a < b else (b, a, ))

	return np.array([norm(vx[a], vx[b]) for (a, b) in edges])


if __name__ == '__main__':
	args = parser.parse_args()

	if args.n_faces_plato:
		n_faces = args.n_faces_plato
		h_total = args.h_total
		l_edge = args.l_edge

		if l_edge is not None:
			h_total = None

		if n_faces == 4:
			title, f_create = 'Tetrahedron', tetrahedron
		elif n_faces == 6:
			title, f_create = 'Hexahedron', hexahedron
		elif n_faces == 8:
			title, f_create = 'Octahedron', octahedron
		elif n_faces == 12:
			title, f_create = 'Dodecahedron', dodecahedron
		elif n_faces == 20:
			title, f_create = 'Icosahedron', icosahedron
		else:
			stderr.write("\x1b[1;31mError\x1b[0m: no platonic solids with {} faces exist, LOL\n".format(n_faces))
			exit(1)

		stderr.write("Will print model of \x1b[1m{}\x1b[0m, ".format(title))

		# TODO: make this parameter mean max of all dimensions, not just along Z
		if h_total is None:
			stderr.write("edge length {:.2f}.\n".format(l_edge))
		else:
			stderr.write("total height {:.2f}.\n".format(h_total, l_edge))

		vx, faces = f_create(l_edge=l_edge) if l_edge is not None else f_create(h_total=h_total)

		print vx.mean(axis=0)

		print "o {}\n".format(title)

		for x, y, z in vx:
			print "v {} {} {}".format(x, y, z)
		print

		for face_vx in faces:
			print 'f', ' '.join([str(v + 1) for v in face_vx])
		print

		lengths_edges = edge_norms(vx, faces)
		stderr.write("Total number of edges: \x1b[1m{:d}\x1b[0m.\n".format(len(lengths_edges)))

		stderr.write(
			"Edge lengths: [{:e}; {:e}] with sigma = \x1b[1;{:d}m{:e}\x1b[0m.\n".format(
				lengths_edges.min(),
				lengths_edges.max(),
				32 if lengths_edges.std() < 0.0001 else 31,
				lengths_edges.std(),
			)
		)

	exit(1337)
