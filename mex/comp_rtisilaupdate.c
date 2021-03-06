/** \addtogroup mex
 *  \{
 *  \file
 *  \brief MEX file interface for rtisilaupdate()
 *
 *  **This is a computaional routine. The input arguments are not checked for correctness!!**
 *
 *  Matlab calling convention:
 *  --------------------------
 *
 *      [frames2,c] = comp_rtisilaupdate(frames,g,scpecg1,specg2,gd,a,M,s,lookahead,maxit)
 *
 *  Input arg.   | Description
 *  ------------ | -------------------------------------------------------------
 *  frames       | M x N real matrix, frame buffer, N = lookback + 1 + lookahead
 *  g            | M el. vector, analysis window
 *  specg1       | M el. vector, alternative analysis window for the first iteration of the newest lookahead frame.
 *  specg2       | M el. vector, alternative analyis window for second and other iterations of the newest lookahead frame.
 *  gd           | M el. vector, synthesis window
 *  a            | Hop factor
 *  M            | Number of channels (FFT length)
 *  s            | M2 x lookahead+1 matrix
 *
 *  Output arg.  | Description
 *  -----------  | -------------------------------------------------------------
 *  frames2      | frames with updated last lookahead + 1 cols.
 *  c            | Complex coefficients of the submit frame.
 *
 *  \author Zdeněk Průša
 *  \date 17.02.2016
 *  \}
 */

#include "mex_helper.h"
#define LTFAT_DOUBLE
#include "phaseret/rtisila.h"

void
mexFunction(int nlhs, mxArray* plhs[],
            int nrhs, const mxArray* prhs[])
{
    UNUSED(nrhs);
    const mxArray* mxcframes = prhs[0];

    int N = mxGetDimensions(mxcframes)[1];
    int W = 1;

    if (mxGetNumberOfDimensions(mxcframes) > 2)
        W = mxGetDimensions(mxcframes)[2];

    const double* cframes = mxGetData(mxcframes);
    const double* gnum = mxGetData(prhs[1]);
    const double* specg1 = mxGetData(prhs[2]);
    const double* specg2 = mxGetData(prhs[3]);
    const double* dgnum = mxGetData(prhs[4]);
    int a = (int) mxGetScalar(prhs[5]);
    int M = (int) mxGetScalar(prhs[6]);
    const double* sframes = mxGetData(prhs[7]);
    int lookahead = (int) mxGetScalar(prhs[8]);
    int maxit = (int) mxGetScalar(prhs[9]);
    int M2 = M / 2 + 1;
    int gl = mxGetM(prhs[1]);

    plhs[0] = mxDuplicateArray(mxcframes);
    double* cframes2 = mxGetData(plhs[0]);

    double* cr = NULL;
    double* ci = NULL;
    complex double* cout = NULL;

    if (nlhs > 1)
    {
        plhs[1] = mxCreateDoubleMatrix(M2, W, mxCOMPLEX);
        cr = mxGetData(plhs[1]);
        ci = mxGetImagData(plhs[1]);
        cout = mxMalloc(M2 * sizeof * cout);
    }

    phaseret_rtisilaupdate_plan_d* p = NULL;
    phaseret_rtisilaupdate_init_d(gnum, specg1, specg2, dgnum, gl, a, M, &p);

    for (int w = 0; w < W; w++)
    {
        phaseret_rtisilaupdate_execute_d(p, cframes + w * M * N, N,
                                         sframes + w * M2 * N,
                                         lookahead, maxit, cframes2 + w * M * N,
                                         cout);

        if (nlhs > 1)
            complex2split(cout, M2,  cr + w * M2, ci + w * M2);
    }

    if (nlhs > 1)
        mxFree(cout);


    phaseret_rtisilaupdate_done_d(&p);
}
