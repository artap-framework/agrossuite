#include <vector>
#include <limits>
#include "aux_math.h"
#include <cmath>
using namespace std;

namespace MathAux
{

// ----------------------------------------------------------------------
// ASF: Achivement Scalarization Function
// ----------------------------------------------------------------------
double ASF(const vector<double> &objs, const vector<double> &weight)
{
	double max_ratio = -numeric_limits<double>::max();
	for (size_t f=0; f<objs.size(); f+=1)
	{
		double w = weight[f]?weight[f]:0.00001;
		max_ratio = std::max(max_ratio, objs[f]/w);
	}
	return max_ratio;
}

// ---------------------------------------------------------------------
// GuassianElimination:
//
// Given a NxN matrix A and a Nx1 vector b, generate a Nx1 vector x
// such that Ax = b.
//
// Use this to get a hyperplane for a given set of points.
// Example.
//
// Given three points (-1, 1, 2), (2, 0, -3), and (5, 1, -2).
// The equation of the hyperplane is ax+by+cz=d, or a'x +b'y + c'z = 1.
// So we have
//
//    (-1)a' + b' + 2c' = 1.
//    2a' - 3c' = 1.
//    5a' + b' -2c' = 1.
//
// Let A be { {-1, 1, 2}, {2, 0, -3}, {5, 1, -2} } and b be {1, 1, 1}.
// This function will generate x as {-0.4, 1.8, -0.6},
// which means the equation is (-0.4)x + 1.8y - 0.6z = 1, or 2x-9y+3z+5=0.
//
// The intercepts are {1/-0.4, 0, 0}, {0, 1/1.8, 0}, and {0, 0, 1/-0.6}.
//
// Code example:
//
//    vector<double> x, b = {1, 1, 1};
//    vector< vector<double> > A = { {-1, 1, 2}, {2, 0, -3}, {5, 1, -2}  };
//
//    GuassianElimination(x, A, b);
//    cout << x[0] << ' ' << x[1] << ' ' << x[2] << endl;
// ---------------------------------------------------------------------
void GuassianElimination(vector<double> *px, vector< vector<double> > A, const vector<double> &b)
{
	vector<double> &x = *px;

    const size_t N = A.size();
    for (size_t i=0; i<N; i+=1)
    {
        A[i].push_back(b[i]);
    }

    for (size_t base=0; base<N-1; base+=1)
    {
        for (size_t target=base+1; target<N; target+=1)
        {
            double ratio = A[target][base]/A[base][base];
            for (size_t term=0; term<A[base].size(); term+=1)
            {
                A[target][term] -= A[base][term]*ratio;
            }
        }
    }

    x.resize(N);
    for (int i=N-1; i>=0; i-=1)
    {
        for (size_t known=i+1; known<N; known+=1)
        {
            A[i][N] -= A[i][known]*x[known];
        }
        x[i] = A[i][N]/A[i][i];
    }
}


// ---------------------------------------------------------------------
// PerpendicularDistance:
//
// Given a direction vector (w1, w2) and a point P(x1, y1),
// we want to find a point Q(x2, y2) on the line connecting (0, 0)-(w1, w2)
// such that (x1-x2, y1-y2) is perpendicular to (w1, w2).
//
// Since Q is on the line (0, 0)-(w1, w2), it should be (w1*k, w2*k).
// (x1-w1*k, y1-w2*k).(w1, w2) = 0. (inner product)
// => k(w1^2 + w2^2) = w1x1 + w2x2
// => k = (w1x1 + w2x2)/(w1^2 +w2^2).
//
// After obtaining k, we have Q = (w1*k, w2*k) and the distance between P and Q.
//
// Code example:
//    vector<double> dir{1, 3}, point{5.5, 1.5};
//    cout << PerpendicularDistance(dir, point) << endl;
// ---------------------------------------------------------------------
double PerpendicularDistance(const vector<double> &direction, const vector<double> &point)
{
    double numerator = 0, denominator = 0;
    for (size_t i=0; i<direction.size(); i+=1)
    {
        numerator += direction[i]*point[i];
        denominator += square(direction[i]);
    }
    double k = numerator/denominator;

    double d = 0;
    for (size_t i=0; i<direction.size(); i+=1)
    {
        d += square(k*direction[i] - point[i]);
    }
    return sqrt(d);
}
// ---------------------------------------------------------------------




}// namespace MathAux