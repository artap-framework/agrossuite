import matplotlib.pyplot as pl
import numpy as np
import os
import tempfile


class GeometryView():
    def __init__(self):
        self.debug = False

        self.points = []
        self.edges = []

    def read_from_vtk(self, fn):
        mode = ""
        idx = -1

        npoints = -1
        nedges = -1

        with open(fn, 'r') as f:
            for line in f:
                # print(line)

                # read points
                if mode == "points":
                    if idx < npoints:
                        spl = line.split()
                        self.points.append([float(spl[0]), float(spl[1])])
                        if idx == npoints:
                            mode = ""
                        idx += 1

                # read lines
                if mode == "lines":
                    if idx < nedges:
                        spl = line.split()
                        self.edges.append([int(spl[1]), int(spl[2])])
                        if idx == nedges:
                            mode = ""
                        idx += 1

                # number of points
                if line.startswith('POINTS'):
                    npoints = int(line.split()[1])
                    # reset counter
                    idx = 0
                    # set mode
                    mode = "points"

                # number of line segments
                if line.startswith('LINES'):
                    nedges = int(line.split()[1])
                    # reset counter
                    idx = 0
                    # set mode
                    mode = "lines"

        if self.debug:
            print("Number of points: ", len(self.points))
            print("Points: ", self.points)
            print("Number of lines: ", len(self.edges))
            print("Lines: ", self.edges)

    def plot_geometry(self, fig):
        for l in self.edges:
            pl.plot([self.points[l[0]][0], self.points[l[1]][0]], [self.points[l[0]][1], self.points[l[1]][1]], 'b')

class ScalarView():
    def __init__(self):
        self.debug = False

        self.points = []
        self.tris = []
        self.mesh = []
        self.scalar = []

    def read_from_vtk(self, fn):
        mode = ""
        idx = -1

        npoints = -1
        ncells = -1

        points_x = []
        points_y = []

        with open(fn, 'r') as f:
            for line in f:
                # print(line)

                # read points
                if mode == "points":
                    if idx < npoints:
                        spl = line.split()
                        points_x.append(float(spl[0]))
                        points_y.append(float(spl[1]))
                        if idx == npoints:
                            mode = ""
                        idx += 1

                # read cells
                if mode == "cells":
                    if idx < ncells:
                        spl = line.split()
                        self.mesh.append([int(spl[1]), int(spl[2]), int(spl[3]), int(spl[4])])
                        self.tris.append([int(spl[1]), int(spl[3]), int(spl[2])])
                        self.tris.append([int(spl[1]), int(spl[3]), int(spl[4])])

                        if idx == ncells:
                            mode = ""
                        idx += 1

                # read scalars
                if mode == "scalar":
                    spl = line.split()
                    self.scalar = list(map(float, spl))

                # number of points
                if line.startswith('POINTS'):
                    npoints = int(line.split()[1])
                    # reset counter
                    idx = 0
                    # set mode
                    mode = "points"

                # number of line segments
                if line.startswith('CELLS'):
                    ncells = int(line.split()[1])
                    # reset counter
                    idx = 0
                    # set mode
                    mode = "cells"

                # lookup table
                if line.startswith('LOOKUP_TABLE'):
                    # set mode
                    mode = "scalar"

        self.points = [points_x, points_y]

        if self.debug:
            print("Number of points: ", len(self.points))
            print("Points: ", self.points)
            print("Number of triangles: ", len(self.tris))
            print("Triangles: ", self.tris)
            print("Number of scalar: ", len(self.scalar))
            print("Scalar: ", self.scalar)

    def plot_scalar(self, fig, count=20):
        tcf = pl.tricontourf(self.points[0], self.points[1], self.tris, self.scalar, count)
        fig.colorbar(tcf)

    def plot_contours(self, fig, count=20, linewidths=0.5, figsize=(5, 5)):
        pl.tricontour(self.points[0], self.points[1], self.tris, self.scalar, count, linewidths=linewidths, colors='g')

    def plot_mesh(self, fig, linewidth=0.5):
        # pl.triplot(self.points[0], self.points[1], self.tris, linewidth=0.5, color='k')
        # TODO: fix - very slow
        for m in self.mesh:
            pl.plot([self.points[0][m[0]], self.points[0][m[1]], self.points[0][m[2]], self.points[0][m[3]]], [self.points[1][m[0]], self.points[1][m[1]], self.points[1][m[2]], self.points[1][m[3]]], 'k', linewidth=linewidth)
            # pl.plot([self.points[m[1]], self.points[m[1]], [self.points[m[1]], self.points[m[2]], 'b')


def view_geometry(problem):
    temp_geometry_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    problem.geometry().export_vtk(temp_geometry_name)

    fig = pl.figure(figsize=(5, 5))

    geom = GeometryView()
    geom.debug = False
    geom.read_from_vtk(temp_geometry_name)
    geom.plot_geometry(fig)

    os.remove(temp_geometry_name)

def read_data(problem, computation, field, time_step, adaptive_step, variable, component):
    # geometry
    temp_geometry_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    problem.geometry().export_vtk(temp_geometry_name)

    geom = GeometryView()
    geom.debug = False
    geom.read_from_vtk(temp_geometry_name)
    os.remove(temp_geometry_name)

    # solution
    temp_post_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    solution = computation.solution(field)
    solution.export_vtk(temp_post_name, time_step, adaptive_step, variable, component)

    scalar = ScalarView()
    scalar.debug = False
    scalar.read_from_vtk(temp_post_name)
    os.remove(temp_post_name)

    return [geom, scalar]

def view_scalar(problem, computation, field, time_step, adaptive_step, variable, component, scalars=True, contours=False, mesh=False):
    [geom, scalar] = read_data(problem, computation, field, time_step, adaptive_step, variable, component)

    fig = pl.figure(figsize=(5, 5))

    if scalar:
        scalar.plot_scalar(fig)
    
    if contours:
        scalar.plot_contours(fig)
    
    if mesh:
        scalar.plot_mesh(fig)

    # geometry

    geom.plot_geometry(fig)

    ax = fig.axes[0]
    ax.set_aspect('equal')