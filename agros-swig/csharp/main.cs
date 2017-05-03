using System;
using org.agros;

public class main {
     static void Main() {
         Console.WriteLine(utils.version());

         Problem problem = new Problem(true);
                  
         Field electrostatic = problem.field("electrostatic");        
         electrostatic.setAnalysisType("steadystate");

         Geometry geometry = problem.geometry();
         
         Computation computation = problem.computation();
         // computation.solve();
         
         Console.WriteLine(problem.getCoordinateType());
         Console.WriteLine(electrostatic.getAnalysisType());
     }
 }
