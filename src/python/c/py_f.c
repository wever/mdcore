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

#include "py_f.h"

#define MAX_STR 100

#define min(x, y)  ( (x) < (y) ? x : y )

int
error_to_py(int ierror)
{
  char errstr[ERRSTRLEN];

  if (ierror != ERROR_NONE) {
    get_full_error_string(errstr);
    PyErr_SetString(PyExc_RuntimeError, errstr);
    return 1;
  } else {
    return 0;
  }   
}


void
pystring_to_fstring(PyObject *pystr, char *forstr, int len)
{
  char *str;
  int j;

  str = PyString_AS_STRING(pystr);
  strncpy(forstr, str, len);
  j = strlen(str);
  if (j < len)  memset(forstr+j, ' ', len-j);
}


void
cstring_to_fstring(char *cstr, int clen, char *forstr, int forlen)
{
  int j;

  strncpy(forstr, cstr, min(clen, forlen));
  j = min(strlen(cstr), clen);
  if (j < forlen)  memset(forstr+j, ' ', forlen-j);
}


PyObject*
fstring_to_pystring(char *forstr, int len)
{
  char str[MAX_STR];
  int j;

  strncpy(str, forstr, min(MAX_STR, len));
  j = min(len, strlen(str)-1);
  while (j > 0 && str[j] == ' ')  j--;
  str[j+1] = 0;

  return PyString_FromString(str);
}


int
pydict_to_ptrdict(PyObject *dict, section_t *s)
{
  Py_ssize_t pos;
  PyObject *pkey, *value;

  property_t *p;
  section_t *child;

  char *key;

  BOOL b;
  double d;
  int i, j, k;
  char *str, *str2;
  PyArrayObject *arr;

  PyObject *t;

  char errstr[120];

#ifdef DEBUG
  printf("[pydict_to_ptrdict] %p %p\n", dict, s);
  printf("[pydict_to_ptrdict] size(dict) = %i\n", PyDict_Size(dict));
#endif

  pos = 0;
  while (PyDict_Next(dict, &pos, &pkey, &value)) {
    if (!PyString_Check(pkey)) {
      PyErr_SetString(PyExc_TypeError, "Dictionary key needs to be string.");
      return -1;
    }

    key = PyString_AS_STRING(pkey);

#ifdef DEBUG
    printf("[pydict_to_ptrdict] key = %s\n", key);
#endif

    // Look for property with name *key*
    p = s->first_property;
    while (p != NULL && strcmp(p->name, key)) {
      p = p->next;
    }

    if (p) {
      // Property found

#ifdef DEBUG
      printf("[pydict_to_ptrdict] Property: %s, kind %i\n", p->name, p->kind);
#endif
      
      switch (p->kind) {
      case PK_INT:
	i = PyInt_AsLong(value);
	if (i == -1 && PyErr_Occurred())
	  return -1;
	*((int*) p->ptr) = i;
	break;
      case PK_DOUBLE:
	d = PyFloat_AsDouble(value);
	if (PyErr_Occurred())
	  return -1;
	*((double*) p->ptr) = d;
	break;
      case PK_BOOL:
	if (!PyBool_Check(value))
	  return -1;
	b = value == Py_True;
	*((BOOL*) p->ptr) = b;
	break;
      case PK_STRING:
	if (!PyString_Check(value)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a string.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_ValueError, errstr);
	  return -1;
        }
	str = PyString_AS_STRING(value);
	strncpy((char*) p->ptr, str, p->tag-1);
	break;
      case PK_FORTRAN_STRING:
	if (!PyString_Check(value)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a string.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_ValueError, errstr);
	  return -1;
        }

        pystring_to_fstring(value, (char*) p->ptr, p->tag);
	break;
      case PK_POINT:
	if (!PyTuple_Check(value)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a tuple.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_ValueError, errstr);
	  return -1;
        }
	if (PyTuple_Size(value) != 3) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a 3-tuple of floats.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_ValueError, errstr);
	  return -1;
	}

	for (i = 0; i < 3; i++) {
	  t = PyTuple_GET_ITEM(value, i);

	  if (!PyFloat_Check(t)) {
	    sprintf(errstr,
		    "Property '%s' of section '%s' should be a 3-tuple of "
		    "floats.\n",
		    p->name, p->parent->name);
	    PyErr_SetString(PyExc_ValueError, errstr);
	    return -1;
	  }

          ((double *) p->ptr)[i] = PyFloat_AS_DOUBLE(t);
	}

	break;
      case PK_INTPOINT:
	if (!PyTuple_Check(value)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a 3-tuple of "
		  "integers.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}
	if (PyTuple_Size(value) != 3) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a 3-tuple of "
		  "integers.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_ValueError, errstr);
	  return -1;
	}

	for (i = 0; i < 3; i++) {
	  t = PyTuple_GET_ITEM(value, i);

	  if (!PyInt_Check(t)) {
	    sprintf(errstr,
		    "Property '%s' of section '%s' should be a 3-tuple of "
		    "integers.\n",
		    p->name, p->parent->name);
	    PyErr_SetString(PyExc_ValueError, errstr);
	    return -1;
	  }

          ((int *) p->ptr)[i] = PyInt_AS_LONG(t);
	}
	
	break;
      case PK_LIST:
        if (PyFloat_Check(value)) {
          *p->tag5 = 1;
          *((double*) p->ptr) = PyFloat_AS_DOUBLE(value);
        } else {
          PyArray_Converter(value, (PyObject**) &arr);

          if (!PyArray_ISFLOAT(arr)) {
	    sprintf(errstr,
		    "Property '%s' of section '%s' should be a list of "
		    "floats.\n",
		    p->name, p->parent->name);
	    PyErr_SetString(PyExc_TypeError, errstr);
            return -1;
	  }

          if (arr->nd == 1) {
            *p->tag5 = PyArray_DIM(arr, 0);
          } else {
            Py_DECREF(arr);
            PyErr_SetString(PyExc_TypeError, "Array needs to be scalar or "
			    "one-dimensional.");
            return -1;
          }

	  /* Type conversion madness */
          switch (arr->descr->type_num) {
  	  case NPY_FLOAT:
	    for (i = 0; i < *p->tag5; i++) {
	      ((double *) p->ptr)[i] = ((npy_float *) PyArray_DATA(arr))[i];
	    }
	    break;
  	  case NPY_DOUBLE:
	    for (i = 0; i < *p->tag5; i++) {
	      ((double *) p->ptr)[i] = ((npy_double *) PyArray_DATA(arr))[i];
	    }
	    break;
	  default:
	    PyErr_SetString(PyExc_TypeError, "Don't know how to convert from "
			    "numpy float type.");
	    return -1;
	  }

          Py_DECREF(arr);
        }

	break;
      case PK_INT_LIST:
        if (PyInt_Check(value)) {
          *p->tag5 = 1;
          *((int*) p->ptr) = PyInt_AS_LONG(value);
        } else {
          PyArray_Converter(value, (PyObject**) &arr);

          if (!PyArray_ISINTEGER(arr)) {
	    sprintf(errstr,
		    "Property '%s' of section '%s' should be a list of "
		    "integers.\n",
		    p->name, p->parent->name);
	    PyErr_SetString(PyExc_TypeError, errstr);
            return -1;
	  }

          if (arr->nd == 1) {
            *p->tag5 = PyArray_DIM(arr, 0);
          } else {
            Py_DECREF(arr);
            PyErr_SetString(PyExc_TypeError, "Array needs to be scalar or "
			    "one-dimensional.");
            return -1;
          }

	  /* Type conversion madness */
          switch (arr->descr->type_num) {
	  case NPY_INT:
	    for (i = 0; i < *p->tag5; i++) {
	      ((int *) p->ptr)[i] = ((npy_int*) PyArray_DATA(arr))[i];
	    }
	    break;
	  case NPY_LONG:
	    for (i = 0; i < *p->tag5; i++) {
	      ((int *) p->ptr)[i] = ((npy_long*) PyArray_DATA(arr))[i];
	    }
	    break;
	  default:
	    PyErr_SetString(PyExc_TypeError, "Don't know how to convert from "
			    "numpy integer type.");
	    return -1;
	  }

          Py_DECREF(arr);
        }

	break;
      case PK_FORTRAN_STRING_LIST:
#ifdef DEBUG
        printf("PK_FORTRAN_STRING_LIST, key = %s\n", key);
#endif
        if (PyString_Check(value)) {
          *p->tag5 = 1;
          pystring_to_fstring(value, (char*) p->ptr, p->tag);
        } else {
          PyArray_Converter(value, (PyObject**) &arr);

          if (!PyArray_ISSTRING(arr)) {
	    sprintf(errstr,
		    "Property '%s' of section '%s' should be a list of "
		    "strings.\n",
		    p->name, p->parent->name);
	    PyErr_SetString(PyExc_TypeError, errstr);
            return -1;
	  }

          if (arr->nd == 1) {
            *p->tag5 = PyArray_DIM(arr, 0);
          } else {
            Py_DECREF(arr);
            PyErr_SetString(PyExc_TypeError, "Array needs to be scalar or "
			    "one-dimensional.");
            return -1;
          }

          k = PyArray_STRIDE(arr, 0);
          str2 = (char *) p->ptr;
          for (i = 0; i < *p->tag5; i++) {
            str = PyArray_GETPTR1(arr, i);
            cstring_to_fstring(str, k, str2, p->tag);
            str2 += p->tag;
          }

          Py_DECREF(arr);
        }

	break;
      case PK_ARRAY2D:
	if (!PyArray_Check(value)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a 2d array "
		  "of floats.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}
	arr = (PyArrayObject *) value;
	if (!PyArray_ISFLOAT(arr)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a 2d array "
		  "of floats.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}
	if (arr->nd != 2) {
	  PyErr_SetString(PyExc_TypeError, "Array needs to be 2-dimensional.");
	  return -1;
	}
	if (PyArray_DIM(arr, 0) != p->tag || PyArray_DIM(arr, 1) != p->tag2) {
	  sprintf(errstr, "Wrong dimensions: Array needs to be %ix%i.", p->tag, p->tag2);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}

	/* Type conversion madness */
	switch (arr->descr->type_num) {
	case NPY_FLOAT:
	  for (i = 0; i < p->tag; i++) {
	    for (j = 0; j < p->tag2; j++) {
	      ((double*) p->ptr)[i + j*p->tag] = ((npy_float *) PyArray_DATA(arr))[j + i*p->tag2];
	    }
	  }
	  break;
	case NPY_DOUBLE:
	  for (i = 0; i < p->tag; i++) {
	    for (j = 0; j < p->tag2; j++) {
	      ((double*) p->ptr)[i + j*p->tag] = ((npy_double *) PyArray_DATA(arr))[j + i*p->tag2];
	    }
	  }
	  break;
	default:
	  PyErr_SetString(PyExc_TypeError, "Don't know how to convert from "
			  "numpy float type.");
	  return -1;
	}
	break;
      case PK_ARRAY3D:
	if (!PyArray_Check(value)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a 3d array "
		  "of floats.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}
	arr = (PyArrayObject *) value;
	if (!PyArray_ISFLOAT(arr)) {
	  sprintf(errstr,
		  "Property '%s' of section '%s' should be a 3d array "
		  "of floats.\n",
		  p->name, p->parent->name);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}
	if (arr->nd != 3) {
	  PyErr_SetString(PyExc_TypeError, "Array needs to be 3-dimensional.");
	  return -1;
	}
	if (PyArray_DIM(arr, 0) != p->tag || PyArray_DIM(arr, 1) != p->tag2 ||
	    PyArray_DIM(arr, 2) != p->tag3) {
	  sprintf(errstr, "Wrong dimensions: Array needs to be %ix%ix%i.",
		  p->tag, p->tag2, p->tag3);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}

	/* Type conversion madness */
	switch (arr->descr->type_num) {
	case NPY_FLOAT:
	  for (i = 0; i < p->tag; i++) {
	    for (j = 0; j < p->tag2; j++) {
	      for (k = 0; k < p->tag3; k++) {
		((double*) p->ptr)[i + (j + k*p->tag2)*p->tag] = 
		  ((npy_float *) PyArray_DATA(arr))[k + (j + i*p->tag2)*p->tag3];
	      }
	    }
	  }
	  break;
	case NPY_DOUBLE:
	  for (i = 0; i < p->tag; i++) {
	    for (j = 0; j < p->tag2; j++) {
	      for (k = 0; k < p->tag3; k++) {
		((double*) p->ptr)[i + (j + k*p->tag2)*p->tag] = 
		  ((npy_double *) PyArray_DATA(arr))[k + (j + i*p->tag2)*p->tag3];
	      }
	    }
	  }
	  break;
	default:
	  PyErr_SetString(PyExc_TypeError, "Don't know how to convert from "
			  "numpy float type.");
	  return -1;
	}
	//      memcpy(p->ptr, data, p->tag*p->tag2*p->tag3*sizeof(double));
	break;
      default:
	sprintf(errstr, "Internal error: Unknown type with id %i encountered in section.", p->kind);
	PyErr_SetString(PyExc_TypeError, errstr);
	return -1;
	break;
      }
    } else {
#ifdef DEBUG
      printf("[pydict_to_ptrdict] Property '%s' not found.\n", key);
#endif

      // No property found, check if there is a section with that name
      child = s->first_child;

      while (child != NULL && strcmp(child->name, key)) {
	child = child->next;
      }

      if (child) {

#ifdef DEBUG
        printf("[pydict_to_ptrdict] Section '%s' found.\n", key);
#endif

	// Value should be a dictionary
	if (!PyDict_Check(value))
	  return -1;

#ifdef DEBUG
	printf("[pydict_to_ptrdict] Child: %s\n", child->name);
#endif

	child->provided = TRUE;
	if (child->provided_notification)
	  *child->provided_notification = TRUE;
      
	pydict_to_ptrdict(value, child);

      } else {

	/* Ignore this property if it starts with '__' */
	if (key[0] != '_' || key[1] != '_') {
	  sprintf(errstr, "Could not find property '%s' of section '%s'.",
		  key, s->name);
	  PyErr_SetString(PyExc_TypeError, errstr);
	  return -1;
	}
      }
    }

  }

  return 0;
}
