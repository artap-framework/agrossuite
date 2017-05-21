agros;
agros.initSingleton(); 

disp(agros.version());

problem = agros.Problem(); 

disp(problem.getMeshType());
disp(problem.getCoordinateType());

electrostatic = problem.field("electrostatic");        
electrostatic.setAnalysisType("steadystate");

geometry = problem.geometry();

computation = problem.computation();
# computation.solve();

disp(electrostatic.getAnalysisType());    
