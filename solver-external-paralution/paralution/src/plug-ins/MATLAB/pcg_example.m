function pcg_example(dim)

  if nargin < 1
    dim = 100
  end

  n = dim * dim;
  nnz = n*5-dim*4;

  A = sparse(n, n);
  b(1:n) = 1.0;

  for i=0:dim-1
    for j=0:dim-1

      idx = i*dim+j+1;

      if i ~= 0
        A(idx,idx-dim) = -1.0;
      end

      if j ~= 0
        A(idx,idx-1) = -1.0;
      end

      A(idx,idx) = 4.0;

      if j ~= dim-1
        A(idx,idx+1) = -1.0;
      end

      if i ~= dim-1
        A(idx,idx+dim) = -1.0;
      end

    end
  end

 x_ref = pcg(A, b', 1e-6, 500);

 x_paralution = paralution_pcg(A, b', 1e-6, 500);

 err = x_ref - x_paralution;
 disp ('L2 norm |x_matlab - x_paralution| is')
 disp(norm(err));
    
end
