import vtk
from vtk.util.numpy_support import vtk_to_numpy
import matplotlib.pyplot as pl
import numpy as np
import os
import tempfile

def get_geometry(problem):    
    temp_geometry_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    problem.geometry().export_vtk(temp_geometry_name)
    
    reader_poly = vtk.vtkPolyDataReader()
    reader_poly.SetFileName(temp_geometry_name)        
    reader_poly.Update()
    geometry = reader_poly.GetOutput()

    # points
    points = geometry.GetPoints()
    npts = geometry.GetNumberOfPoints()
    pts = vtk_to_numpy(points.GetData())
    # print(pts)

    # lines
    lines = vtk_to_numpy(geometry.GetLines().GetData())
    nln = lines.size//3  # number of lines

    lns = np.take(lines, [n for n in range(lines.size) if n%3 != 0]).reshape(nln, 2)

    os.remove(temp_geometry_name)
    
    return [pts, lns]

def view_geometry(problem):
    [pts, lns] = get_geometry(problem)
    
    fig = pl.figure(figsize=(5, 5))

    for l in lns:
        pl.plot([pts[l[0]][0], pts[l[1]][0]], [pts[l[0]][1], pts[l[1]][1]], 'b')    
        
    ax = fig.axes[0]
    ax.set_aspect('equal')
    
def view_scalar(problem, computation, field, time_step, adaptive_step, variable, component, scalars=True, contours=False, mesh=False):    
    temp_post_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    solution = computation.solution(field)
    solution.export_vtk(temp_post_name, time_step, adaptive_step, variable, component)
    
    reader_grid = vtk.vtkUnstructuredGridReader()
    reader_grid.SetFileName(temp_post_name)        
    reader_grid.Update()
    post = reader_grid.GetOutput()

    # points
    points = post.GetPoints()
    npts = post.GetNumberOfPoints()
    pts = vtk_to_numpy(points.GetData())
    # print(pts)

    # triangles
    quads = vtk_to_numpy(post.GetCells().GetData())    
    nqd = quads.size//5  # number of triangles
    # print(nqd)

    quads = np.take(quads, [n for n in range(quads.size) if n%5 != 0]).reshape(nqd, 4)
    # print(quads)
        
    u = vtk_to_numpy(post.GetPointData().GetArray(0))
    
    os.remove(temp_post_name)
    
    fig = pl.figure(figsize=(5, 5))

    tri = []
    for q in quads:
        tri.append([q[0], q[2], q[1]])
        tri.append([q[0], q[2], q[3]])
        
    if scalars:
        tcf = pl.tricontourf(pts[:,0], pts[:,1], tri, u, 20)
    
    if contours:
        ax.tricontour(pts[:,0], pts[:,1], tri, u, colors='k')
    
    if mesh:
        pl.triplot(pts[:,0], pts[:,1], tri, linewidth=0.5, color='k')

    # geometry
    [geometry_pts, geometry_lns] = get_geometry(problem)
    for l in geometry_lns:
        pl.plot([geometry_pts[l[0]][0], geometry_pts[l[1]][0]], [geometry_pts[l[0]][1], geometry_pts[l[1]][1]], 'b')
        
    ax = fig.axes[0]
    ax.set_aspect('equal')

    fig.colorbar(tcf)    
