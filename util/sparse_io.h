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

#include <chrono>

#include "../3rdparty/tclap/CmdLine.h"

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

    bool compare(const std::string &fileName, double tolerance = 1e-3)
    {
        // read reference solution
        VectorRW ref;
        std::ifstream readSLN(fileName);
        ref.block_read(readSLN);
        readSLN.close();

        // compare
        return compare(ref, tolerance);
    }

    bool compare(const VectorRW &comp, double relative_tolerance = 1e-1)
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

void csr2csc(int size, int nnz, double *data, int *ir, int *jc)
{
    int *tempjc = new int[size + 1];
    int *tempir = new int[nnz];
    double *tempdata = new double[nnz];

    int run_i = 0;
    for (int target_row = 0; target_row < size; target_row++)
    {
        tempjc[target_row] = run_i;
        for (int src_column = 0; src_column < size; src_column++)
        {
            for (int src_row = jc[src_column]; src_row < jc[src_column + 1]; src_row++)
            {
                if (ir[src_row] == target_row)
                {
                    tempir[run_i] = src_column;
                    tempdata[run_i++] = data[src_row];
                }
            }
        }
    }

    tempjc[size] = nnz;

    memcpy(ir, tempir, sizeof(int) * nnz);
    memcpy(jc, tempjc, sizeof(int) * (size + 1));
    memcpy(data, tempdata, sizeof(double) * nnz);

    delete [] tempir;
    delete [] tempdata;
    delete [] tempjc;
}

class LinearSystem
{
public:
    LinearSystem(const std::string &name = "") : matA(nullptr), cooRowInd(nullptr), cooColInd(nullptr), csrRowPtr(nullptr), csrColInd(nullptr),
        infoName(name), infoNumOfProc(1), infoTimeReadMatrix(0.0), infoTimeSolveSystem(0.0), infoTimeTotal(0.0)
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

    inline unsigned int n() { return system_matrix_pattern->rows; }
    inline unsigned int nz() { return system_matrix->max_len; }

    inline void setInfoNumOfProc(unsigned int num) { infoNumOfProc = num; }
    inline void setInfoTimeReadMatrix(double time) { infoTimeReadMatrix = time; }
    inline void setInfoTimeSolveSystem(double time) { infoTimeSolveSystem = time; }
    inline void setInfoTimeTotal(double time) { infoTimeTotal = time; }

    void printStatus()
    {
        std::cout << "Solver: " << infoName << std::endl;
        std::cout << "Number of processes: " << infoNumOfProc << std::endl;
        std::cout << "Matrix size: " << n() << " (" << 100 * nz() / pow(n(), 2.0) << " % of nonzero elements)" << std::endl;
        std::cout << "Read matrix: " << infoTimeReadMatrix << " s" << std::endl;
        std::cout << "Solve system: " << infoTimeSolveSystem << " s" << std::endl;
        std::cout << "Total time: " << infoTimeTotal << " s" << std::endl;
    }

    int compareWithReferenceSolution(double relative_tolerance = 1e-1)
    {
        return reference_sln->compare(*system_sln, relative_tolerance) ? 0 : -1;
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

protected:
    void readLinearSystemInternal(const std::string &matrixPaternFN,
                                  const std::string &matrixFN,
                                  const std::string &rhsFN,
                                  const std::string &initialFN = "",
                                  const std::string &referenceFN = "")
    {
        // TODO: check existence

        std::ifstream readMatrixSparsityPattern(matrixPaternFN);
        system_matrix_pattern->block_read(readMatrixSparsityPattern);
        readMatrixSparsityPattern.close();

        std::ifstream readMatrix(matrixFN);
        system_matrix->block_read(readMatrix);
        readMatrix.close();

        std::ifstream readRHS(rhsFN);
        system_rhs->block_read(readRHS);
        readRHS.close();

        if (!initialFN.empty())
        {
            std::ifstream readInitial(initialFN);
            initial_sln->block_read(readInitial);
            readInitial.close();
        }

        if (!referenceFN.empty())
        {
            std::ifstream readReference(referenceFN);
            reference_sln->block_read(readReference);
            readReference.close();
        }

        infoN = n();
        infoNZ = nz();
    }

    void writeSolutionInternal(const std::string &solutionFN = "")
    {
        // TODO: check existence
        std::ofstream writeSln(solutionFN);
        system_sln->block_write(writeSln);
        writeSln.close();
    }

    std::string infoName;
    unsigned int infoNumOfProc;
    double infoTimeReadMatrix;
    double infoTimeSolveSystem;
    double infoTimeTotal;
    unsigned int infoN;
    unsigned int infoNZ;
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
        verboseArg(TCLAP::SwitchArg("v", "verbose", "Verbose mode", false)),
        argc(argc),
        argv(argv)
    {
        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(referenceSolutionArg);
        cmd.add(initialArg);
        cmd.add(verboseArg);
    }

    inline bool hasSolution() { return !solutionArg.getValue().empty(); }
    inline std::string solutionFileName() { return solutionArg.getValue(); }
    inline bool hasReferenceSolution() { return !referenceSolutionArg.getValue().empty(); }

    inline bool isVerbose() { return verboseArg.getValue(); }

    virtual void readLinearSystem()
    {
        // parse the argv array.
        cmd.parse(argc, argv);

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

protected:
    // command line info
    TCLAP::CmdLine cmd;

    TCLAP::ValueArg<std::string> matrixArg;
    TCLAP::ValueArg<std::string> matrixPatternArg;
    TCLAP::ValueArg<std::string> rhsArg;
    TCLAP::ValueArg<std::string> solutionArg;
    TCLAP::ValueArg<std::string> referenceSolutionArg;
    TCLAP::ValueArg<std::string> initialArg;
    TCLAP::SwitchArg verboseArg;

private:
    int argc;
    const char * const *argv;
};

double elapsedSeconds(std::chrono::time_point<std::chrono::steady_clock> start,
                      std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now())
{
    return (end - start).count() * std::chrono::steady_clock::period::num / static_cast<double>(std::chrono::steady_clock::period::den);
}
