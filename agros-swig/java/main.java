import org.agros.utils;
import org.agros.*;
import java.util.Map;
import java.util.HashMap;

public class main {
    static {
     System.loadLibrary("_wrapper_java");
    }

    public static void main(String argv[]) {
        System.out.println(utils.version());

        Problem problem = new org.agros.Problem(true);
          
        Field electrostatic = problem.field("electrostatic");        
        electrostatic.setAnalysisType("steadystate");
        
        // add boundaries
        // Map<String, Double> boundaryZero = new HashMap<>();
        // boundaryZero.put("electrostatic_potential", 0);
        // electrostatic.addBoundary("U = 0 V", "electrostatic_potential", boundaryZero);

        Geometry geometry = problem.geometry();

        System.out.println(problem.getCoordinateType());
        System.out.println(electrostatic.getAnalysisType());     

        Computation computation = problem.computation();
        // computation.solve();
    }
}
