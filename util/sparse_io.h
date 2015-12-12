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
