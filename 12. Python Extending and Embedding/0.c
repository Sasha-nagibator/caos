/*inf-IV-05-0
TODO посмотреть условие у Сереги
*/
#include <Python.h>

static PyObject* dot(PyObject *self, PyObject *args) {
  int size = 0;
  PyObject* list1 = NULL;
  PyObject* list2 = NULL;

  if (!PyArg_ParseTuple(args, "IOO", &size, &list1, &list2)) {
    return NULL;
  }

  double m1[size*size];
  double m2[size*size];
  double res_m[size*size];

  for (int i = 0; i < size; ++i) {
    PyObject* sublist1 = PyList_GET_ITEM(list1, i);
    PyObject* sublist2 = PyList_GET_ITEM(list2, i);

    for (int j = 0; j < size; ++j) {
      m1[i*size + j] = PyFloat_AsDouble(PyList_GET_ITEM(sublist1, j));
      m2[j*size + i] = PyFloat_AsDouble(PyList_GET_ITEM(sublist2, j));
    }
  }

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      res_m[i*size + j] = 0;
      for (int k = 0; k < size; ++k) {
        res_m[i*size + j] += m1[i*size + k] * m2[j*size + k];
      }
    }
  }

  PyObject* res_list = PyList_New(0);
  for (int i = 0; i < size; ++i) {
    PyObject* row = PyList_New(0);
    for (int j = 0; j < size; ++j) {
      PyList_Append(row, PyFloat_FromDouble(res_m[i*size + j]));
    }
    PyList_Append(res_list, row);
  }
  return res_list;
}


static PyMethodDef methods[] = {
        {
                .ml_name = "dot",
                .ml_meth = dot,
                // флаги использования Си-функции
                .ml_flags = METH_VARARGS,
                .ml_doc = "Args: (size, matrix, matrix)"
        },
        {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC PyInit_matrix() {
  static PyModuleDef moduleDef = {
          .m_base = PyModuleDef_HEAD_INIT,
          .m_name = "matrix",
          .m_size = -1,
          .m_methods = methods,
  };
  return PyModule_Create(&moduleDef);
}
