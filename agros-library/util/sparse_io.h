// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <assert.h>
#include <cstring>
#include <math.h>

#include <chrono>

#include "tclap/CmdLine.h"

class SparseMatrixRW
{
public:
    SparseMatrixRW();
    ~SparseMatrixRW();

    void clear()
    {
        max_len = 0;

        if (val)
        {
            delete [] val;
            val = nullptr;
        }
    }

    std::size_t max_len;
    double *val;

    void block_write(std::ostream &out) const
    {
        assert(out);

        // first the simple objects,
        // bracketed in [...]
        out << '[' << max_len << "][";
        // then write out real data
        out.write (reinterpret_cast<const char *>(&val[0]),
                reinterpret_cast<const char *>(&val[max_len])
                - reinterpret_cast<const char *>(&val[0]));
        out << ']';

        assert(out);
    }

    void block_read(std::istream &in)
    {
        assert(in);

        char c;

        // first read in simple data
        in >> c;
        assert(c == '[');
        in >> max_len;

        in >> c;
        assert(c == ']');
        in >> c;
        assert(c == '[');

        // reallocate space
        if (val)
            delete [] val;
        val = new double[max_len];

        // then read data
        in.read (reinterpret_cast<char *>(&val[0]),
                reinterpret_cast<char *>(&val[max_len])
                - reinterpret_cast<char *>(&val[0]));
        in >> c;
        assert(c == ']');
    }
};

inline SparseMatrixRW::SparseMatrixRW() : max_len(0), val(nullptr)
{
}

inline SparseMatrixRW::~SparseMatrixRW()
{
    clear();
}

class SparsityPatternRW
{
public:
    SparsityPatternRW();
    ~SparsityPatternRW();

    void clear()
    {
        max_dim = 0;
        rows = 0;
        cols = 0;
        max_vec_len = 0;
        max_row_length = 0;

        compressed = false;
        store_diagonal_first_in_row = false;

        if (rowstart)
        {
            delete [] rowstart;
            rowstart = nullptr;
        }
        if (colnums)
        {
            delete [] colnums;
            colnums = nullptr;
        }
    }

    unsigned int max_dim;
    unsigned int rows;
    unsigned int cols;
    unsigned int max_vec_len;
    unsigned int max_row_length;
    std::size_t *rowstart;
    unsigned int *colnums;
    bool compressed;
    bool store_diagonal_first_in_row;

    void block_write(std::ostream &out) const
    {
        assert(out);

        // first the simple objects, bracketed in [...]
        out << '[' << max_dim << ' '
            << rows << ' '
            << cols << ' '
            << max_vec_len << ' '
            << max_row_length << ' '
            << compressed << ' '
            << store_diagonal_first_in_row << "][";
        // then write out real data
        out.write (reinterpret_cast<const char *>(&rowstart[0]),
                reinterpret_cast<const char *>(&rowstart[max_dim+1])
                - reinterpret_cast<const char *>(&rowstart[0]));
        out << "][";
        out.write (reinterpret_cast<const char *>(&colnums[0]),
                reinterpret_cast<const char *>(&colnums[max_vec_len])
                - reinterpret_cast<const char *>(&colnums[0]));
        out << ']';

        assert(out);
    }

    void block_read(std::istream &in)
    {
        assert(in);

        char c;

        // first read in simple data
        in >> c;
        assert(c == '[');
        in >> max_dim
                >> rows
                >> cols
                >> max_vec_len
                >> max_row_length
                >> compressed
                >> store_diagonal_first_in_row;

        in >> c;
        assert(c == ']');
        in >> c;
        assert(c == '[');

        // reallocate space
        if (rowstart)
            delete [] rowstart;
        if (colnums)
            delete [] colnums;

        rowstart = new std::size_t[max_dim+1];
        colnums  = new unsigned int[max_vec_len];

        // then read data
        in.read (reinterpret_cast<char *>(&rowstart[0]),
                reinterpret_cast<char *>(&rowstart[max_dim+1])
                - reinterpret_cast<char *>(&rowstart[0]));
        in >> c;
        assert(c == ']');
        in >> c;
        assert(c == '[');
        in.read (reinterpret_cast<char *>(&colnums[0]),
                reinterpret_cast<char *>(&colnums[max_vec_len])
                - reinterpret_cast<char *>(&colnums[0]));
        in >> c;
        assert(c == ']');
    }
};

inline SparsityPatternRW::SparsityPatternRW() : max_dim(0), rows(0), cols(0), max_vec_len(0), max_row_length(0),
    rowstart(nullptr), colnums(nullptr), compressed(false), store_diagonal_first_in_row(false)
{
}

inline SparsityPatternRW::~SparsityPatternRW()
{
    clear();
}

class VectorRW
{
public:
    VectorRW();
    VectorRW(std::size_t len);
    ~VectorRW();

    inline void clear()
    {
        max_len = 0;

        if (val)
        {
            delete [] val;
            val = nullptr;
        }
    }

    void resize(std::size_t len);

    std::size_t max_len;
    double *val;

    bool compare(const std::string &fileName, double tolerance = 1e-3, double *diff_out = nullptr)
    {
        // read reference solution
        VectorRW ref;
        std::ifstream readSLN(fileName);
        ref.block_read(readSLN);
        readSLN.close();

        // compare
        return compare(ref, tolerance, diff_out);
    }

    bool compare(const VectorRW &comp, double relative_tolerance = 1e-1, double *diff_out = nullptr)
    {
        if (comp.max_len != max_len)
        {
            std::cerr << "Size doesn't match." << std::endl;
            return false;
        }

        double diff = 0.0;
        double sum = 0.0;
        for (unsigned int i = 0; i < max_len; i++)
        {
            sum += fabs(val[i]);
            diff += fabs(comp.val[i] - val[i]);
        }

        if (diff_out)
            *diff_out = diff / sum * 100;

        if (diff / sum > relative_tolerance)
        {
            std::cerr << "Difference is greater then " << (diff / sum) * 100 << " % " << std::endl;
            return false;
        }

        return true;
    }

    inline double &operator[] (std::size_t idx) { return val[idx]; }

    void block_write(std::ostream &out) const
    {
        assert(out);

        char buf[16];

        // DEAL_II_WITH_64BIT_INDICES - std::sprintf(buf, "%llu", sz);
        sprintf(buf, "%lu", max_len);
        strcat(buf, "\n[");

        out.write(buf, strlen(buf));
        out.write (reinterpret_cast<const char *>(&val[0]),
                reinterpret_cast<const char *>(&val[max_len])
                - reinterpret_cast<const char *>(&val[0]));

        const char outro = ']';
        out.write (&outro, 1);

        assert(out);
    }

    void block_read(std::istream &in)
    {
        assert(in);

        char buf[16];

        in.getline(buf,16,'\n');
        max_len = std::atoi(buf);

        // reallocate space
        if (val)
            delete [] val;
        val = new double[max_len];

        char c;
        in.read (&c, 1);
        assert(c == '[');

        in.read (reinterpret_cast<char *>(&val[0]),
                reinterpret_cast<const char *>(&val[max_len])
                - reinterpret_cast<const char *>(&val[0]));

        //  in >> c;
        in.read (&c, 1);
        assert(c == ']');
    }
};

inline VectorRW::VectorRW() : max_len(0), val(nullptr)
{
}

inline VectorRW::VectorRW(std::size_t len) : max_len(len), val(nullptr)
{
    resize(len);
}

inline void VectorRW::resize(std::size_t len)
{
    clear();

    max_len = len;
    val = new double[len];
    // zeroes
    memset(val, 0, sizeof(double) * len);
}

inline VectorRW::~VectorRW()
{
    clear();
}

class LinearSystem
{
public:
    LinearSystem(const std::string &name = "") : matA(nullptr), cooRowInd(nullptr), cooColInd(nullptr), csrRowPtr(nullptr), csrColInd(nullptr),
        infoName(name), infoNumOfProc(1), infoTimeReadMatrix(0.0), infoTimeSolver(0.0), infoTimeTotal(0.0), infoNumOfIterations(0),
        infoState("not_solved"), infoRelativeDiff(0.0)
    {
        // matrix system
        system_matrix_pattern = new SparsityPatternRW();
        system_matrix = new SparseMatrixRW();
        // rhs
        system_rhs = new VectorRW();
        // solution
        system_sln = new VectorRW();
        // initial guess
        initial_sln = new VectorRW();
        // reference solution (for testing)
        reference_sln = new VectorRW();
    }

    ~LinearSystem()
    {
        delete system_matrix_pattern;
        delete system_matrix;
        delete system_rhs;
        delete system_sln;
        delete initial_sln;
        delete reference_sln;

        // coo
        if (cooRowInd) { delete [] cooRowInd; cooRowInd = nullptr; }
        if (cooColInd) { delete [] cooColInd; cooColInd = nullptr; }
        // csr
        if (csrRowPtr) { delete [] csrRowPtr; csrRowPtr = nullptr; }
        if (csrColInd) { delete [] csrColInd; csrColInd = nullptr; }
    }

    // matrix system
    SparsityPatternRW *system_matrix_pattern;
    SparseMatrixRW *system_matrix;
    // rhs
    VectorRW *system_rhs;
    // solution
    VectorRW *system_sln;
    // initial guess
    VectorRW *initial_sln;
    // reference solution (for testing)
    VectorRW *reference_sln;

    inline unsigned int n() { return infoN; }
    inline unsigned int nz() { return infoNZ; }

    inline void setInfoNumOfProc(unsigned int num) { infoNumOfProc = num; }
    inline void setInfoTimeReadMatrix(double time) { infoTimeReadMatrix = time; }
    inline void setInfoTimeSolver(double time) { infoTimeSolver = time; }
    inline void setInfoSolverNumOfIterations(int num) { infoNumOfIterations = num; }
    inline void setInfoTimeTotal(double time) { infoTimeTotal = time; }
    inline void setInfoSolverStateSolved() { infoState = "solved"; }

    virtual void printStatus()
    {
        std::cout << "Solver: " << infoName << std::endl;
        std::cout << " - state: " << infoState << std::endl;
        std::cout << " - number of iterations: " << infoNumOfIterations << std::endl;
        std::cout << " - number of processes: " << infoNumOfProc << std::endl;
        std::cout << " - matrix size: " << n() << " (" << nz() << " nonzeros, " << 100 * nz() / pow(n(), 2.0) << " % of nonzero elements)" << std::endl;
        std::cout << " - relative diff.: " << infoRelativeDiff << std::endl;

        std::cout << "Elapsed times" << std::endl;
        std::cout << " - read matrix: " << infoTimeReadMatrix << " s" << std::endl;
        std::cout << " - solver: " << infoTimeSolver << " s" << std::endl;
        std::cout << " - total time: " << infoTimeTotal << " s" << std::endl;

        std::cout << "Arguments" << std::endl;
        std::cout << " - preconditioner: " << infoParameterPreconditioner << std::endl;
        std::cout << " - solver: " << infoParameterSolver << std::endl;
        std::cout << " - absolute tolerance: " << infoParameterAbsTol << std::endl;
        std::cout << " - relative tolerance: " << infoParameterRelTol << std::endl;
        std::cout << " - maximum number of iterations: " << infoParameterMaxIter << std::endl;
        std::cout << " - multigrid: " << infoParameterMultigrid << std::endl;
        std::cout << " - multigrid aggregator: " << infoParameterMultigridAggregator << std::endl;
        std::cout << " - multigrid smoother: " << infoParameterMultigridSmoother << std::endl;
        std::cout << " - multigrid coarser: " << infoParameterMultigridCoarser << std::endl;
        std::cout << " - multigrid sweeps: " << infoParameterNumSweeps << std::endl;

        std::cout << "Filename: " << infoFileName << std::endl;
        std::cout << "Arguments: " << infoArgs << std::endl;
    }

    void exportStatusToFile()
    {
        auto tp = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>( tp.time_since_epoch() );
        size_t modulo = ms.count() % 1000;
        time_t seconds = std::chrono::duration_cast<std::chrono::seconds>( ms ).count();

        std::stringstream ss;
        ss << std::put_time(localtime(&seconds), "output_%Y-%m-%d %H-%M-%S.") << modulo << "_" << infoName << "_" << infoParameterPreconditioner << "_" << infoParameterSolver << ".out";
        std::string fn = ss.str();

        std::ofstream file(fn);

        if (file.is_open())
        {
            file << "solver_name = " << infoName << "\n";
            file << "solver_state = " << infoState << "\n";
            file << "solver_num_of_iterations = " << infoNumOfIterations << "\n";
            file << "solver_num_of_proc = " << infoNumOfProc << "\n";
            file << "solver_matrix_size = " << n() << "\n";
            file << "solver_matrix_nonzeros = " << nz() << "\n";
            file << "solver_matrix_nonzeros_percentage = " << 100 * nz() / pow(n(), 2.0) << "\n";
            file << "solver_rel_diff = " << infoRelativeDiff << "\n";

            file << "time_read_matrix = " << infoTimeReadMatrix << "\n";
            file << "time_solver = " << infoTimeSolver << "\n";
            file << "time_total = " << infoTimeTotal << "\n";

            file << "parameter_preconditioner = " << infoParameterPreconditioner << "\n";
            file << "parameter_solver = " << infoParameterSolver << "\n";

            file << "parameter_abs_tol = " << infoParameterAbsTol << "\n";
            file << "parameter_rel_tol = " << infoParameterRelTol << "\n";
            file << "parameter_max_iter = " << infoParameterMaxIter << "\n";
            file << "parameter_multigrid = " << infoParameterMultigrid << "\n";
            file << "parameter_multigrid_aggregator = " << infoParameterMultigridAggregator << "\n";
            file << "parameter_multigrid_smoother = " << infoParameterMultigridSmoother << "\n";
            file << "parameter_multigrid_coarser = " << infoParameterMultigridCoarser << "\n";
            file << "parameter_multigrid_sweeps = " << infoParameterNumSweeps << "\n";

            file << "filename = " << infoFileName << "\n";
            file << "arguments = " << infoArgs << "\n";

            file.close();
        }
        else
        {
            std::cerr << "Could not open file." << std::endl;
        }
    }

    int compareWithReferenceSolution(double relative_tolerance = 1e-1)
    {
        return reference_sln->compare(*system_sln, relative_tolerance, &infoRelativeDiff) ? 0 : -1;
    }

    void convertToCOO()
    {
        // coo
        if (cooRowInd) delete [] cooRowInd;
        if (cooColInd) delete [] cooColInd;

        cooRowInd = new unsigned int[nz()];
        cooColInd = new unsigned int[nz()];
        matA = system_matrix->val;

        int index = 0;

        // loop over the elements of the matrix row by row
        for (int row = 0; row < n(); row++)
        {
            std::size_t col_start = system_matrix_pattern->rowstart[row];
            std::size_t col_end = system_matrix_pattern->rowstart[row + 1];

            for (int i = col_start; i < col_end; i++)
            {
                cooRowInd[index] = row + 0;
                cooColInd[index] = system_matrix_pattern->colnums[i] + 0;

                index++;
            }
        }
    }

    void convertToCSR()
    {
        // csr
        if (csrRowPtr) delete [] csrRowPtr;
        if (csrColInd) delete [] csrColInd;

        csrRowPtr = new unsigned int[n() + 1];
        csrColInd = new unsigned int[nz()];
        matA = system_matrix->val;

        unsigned int index = 0;

        // loop over the elements of the matrix row by row
        for (unsigned int row = 0; row < n(); row++)
        {
            std::size_t col_start = system_matrix_pattern->rowstart[row];
            std::size_t col_end = system_matrix_pattern->rowstart[row + 1];

            csrRowPtr[row] = index;

            for (unsigned int i = col_start; i < col_end; i++)
            {
                csrColInd[index] = system_matrix_pattern->colnums[i];

                index++;
            }
        }

        csrRowPtr[n()] = nz();
    }

    double *matA;

    // coo
    unsigned int *cooRowInd;
    unsigned int *cooColInd;

    // csr
    unsigned int *csrRowPtr;
    unsigned int *csrColInd;

    std::string infoParameterPreconditioner;
    std::string infoParameterSolver;
    double infoParameterAbsTol;
    double infoParameterRelTol;
    int infoParameterMaxIter;
    bool infoParameterMultigrid;
    std::string infoParameterMultigridAggregator;
    std::string infoParameterMultigridSmoother;
    std::string infoParameterMultigridCoarser;
    int infoParameterNumSweeps;

    std::string infoName;

    int infoNumOfProc;
    double infoTimeReadMatrix;
    double infoTimeSolver;
    double infoTimeTotal;
    std::string infoState;
    unsigned int infoNumOfIterations;
    unsigned int infoN;
    unsigned int infoNZ;
    double infoRelativeDiff;

    std::string infoArgs;
    std::string infoFileName;

protected:
    void readLinearSystemInternal(const std::string &matrixPaternFN,
                                  const std::string &matrixFN,
                                  const std::string &rhsFN,
                                  const std::string &initialFN = "",
                                  const std::string &referenceFN = "")
    {        
        std::ifstream readMatrixSparsityPattern(matrixPaternFN);
        assert(readMatrixSparsityPattern.is_open());
        system_matrix_pattern->block_read(readMatrixSparsityPattern);
        readMatrixSparsityPattern.close();

        std::ifstream readMatrix(matrixFN);
        assert(readMatrix.is_open());
        system_matrix->block_read(readMatrix);
        readMatrix.close();

        std::ifstream readRHS(rhsFN);
        assert(readRHS.is_open());
        system_rhs->block_read(readRHS);
        readRHS.close();

        if (!initialFN.empty())
        {
            std::ifstream readInitial(initialFN);
            assert(readInitial.is_open());
            initial_sln->block_read(readInitial);
            readInitial.close();
        }

        if (!referenceFN.empty())
        {
            std::ifstream readReference(referenceFN);
            reference_sln->block_read(readReference);
            readReference.close();
        }

        infoN = system_matrix_pattern->rows;
        infoNZ = system_matrix_pattern->rowstart[system_matrix_pattern->rows]-system_matrix_pattern->rowstart[0];
    }

    void writeSolutionInternal(const std::string &solutionFN = "")
    {
        // TODO: check existence
        std::ofstream writeSln(solutionFN);
        system_sln->block_write(writeSln);
        writeSln.close();
    }
};

class LinearSystemArgs : public LinearSystem
{
public:
    LinearSystemArgs(const std::string &name, int argc, const char * const *argv) : LinearSystem(name),
        cmd(name, ' '),

        matrixArg(TCLAP::ValueArg<std::string>("m", "matrix", "Matrix", true, "", "string")),
        matrixPatternArg(TCLAP::ValueArg<std::string>("p", "matrix_pattern", "Matrix pattern", true, "", "string")),
        rhsArg(TCLAP::ValueArg<std::string>("r", "rhs", "RHS", true, "", "string")),
        solutionArg(TCLAP::ValueArg<std::string>("s", "solution", "Solution", false, "", "string")),
        referenceSolutionArg(TCLAP::ValueArg<std::string>("q", "reference_solution", "Reference solution", false, "", "string")),
        initialArg(TCLAP::ValueArg<std::string>("i", "initial", "Initial vector", false, "", "string")),

        solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
        preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),

        multigridArg(TCLAP::SwitchArg("g", "multigrid", "Algebraic multigrid", false)),
        multigridAggregatorTypeArg(TCLAP::ValueArg<std::string>("e", "aggregationType", "AggregationType", false, "", "string")),
        multigridSmootherTypeArg(TCLAP::ValueArg<std::string>("o", "smootherType", "SmootherType", false, "", "string")),
        multigridCoarserTypeArg(TCLAP::ValueArg<std::string>("z", "coarseType", "CoarseType", false, "", "string")),

        absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
        relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
        maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 2000, "int")),
        numSweepsArg(TCLAP::ValueArg<int>("w", "num_sweeps", "Number of sweeps", false, -1, "int")),

        verboseArg(TCLAP::ValueArg<int>("v", "verbose", "Verbose mode", false, 0, "int")),

        argc(argc),
        argv(argv)
    {
        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(referenceSolutionArg);
        cmd.add(initialArg);

        cmd.add(solverArg);
        cmd.add(preconditionerArg);

        cmd.add(multigridArg);
        cmd.add(multigridAggregatorTypeArg);
        cmd.add(multigridSmootherTypeArg);
        cmd.add(multigridCoarserTypeArg);

        cmd.add(absTolArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
        cmd.add(numSweepsArg);

        cmd.add(verboseArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        infoParameterSolver = solverArg.getValue();
        infoParameterPreconditioner = preconditionerArg.getValue();

        infoParameterMultigrid = multigridArg.getValue();
        infoParameterMultigridAggregator = multigridAggregatorTypeArg.getValue();
        infoParameterMultigridSmoother = multigridSmootherTypeArg.getValue();
        infoParameterMultigridCoarser = multigridCoarserTypeArg.getValue();

        infoParameterAbsTol = absTolArg.getValue();
        infoParameterRelTol = relTolArg.getValue();
        infoParameterMaxIter = maxIterArg.getValue();
        infoParameterNumSweeps = numSweepsArg.getValue();
    }

    inline bool hasSolution() { return !solutionArg.getValue().empty(); }
    inline std::string solutionFileName() { return solutionArg.getValue(); }
    inline bool hasReferenceSolution() { return !referenceSolutionArg.getValue().empty(); }

    inline int verbose() { return verboseArg.getValue(); }

    virtual void readLinearSystem()
    {
        for (int i = 0; i < argc; i++)
        {
            if (i > 0)
                infoArgs += " ";

            infoArgs += argv[i];
        }

        infoFileName = matrixArg.getValue().substr(0, matrixArg.getValue().length() - 7);

        readLinearSystemInternal(matrixPatternArg.getValue(),
                                 matrixArg.getValue(),
                                 rhsArg.getValue(),
                                 initialArg.getValue(),
                                 referenceSolutionArg.getValue());
    }

    void writeSolution()
    {
        // system_rhs (solution)
        if (hasSolution())
            writeSolutionInternal(solutionFileName());
    }

    // matrices and vectors
    TCLAP::ValueArg<std::string> matrixArg;
    TCLAP::ValueArg<std::string> matrixPatternArg;
    TCLAP::ValueArg<std::string> rhsArg;
    TCLAP::ValueArg<std::string> solutionArg;
    TCLAP::ValueArg<std::string> referenceSolutionArg;
    TCLAP::ValueArg<std::string> initialArg;
    // verbose mode (0 .. disabled, 1 .. simple verbose, 2 .. details, 3 .. file output)
    TCLAP::ValueArg<int> verboseArg;

protected:
    // command line info
    TCLAP::CmdLine cmd;

    // iterative solver
    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<std::string> preconditionerArg;
    // iterative solver control
    TCLAP::ValueArg<double> absTolArg;
    TCLAP::ValueArg<double> relTolArg;
    TCLAP::ValueArg<int> maxIterArg;
    // multigrid
    TCLAP::SwitchArg multigridArg;
    TCLAP::ValueArg<std::string> multigridAggregatorTypeArg;
    TCLAP::ValueArg<std::string> multigridSmootherTypeArg;
    TCLAP::ValueArg<std::string> multigridCoarserTypeArg;
    TCLAP::ValueArg<int> numSweepsArg;

private:
    int argc;
    const char * const *argv;
};

double elapsedSeconds(std::chrono::time_point<std::chrono::steady_clock> start,
                      std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now())
{
    return (end - start).count() * std::chrono::steady_clock::period::num / static_cast<double>(std::chrono::steady_clock::period::den);
}
