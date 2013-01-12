/* ======================================================================
   MDCORE - Interatomic potential library
   https://github.com/pastewka/mdcore
   Lars Pastewka, lars.pastewka@iwm.fraunhofer.de, and others
   See the AUTHORS file in the top-level MDCORE directory.

   Copyright (2005-2013) Fraunhofer IWM
   This software is distributed under the GNU General Public License.
   See the LICENSE file in the top-level MDCORE directory.
   ====================================================================== */
#include <Python.h>
#define PY_ARRAY_UNIQUE_SYMBOL MDCORE_ARRAY_API
#define NO_IMPORT_ARRAY
#include <numpy/arrayobject.h>

#include "mdcoremodule.h"


PyObject *
py_pair_distribution(PyObject *self, PyObject *args)
{
  PyObject *i_arr, *r_arr;
  int nbins;
  double cutoff;

  if (!PyArg_ParseTuple(args, "O!O!id", &PyArray_Type, &i_arr,
			&PyArray_Type, &r_arr, &nbins, &cutoff))
    return NULL;

  if (PyArray_NDIM(i_arr) != 1 || PyArray_TYPE(i_arr) != NPY_INT) {
    PyErr_SetString(PyExc_TypeError, "First argument needs to be "
                    "one-dimensional integer array.");
    return NULL;
  }
  if (PyArray_NDIM(r_arr) != 1 || PyArray_TYPE(r_arr) != NPY_DOUBLE) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be "
                    "one-dimensional double array.");
    return NULL;
  }

  npy_intp npairs = PyArray_DIM(i_arr, 0);
  if (PyArray_DIM(r_arr, 0) != npairs) {
    PyErr_SetString(PyExc_RuntimeError, "First two arguments need to be arrays "
                    "of identical length.");
    return NULL;
  }

  npy_intp dim = nbins;
  PyObject *h_arr = PyArray_ZEROS(1, &dim, NPY_DOUBLE, 1);
  PyObject *h2_arr = PyArray_ZEROS(1, &dim, NPY_DOUBLE, 1);
  PyObject *tmp_arr = PyArray_ZEROS(1, &dim, NPY_INT, 1);

  npy_int *i = PyArray_DATA(i_arr);
  double *r = PyArray_DATA(r_arr);
  double *h = PyArray_DATA(h_arr);
  double *h2 = PyArray_DATA(h2_arr);
  npy_int *tmp = PyArray_DATA(tmp_arr);

  npy_int last_i = i[0];
  memset(tmp, 0, nbins*sizeof(npy_int));
  int nat = 1, p;
  for (p = 0; p < npairs; p++) {
    if (last_i != i[p]) {
      int bin;
      for (bin = 0; bin < nbins; bin++) {
	h[bin] += tmp[bin];
	h2[bin] += tmp[bin]*tmp[bin];
      }
      memset(tmp, 0, nbins*sizeof(npy_int));
      last_i = i[p];
      nat++;
    }

    int bin = (int) (nbins*r[p]/cutoff);
    if (bin >= 0 && bin < nbins) {
      tmp[bin]++;
    }
  }
  int bin;
  for (bin = 0; bin < nbins; bin++) {
    h[bin] += tmp[bin];
    h2[bin] += tmp[bin]*tmp[bin];

    double r1 = bin*cutoff/nbins, r2 = (bin+1)*cutoff/nbins;
    double binvol = 4*M_PI/3*(r2*r2*r2 - r1*r1*r1);

    h[bin] /= nat*binvol;
    h2[bin] /= nat*binvol*binvol;
    h2[bin] -= h[bin]*h[bin];
  }

  Py_DECREF(tmp_arr);

  return Py_BuildValue("OO", h_arr, h2_arr);
}


PyObject *
py_angle_distribution(PyObject *self, PyObject *args)
{
  PyObject *i_arr, *j_arr, *r_arr;
  int nbins;
  double cutoff;

  if (!PyArg_ParseTuple(args, "O!O!O!id", &PyArray_Type, &i_arr, &PyArray_Type,
                        &j_arr, &PyArray_Type, &r_arr, &nbins, &cutoff))
    return NULL;

  if (PyArray_NDIM(i_arr) != 1 || PyArray_TYPE(i_arr) != NPY_INT) {
    PyErr_SetString(PyExc_TypeError, "First argument needs to be one-dimensional "
                    "integer array.");
    return NULL;
  }
  if (PyArray_NDIM(j_arr) != 1 || PyArray_TYPE(j_arr) != NPY_INT) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be one-dimensional "
                    "integer array.");
    return NULL;
  }
  if (PyArray_NDIM(r_arr) != 2 || PyArray_DIM(r_arr, 1) != 3 ||
      PyArray_TYPE(r_arr) != NPY_DOUBLE) {
    PyErr_SetString(PyExc_TypeError, "Second argument needs to be two-dimensional "
                    "double array.");
    return NULL;
  }

  npy_intp npairs = PyArray_DIM(i_arr, 0);
  if (PyArray_DIM(j_arr, 0) != npairs || PyArray_DIM(r_arr, 0) != npairs) {
    PyErr_SetString(PyExc_RuntimeError, "First three arguments need to be arrays of "
                    "identical length.");
    return NULL;
  }

  npy_intp dim = nbins;
  PyObject *h_arr = PyArray_ZEROS(1, &dim, NPY_DOUBLE, 1);
  PyObject *h2_arr = PyArray_ZEROS(1, &dim, NPY_DOUBLE, 1);
  PyObject *tmp_arr = PyArray_ZEROS(1, &dim, NPY_INT, 1);

  npy_int *i = PyArray_DATA(i_arr);
  npy_int *j = PyArray_DATA(j_arr);
  double *r = PyArray_DATA(r_arr);
  double *h = PyArray_DATA(h_arr);
  double *h2 = PyArray_DATA(h2_arr);
  npy_int *tmp = PyArray_DATA(tmp_arr);

  npy_int last_i = i[0], i_start = 0;
  memset(tmp, 0, nbins*sizeof(npy_int));
  int nangle = 1, p;
  double cutoff_sq = cutoff*cutoff;
  for (p = 0; p < npairs; p++) {
    if (last_i != i[p]) {
      int bin;
      for (bin = 0; bin < nbins; bin++) {
        h[bin] += tmp[bin];
        h2[bin] += tmp[bin]*tmp[bin];
      }
      memset(tmp, 0, nbins*sizeof(npy_int));
      last_i = i[p];
      i_start = p;
    }

    double n = r[3*p]*r[3*p] + r[3*p+1]*r[3*p+1] + r[3*p+2]*r[3*p+2];

    if (n < cutoff_sq) {
      int p2;
      for (p2 = i_start; i[p2] == last_i; p2++) {
        if (p2 != p) {
          double n2 = r[3*p2]*r[3*p2] + r[3*p2+1]*r[3*p2+1] + r[3*p2+2]*r[3*p2+2];
          if (n2 < cutoff_sq) {
            double angle = r[3*p]*r[3*p2] + r[3*p+1]*r[3*p2+1] + r[3*p+2]*r[3*p2+2];
            angle = acos(angle/sqrt(n*n2));
            int bin = (int) (nbins*angle/M_PI);
            while (bin < 0)  bin += nbins;
            while (bin >= nbins)  bin -= nbins;
            tmp[bin]++;
            nangle++;
          } /* n2 < cutoff_sq */
        } /* p!= p */
      }
    } /* n < cutoff_sq */
  }
  double binvol = M_PI/nbins;
  int bin;
  for (bin = 0; bin < nbins; bin++) {
    h[bin] += tmp[bin];
    h2[bin] += tmp[bin]*tmp[bin];

    h[bin] /= nangle*binvol;
    h2[bin] /= nangle*binvol*binvol;
    h2[bin] -= h[bin]*h[bin];
  }

  Py_DECREF(tmp_arr);

  return Py_BuildValue("OO", h_arr, h2_arr);
}
