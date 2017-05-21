using System;
using org.agros;

public class main {
     static void Main() {
         Console.WriteLine(utils.version());

         Problem problem = new Problem(true);
                  
         Field electrostatic = problem.field("electrostatic");        
         electrostatic.setAnalysisType("steadystate");

         Geometry geometry = problem.geometry();
                  
         Console.WriteLine(problem.getCoordinateType());
         Console.WriteLine(electrostatic.getAnalysisType());
         
         Computation computation = problem.computation();
         // computation.solve();         
     }
 }
