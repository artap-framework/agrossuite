import org.agros.utils;
import org.agros.*;

public class main {
   static {
     System.loadLibrary("_wrapper_java");
   }

   public static void main(String argv[]) {
     System.out.println(utils.version());
     
     Problem problem = new org.agros.Problem(true);
          
     Field electrostatic = problem.field("electrostatic");        
     electrostatic.setAnalysisType("steadystate");

     Geometry geometry = problem.geometry();

     Computation computation = problem.computation();
     computation.solve();

     System.out.println(problem.getCoordinateType());
     System.out.println(electrostatic.getAnalysisType());     
   }
}
