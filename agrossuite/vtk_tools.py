import vtk
import math
import os
import tempfile

def geometry_actor(filename):
    reader_poly = vtk.vtkPolyDataReader()
    reader_poly.SetFileName(filename)        
    geometry = reader_poly.GetOutputPort()

    # create the mapper that corresponds the objects of the vtk file into graphics elements
    geometry_mapper = vtk.vtkDataSetMapper()
    geometry_mapper.SetInputConnection(geometry)
    
    # actor
    geometry_actor = vtk.vtkActor()
    geometry_actor.SetMapper(geometry_mapper)
    geometry_actor.GetProperty().SetColor(0.8, 0.2, 0)
    geometry_actor.GetProperty().SetLineWidth(1.8)
    
    return geometry_actor

def contours_actor(filename, count = 10, color = False):
    # read the source file.
    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update()
    output = reader.GetOutput()
    scalar_range = reader.GetOutput().GetScalarRange()

     # contours
    contours = vtk.vtkContourFilter()
    contours.SetInputConnection(reader.GetOutputPort()) 
    contours.GenerateValues(count, scalar_range) 

    # map the contours to graphical primitives 
    contMapper = vtk.vtkPolyDataMapper() 
    contMapper.SetInputConnection(contours.GetOutputPort()) 
    contMapper.SetScalarVisibility(color) # colored contours
    contMapper.SetScalarRange(scalar_range)
    
    # create an actor for the contours 
    contActor = vtk.vtkActor() 
    contActor.SetMapper(contMapper) 
    contActor.GetProperty().SetColor(0.3, 0.3, 0.3)
    contActor.GetProperty().SetLineWidth(1)
    
    return contActor
     
def scalar_actor(filename):
    colobar_agros =  [[ 0.200910, 0.167447, 0.531791 ],
                        [ 0.214038, 0.171125, 0.538240 ],
                        [ 0.217341, 0.176630, 0.549972 ],
                        [ 0.215713, 0.182989, 0.563804 ],
                        [ 0.212312, 0.189597, 0.577930 ],
                        [ 0.209018, 0.196117, 0.591469 ],
                        [ 0.206788, 0.202393, 0.604134 ],
                        [ 0.205938, 0.208392, 0.615985 ],
                        [ 0.206364, 0.214155, 0.627265 ],
                        [ 0.207711, 0.219763, 0.638280 ],
                        [ 0.209497, 0.225316, 0.649328 ],
                        [ 0.211200, 0.230912, 0.660652 ],
                        [ 0.212328, 0.236640, 0.672421 ],
                        [ 0.212452, 0.242572, 0.684724 ],
                        [ 0.211237, 0.248759, 0.697571 ],
                        [ 0.208447, 0.255234, 0.710907 ],
                        [ 0.203952, 0.262008, 0.724625 ],
                        [ 0.197718, 0.269076, 0.738576 ],
                        [ 0.189798, 0.276418, 0.752590 ],
                        [ 0.180323, 0.284001, 0.766486 ],
                        [ 0.169480, 0.291784, 0.780083 ],
                        [ 0.157506, 0.299718, 0.793209 ],
                        [ 0.144666, 0.307753, 0.805711 ],
                        [ 0.131243, 0.315834, 0.817457 ],
                        [ 0.117526, 0.323911, 0.828340 ],
                        [ 0.103796, 0.331931, 0.838281 ],
                        [ 0.090323, 0.339850, 0.847225 ],
                        [ 0.077353, 0.347623, 0.855144 ],
                        [ 0.065106, 0.355214, 0.862033 ],
                        [ 0.053768, 0.362591, 0.867908 ],
                        [ 0.043494, 0.369729, 0.872801 ],
                        [ 0.034405, 0.376608, 0.876761 ],
                        [ 0.026583, 0.383214, 0.879844 ],
                        [ 0.020082, 0.389538, 0.882119 ],
                        [ 0.014919, 0.395577, 0.883657 ],
                        [ 0.011085, 0.401332, 0.884531 ],
                        [ 0.008543, 0.406809, 0.884818 ],
                        [ 0.007232, 0.412018, 0.884589 ],
                        [ 0.007074, 0.416971, 0.883916 ],
                        [ 0.007974, 0.421684, 0.882864 ],
                        [ 0.009823, 0.426175, 0.881494 ],
                        [ 0.012503, 0.430463, 0.879862 ],
                        [ 0.015894, 0.434570, 0.878018 ],
                        [ 0.019868, 0.438517, 0.876006 ],
                        [ 0.024301, 0.442326, 0.873865 ],
                        [ 0.029070, 0.446021, 0.871629 ],
                        [ 0.034057, 0.449623, 0.869326 ],
                        [ 0.039151, 0.453153, 0.866981 ],
                        [ 0.044246, 0.456634, 0.864616 ],
                        [ 0.049250, 0.460084, 0.862249 ],
                        [ 0.054077, 0.463523, 0.859893 ],
                        [ 0.058653, 0.466968, 0.857564 ],
                        [ 0.062915, 0.470435, 0.855272 ],
                        [ 0.066811, 0.473938, 0.853027 ],
                        [ 0.070298, 0.477489, 0.850837 ],
                        [ 0.073345, 0.481100, 0.848712 ],
                        [ 0.075930, 0.484780, 0.846657 ],
                        [ 0.078040, 0.488535, 0.844679 ],
                        [ 0.079672, 0.492373, 0.842784 ],
                        [ 0.080827, 0.496295, 0.840977 ],
                        [ 0.081518, 0.500305, 0.839261 ],
                        [ 0.081759, 0.504403, 0.837640 ],
                        [ 0.081571, 0.508587, 0.836116 ],
                        [ 0.080980, 0.512857, 0.834689 ],
                        [ 0.080014, 0.517207, 0.833360 ],
                        [ 0.078704, 0.521633, 0.832125 ],
                        [ 0.077082, 0.526128, 0.830983 ],
                        [ 0.075184, 0.530685, 0.829927 ],
                        [ 0.073043, 0.535297, 0.828951 ],
                        [ 0.070694, 0.539954, 0.828047 ],
                        [ 0.068170, 0.544647, 0.827204 ],
                        [ 0.065506, 0.549367, 0.826413 ],
                        [ 0.062734, 0.554102, 0.825659 ],
                        [ 0.059883, 0.558843, 0.824929 ],
                        [ 0.056984, 0.563579, 0.824207 ],
                        [ 0.054064, 0.568299, 0.823479 ],
                        [ 0.051149, 0.572994, 0.822727 ],
                        [ 0.048264, 0.577654, 0.821934 ],
                        [ 0.045430, 0.582268, 0.821083 ],
                        [ 0.042668, 0.586828, 0.820159 ],
                        [ 0.039998, 0.591326, 0.819143 ],
                        [ 0.037438, 0.595753, 0.818022 ],
                        [ 0.035004, 0.600102, 0.816780 ],
                        [ 0.032712, 0.604367, 0.815403 ],
                        [ 0.030575, 0.608542, 0.813880 ],
                        [ 0.028608, 0.612623, 0.812201 ],
                        [ 0.026822, 0.616606, 0.810356 ],
                        [ 0.025229, 0.620487, 0.808338 ],
                        [ 0.023842, 0.624266, 0.806143 ],
                        [ 0.022669, 0.627939, 0.803767 ],
                        [ 0.021723, 0.631507, 0.801209 ],
                        [ 0.021013, 0.634971, 0.798470 ],
                        [ 0.020548, 0.638331, 0.795553 ],
                        [ 0.020339, 0.641589, 0.792460 ],
                        [ 0.020393, 0.644748, 0.789198 ],
                        [ 0.020721, 0.647811, 0.785774 ],
                        [ 0.021330, 0.650782, 0.782197 ],
                        [ 0.022228, 0.653665, 0.778474 ],
                        [ 0.023424, 0.656463, 0.774616 ],
                        [ 0.024924, 0.659183, 0.770634 ],
                        [ 0.026735, 0.661830, 0.766537 ],
                        [ 0.028863, 0.664408, 0.762338 ],
                        [ 0.031313, 0.666922, 0.758045 ],
                        [ 0.034090, 0.669380, 0.753669 ],
                        [ 0.037198, 0.671785, 0.749220 ],
                        [ 0.040640, 0.674143, 0.744706 ],
                        [ 0.044416, 0.676459, 0.740136 ],
                        [ 0.048530, 0.678738, 0.735516 ],
                        [ 0.052979, 0.680984, 0.730851 ],
                        [ 0.057763, 0.683203, 0.726147 ],
                        [ 0.062880, 0.685396, 0.721406 ],
                        [ 0.068327, 0.687569, 0.716632 ],
                        [ 0.074100, 0.689723, 0.711824 ],
                        [ 0.080192, 0.691862, 0.706983 ],
                        [ 0.086599, 0.693987, 0.702108 ],
                        [ 0.093311, 0.696101, 0.697196 ],
                        [ 0.100323, 0.698202, 0.692246 ],
                        [ 0.107625, 0.700294, 0.687253 ],
                        [ 0.115207, 0.702374, 0.682214 ],
                        [ 0.123059, 0.704443, 0.677124 ],
                        [ 0.131171, 0.706501, 0.671981 ],
                        [ 0.139531, 0.708544, 0.666779 ],
                        [ 0.148129, 0.710572, 0.661515 ],
                        [ 0.156952, 0.712582, 0.656186 ],
                        [ 0.165989, 0.714572, 0.650789 ],
                        [ 0.175229, 0.716538, 0.645323 ],
                        [ 0.184658, 0.718478, 0.639787 ],
                        [ 0.194267, 0.720388, 0.634181 ],
                        [ 0.204042, 0.722264, 0.628507 ],
                        [ 0.213974, 0.724103, 0.622767 ],
                        [ 0.224051, 0.725901, 0.616965 ],
                        [ 0.234263, 0.727653, 0.611106 ],
                        [ 0.244600, 0.729356, 0.605197 ],
                        [ 0.255051, 0.731007, 0.599245 ],
                        [ 0.265609, 0.732600, 0.593258 ],
                        [ 0.276262, 0.734133, 0.587247 ],
                        [ 0.287004, 0.735603, 0.581221 ],
                        [ 0.297826, 0.737005, 0.575192 ],
                        [ 0.308720, 0.738337, 0.569172 ],
                        [ 0.319678, 0.739596, 0.563171 ],
                        [ 0.330694, 0.740780, 0.557204 ],
                        [ 0.341758, 0.741887, 0.551280 ],
                        [ 0.352866, 0.742916, 0.545413 ],
                        [ 0.364009, 0.743864, 0.539613 ],
                        [ 0.375180, 0.744732, 0.533891 ],
                        [ 0.386373, 0.745519, 0.528258 ],
                        [ 0.397580, 0.746225, 0.522721 ],
                        [ 0.408794, 0.746850, 0.517288 ],
                        [ 0.420006, 0.747397, 0.511966 ],
                        [ 0.431210, 0.747866, 0.506759 ],
                        [ 0.442397, 0.748258, 0.501672 ],
                        [ 0.453558, 0.748577, 0.496706 ],
                        [ 0.464686, 0.748826, 0.491861 ],
                        [ 0.475772, 0.749006, 0.487137 ],
                        [ 0.486805, 0.749121, 0.482530 ],
                        [ 0.497779, 0.749175, 0.478038 ],
                        [ 0.508682, 0.749172, 0.473655 ],
                        [ 0.519507, 0.749116, 0.469375 ],
                        [ 0.530243, 0.749010, 0.465191 ],
                        [ 0.540882, 0.748860, 0.461095 ],
                        [ 0.551417, 0.748668, 0.457078 ],
                        [ 0.561837, 0.748440, 0.453133 ],
                        [ 0.572136, 0.748179, 0.449249 ],
                        [ 0.582307, 0.747889, 0.445418 ],
                        [ 0.592343, 0.747574, 0.441631 ],
                        [ 0.602240, 0.747236, 0.437881 ],
                        [ 0.611994, 0.746880, 0.434159 ],
                        [ 0.621600, 0.746507, 0.430460 ],
                        [ 0.631058, 0.746119, 0.426778 ],
                        [ 0.640366, 0.745718, 0.423109 ],
                        [ 0.649527, 0.745306, 0.419449 ],
                        [ 0.658542, 0.744882, 0.415795 ],
                        [ 0.667416, 0.744447, 0.412148 ],
                        [ 0.676153, 0.744001, 0.408507 ],
                        [ 0.684761, 0.743542, 0.404875 ],
                        [ 0.693248, 0.743069, 0.401253 ],
                        [ 0.701624, 0.742581, 0.397645 ],
                        [ 0.709899, 0.742075, 0.394055 ],
                        [ 0.718086, 0.741549, 0.390486 ],
                        [ 0.726197, 0.741001, 0.386944 ],
                        [ 0.734247, 0.740427, 0.383434 ],
                        [ 0.742249, 0.739826, 0.379958 ],
                        [ 0.750217, 0.739194, 0.376522 ],
                        [ 0.758166, 0.738531, 0.373126 ],
                        [ 0.766110, 0.737834, 0.369775 ],
                        [ 0.774063, 0.737104, 0.366465 ],
                        [ 0.782035, 0.736339, 0.363198 ],
                        [ 0.790040, 0.735542, 0.359968 ],
                        [ 0.798085, 0.734714, 0.356771 ],
                        [ 0.806178, 0.733859, 0.353599 ],
                        [ 0.814326, 0.732982, 0.350443 ],
                        [ 0.822529, 0.732090, 0.347292 ],
                        [ 0.830789, 0.731191, 0.344131 ],
                        [ 0.839102, 0.730295, 0.340946 ],
                        [ 0.847462, 0.729414, 0.337722 ],
                        [ 0.855858, 0.728561, 0.334438 ],
                        [ 0.864279, 0.727754, 0.331080 ],
                        [ 0.872706, 0.727008, 0.327625 ],
                        [ 0.881121, 0.726343, 0.324055 ],
                        [ 0.889499, 0.725778, 0.320353 ],
                        [ 0.897813, 0.725336, 0.316501 ],
                        [ 0.906035, 0.725040, 0.312486 ],
                        [ 0.914131, 0.724912, 0.308295 ],
                        [ 0.922066, 0.724976, 0.303917 ],
                        [ 0.929804, 0.725256, 0.299339 ],
                        [ 0.937307, 0.725774, 0.294570 ],
                        [ 0.944536, 0.726553, 0.289602 ],
                        [ 0.951451, 0.727612, 0.284445 ],
                        [ 0.958013, 0.728971, 0.279108 ],
                        [ 0.964186, 0.730645, 0.273606 ],
                        [ 0.969932, 0.732648, 0.267960 ],
                        [ 0.975219, 0.734987, 0.262186 ],
                        [ 0.980016, 0.737670, 0.256315 ],
                        [ 0.984298, 0.740693, 0.250383 ],
                        [ 0.988043, 0.744058, 0.244414 ],
                        [ 0.991233, 0.747753, 0.238445 ],
                        [ 0.993860, 0.751765, 0.232517 ],
                        [ 0.995918, 0.756075, 0.226656 ],
                        [ 0.997410, 0.760660, 0.220897 ],
                        [ 0.998346, 0.765492, 0.215273 ],
                        [ 0.998740, 0.770540, 0.209814 ],
                        [ 0.998618, 0.775766, 0.204533 ],
                        [ 0.998008, 0.781136, 0.199456 ],
                        [ 0.996947, 0.786610, 0.194590 ],
                        [ 0.995480, 0.792148, 0.189935 ],
                        [ 0.993652, 0.797714, 0.185495 ],
                        [ 0.991518, 0.803269, 0.181230 ],
                        [ 0.989133, 0.808783, 0.177157 ],
                        [ 0.986557, 0.814232, 0.173243 ],
                        [ 0.983851, 0.819592, 0.169427 ],
                        [ 0.981075, 0.824858, 0.165695 ],
                        [ 0.978289, 0.830020, 0.162025 ],
                        [ 0.975551, 0.835090, 0.158336 ],
                        [ 0.972914, 0.840086, 0.154631 ],
                        [ 0.970425, 0.845033, 0.150841 ],
                        [ 0.968128, 0.849967, 0.146964 ],
                        [ 0.966058, 0.854944, 0.142970 ],
                        [ 0.964242, 0.860003, 0.138868 ],
                        [ 0.962699, 0.865202, 0.134624 ],
                        [ 0.961438, 0.870595, 0.130264 ],
                        [ 0.960463, 0.876230, 0.125807 ],
                        [ 0.959771, 0.882149, 0.121286 ],
                        [ 0.959348, 0.888379, 0.116730 ],
                        [ 0.959176, 0.894922, 0.112174 ],
                        [ 0.959241, 0.901756, 0.107614 ],
                        [ 0.959524, 0.908836, 0.103147 ],
                        [ 0.960009, 0.916100, 0.098686 ],
                        [ 0.960688, 0.923451, 0.094267 ],
                        [ 0.961568, 0.930778, 0.089829 ],
                        [ 0.962659, 0.937985, 0.085251 ],
                        [ 0.964003, 0.945008, 0.080502 ],
                        [ 0.965651, 0.951839, 0.075546 ],
                        [ 0.967680, 0.958659, 0.070202 ],
                        [ 0.970199, 0.965828, 0.064644 ],
                        [ 0.973325, 0.974050, 0.059096 ],
                        [ 0.977210, 0.984569, 0.054189 ]]
                         
    # read the source file.
    reader = vtk.vtkUnstructuredGridReader()
    reader.SetFileName(filename)
    reader.Update() 
    output = reader.GetOutputPort()
    scalar_range = reader.GetOutput().GetScalarRange()
    
    # create own palette
    lut = vtk.vtkLookupTable()
    lut.SetRange(scalar_range)
    for i in range(len(colobar_agros)):
        lut.SetTableValue(i, colobar_agros[i][0], colobar_agros[i][1], colobar_agros[i][2])
    lut.Build()
    
    # create the mapper that corresponds the objects of the vtk file into graphics elements
    mapper = vtk.vtkDataSetMapper()
    mapper.SetInputConnection(output)
    mapper.SetScalarRange(scalar_range)
    mapper.SetLookupTable(lut)
     
    # actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    
    return actor
                                  
def figure(geometry, scalar = None, contours = None, scalar_colorbar = True, width = 600, height = 300):
    from IPython.display import Image

    import os
    os.environ['DISPLAY']=':99.0'

    # bounding box
    bounds = geometry.GetBounds()
    cx = (bounds[0] + bounds[1]) / 2.0
    cy = (bounds[2] + bounds[3]) / 2.0
    scale = max(math.fabs(bounds[0] - cx), math.fabs(bounds[1] - cx), math.fabs(bounds[2] - cy), math.fabs(bounds[3] - cy))

    # renderer
    renderer = vtk.vtkRenderer()
    renderer.SetBackground(1, 1, 1)
        
    # scalar
    if (scalar != None):
        renderer.AddActor(scalar)  
            
    # contours
    if (contours != None):
        renderer.AddActor(contours)

    # geometry
    renderer.AddActor(geometry)
        
    # camera
    camera = renderer.GetActiveCamera()
    camera.ParallelProjectionOn()
    camera.SetParallelScale(scale)
    camera.SetPosition(cx, cy, 1);
    camera.SetFocalPoint(cx, cy, 0);
    
    # render window
    render_window = vtk.vtkRenderWindow()
    render_window.SetOffScreenRendering(True)
    render_window.SetSize(width, height)
    render_window.AddRenderer(renderer)
    render_window.Render()
    
    # scalar colorbar
    if (scalar != None):
        if (scalar_colorbar):
            interactor = vtk.vtkRenderWindowInteractor()
            interactor.SetRenderWindow(render_window)
            
            scalar_bar = vtk.vtkScalarBarActor()
            scalar_bar.SetOrientationToHorizontal()
            scalar_bar.SetNumberOfLabels(8)
            scalar_bar.SetLabelFormat("%+#6.2e")
            scalar_bar.SetLookupTable(scalar.GetMapper().GetLookupTable())
            scalar_bar.GetLabelTextProperty().SetFontFamilyToCourier()
            scalar_bar.GetLabelTextProperty().SetJustificationToRight()
            scalar_bar.GetLabelTextProperty().SetVerticalJustificationToCentered()
            scalar_bar.GetLabelTextProperty().BoldOff()
            scalar_bar.GetLabelTextProperty().ItalicOff()
            scalar_bar.GetLabelTextProperty().ShadowOff()        
            scalar_bar.GetLabelTextProperty().SetColor(0, 0, 0)
                
            # create the scalar bar widget
            scalar_bar_widget = vtk.vtkScalarBarWidget()
            scalar_bar_widget.SetInteractor(interactor)
            scalar_bar_widget.SetScalarBarActor(scalar_bar)
            scalar_bar_widget.On()    
       
    windowToImageFilter = vtk.vtkWindowToImageFilter()
    windowToImageFilter.SetInput(render_window)
    windowToImageFilter.Update()
     
    writer = vtk.vtkPNGWriter()
    writer.SetWriteToMemory(True)
    writer.SetInputConnection(windowToImageFilter.GetOutputPort())
    writer.Write()
    data = memoryview(writer.GetResult()).tobytes()
    
    return Image(data)   
    
def show(problem, computation, field, variable, component, time_step=0, adaptive_step=0, width=600, height=400):
    temp_geometry_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    problem.geometry().export_vtk(temp_geometry_name)
    
    geometry = geometry_actor(temp_geometry_name)    
    
    temp_post_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    solution = computation.solution(field)
    solution.export_vtk(temp_post_name, time_step, adaptive_step, variable, component)

    scalar = scalar_actor(temp_post_name)
    contours = contours_actor(temp_post_name)    
    
    fig = figure(geometry=geometry, scalar=scalar, contours=contours, width=width, height=height)
    
    os.remove(temp_post_name)
    os.remove(temp_geometry_name)
    
    return fig
    
def show_geometry(problem, width=600, height=400):
    temp_geometry_name = "{}.vtk".format(next(tempfile._get_candidate_names()))
    problem.geometry().export_vtk(temp_geometry_name)
    
    geometry = geometry_actor(temp_geometry_name)    
    
    fig = figure(geometry=geometry, width=width, height=height)
    
    os.remove(temp_geometry_name)
    
    return fig


