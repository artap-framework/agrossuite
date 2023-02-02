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

#include "plugin_solver_interface.h"

PluginSolverInterface::PluginSolverInterface()
{

}

PluginSolverInterface::~PluginSolverInterface()
{

}

QStringList PluginSolverInterface::methods() const
{
    QStringList list;
    list.append("none");

    return list;
}

void PluginSolverInterface::prepare_crs(const dealii::SparseMatrix<double> &matrix)
{
    const int N = matrix.m();

    // copy over the data from the matrix to the data structures UMFPACK
    // wants. note two things: first, UMFPACK wants compressed column storage
    // whereas we always do compressed row storage; we work around this by,
    // rather than shuffling things around, copy over the data we have, but
    // then call the umfpack_dl_solve function with the UMFPACK_At argument,
    // meaning that we want to solve for the transpose system
    //
    // second: the data we have in the sparse matrices is "almost" right
    // already; UMFPACK wants the entries in each row (i.e. really: column)
    // to be sorted in ascending order. we almost have that, except that we
    // usually store the diagonal first in each row to allow for some
    // optimizations. thus, we have to resort things a little bit, but only
    // within each row
    //
    // final note: if the matrix has entries in the sparsity pattern that are
    // actually occupied by entries that have a zero numerical value, then we
    // keep them anyway. people are supposed to provide accurate sparsity
    // patterns.
    Ap.resize(N + 1);
    Ai.resize(matrix.n_nonzero_elements());
    Ax.resize(matrix.n_nonzero_elements());

    // first fill row lengths array
    Ap[0] = 0;
    for (int row = 1; row <= N; ++row)
        Ap[row] = Ap[row - 1] + matrix.get_row_length(row - 1);
    Assert(static_cast<int>(Ap.back()) == Ai.size(), ExcInternalError());

    // then copy over matrix elements. note that for sparse matrices,
    // iterators are sorted so that they traverse each row from start to end
    // before moving on to the next row. however, this isn't true for block
    // matrices, so we have to do a bit of book keeping
    {
        // have an array that for each row points to the first entry not yet
        // written to
        std::vector<int> row_pointers = Ap;

        // loop over the elements of the matrix row by row, as suggested in the
        // documentation of the sparse matrix iterator class
        for (int row = 0; row < matrix.m(); ++row)
        {
            for (typename dealii::SparseMatrix<double>::const_iterator p = matrix.begin(row);
                 p != matrix.end(row);
                 ++p)
            {
                // write entry into the first free one for this row
                Ai[row_pointers[row]] = p->column();
                Ax[row_pointers[row]] = std::real(p->value());

                // then move pointer ahead
                ++row_pointers[row];
            }
        }

        // at the end, we should have written all rows completely
        for (int i = 0; i < Ap.size() - 1; ++i)
            Assert(row_pointers[i] == Ap[i + 1], ExcInternalError());
    }

    // make sure that the elements in each row are sorted. we have to be more
    // careful for block sparse matrices, so ship this task out to a
    // different function
    sort_arrays(matrix);
}

void PluginSolverInterface::sort_arrays(const dealii::SparseMatrix<double> &matrix)
{
    // do the copying around of entries so that the diagonal entry is in the
    // right place. note that this is easy to detect: since all entries apart
    // from the diagonal entry are sorted, we know that the diagonal entry is
    // in the wrong place if and only if its column index is larger than the
    // column index of the second entry in a row
    //
    // ignore rows with only one or no entry
    for (int row = 0; row < matrix.m(); ++row)
    {
        // we may have to move some elements that are left of the diagonal
        // but presently after the diagonal entry to the left, whereas the
        // diagonal entry has to move to the right. we could first figure out
        // where to move everything to, but for simplicity we just make a
        // series of swaps instead (this is kind of a single run of
        // bubble-sort, which gives us the desired result since the array is
        // already "almost" sorted)
        //
        // in the first loop, the condition in the while-header also checks
        // that the row has at least two entries and that the diagonal entry
        // is really in the wrong place
        int cursor = Ap[row];
        while ((cursor < Ap[row + 1] - 1) && (Ai[cursor] > Ai[cursor + 1]))
        {
            std::swap(Ai[cursor], Ai[cursor + 1]);
            std::swap(Ax[cursor], Ax[cursor + 1]);

            ++cursor;
        }
    }
}
