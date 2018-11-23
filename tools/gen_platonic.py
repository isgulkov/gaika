
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
		(2, 0, 3)
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

def print_obj(name, vx, faces, l_edge=None):
	print "o {}\n".format(name)

	for x, y, z in vx:
		print "v {} {} {}".format(x, y, z)
	print

	for face_vx in faces:
		print 'f', ' '.join([str(v + 1) for v in face_vx])
	print

	if l_edge is not None:
		total_err = sum([abs(l - l_edge) for l in edge_norms(vx, faces)])

		stderr.write("Total error in edge lengths: \x1b[1m{:f}\x1b[0m.\n".format(total_err))

if __name__ == '__main__':
	args = parser.parse_args()

	n_faces = args.n_faces
	l_edge = args.l_edge

	if n_faces not in (4, 6, 8, 12, 20):
		stderr.write("\x1b[1mError\x1b[0m: there are no platonic solids with {} faces\n".format(n_faces))
		exit(0)

	if n_faces == 4:
		vx, faces = tetrahedron(l_edge)

		print_obj('Tetrahedron', vx, faces, l_edge=l_edge)
	else:
		print n_faces, l_edge, "!!!"
