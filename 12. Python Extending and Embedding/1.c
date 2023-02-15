/*inf-IV-05-1
TODO посмотреть условие у Сереги
*/

#include <Python.h>


static PyObject* factor_out(PyObject* self, PyObject* args)
{
  long n;
  if (!PyArg_ParseTuple(args, "l", &n))
    return NULL;
  PyObject* list = PyList_New(0);
  int append_result = PyList_Append(list, PyLong_FromLong(1));
  if (append_result == -1) {
    return NULL;
  }

  for (long i = 2; i <= n; ++i) {
    if (n % i == 0) {
      append_result = PyList_Append(list, PyLong_FromLong(i));
      if (append_result == -1) {
        return NULL;
      }
      n = n / i;
      i--;
    }
  }
  if (PyList_GET_SIZE(list) == 2) {
    return Py_BuildValue("s", "Prime!");
  }
  return list;
}


static PyMethodDef methods[] = {
        {"factor_out", factor_out, METH_VARARGS, "Returns prime divisors"},
        {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC PyInit_primes() {
  static PyModuleDef moduleDef = {
          .m_base = PyModuleDef_HEAD_INIT,
          .m_name = "primes",
          .m_size = -1,
          .m_methods = methods,
  };
  return PyModule_Create(&moduleDef);
}
