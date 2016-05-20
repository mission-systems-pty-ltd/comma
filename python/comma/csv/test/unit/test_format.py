import unittest
import numpy as np
from comma.csv.format import expand, compress, to_numpy, from_numpy
from comma.csv.format import COMMA_TO_NUMPY_TYPE
from comma.csv.format import format_error


class test(unittest.TestCase):
    def test_expand(self):
        f = expand
        self.assertEqual(f(''), '')
        self.assertEqual(f('ub'), 'ub')
        self.assertEqual(f('ub,uw,ui,b,w,i,f,d,s[1],s[10],t'), 'ub,uw,ui,b,w,i,f,d,s[1],s[10],t')
        self.assertEqual(f('2ub,3s[1],4t'), 'ub,ub,s[1],s[1],s[1],t,t,t,t')
        self.assertEqual(f('2d,3d'), 'd,d,d,d,d')
        self.assertEqual(f('11d'), 'd,d,d,d,d,d,d,d,d,d,d')
        self.assertEqual(f('invalid,2invalid'), 'invalid,invalid,invalid')

    def test_compress(self):
        f = compress
        self.assertEqual(f(''), '')
        self.assertEqual(f('ub'), 'ub')
        self.assertEqual(f('ub,uw,ui,b,w,i,f,d,s[1],s[10],t'), 'ub,uw,ui,b,w,i,f,d,s[1],s[10],t')
        self.assertEqual(f('ub,ub,s[1],s[1],s[1],t,t,t,t'), '2ub,3s[1],4t')
        self.assertEqual(f('d,d,d,d,d'), '5d')
        self.assertEqual(f('d,d,d,d,d,d,d,d,d,d,d'), '11d')
        self.assertEqual(f('invalid,invalid,invalid'), '3invalid')

    def test_time_format(self):
        self.assertEqual(COMMA_TO_NUMPY_TYPE['t'], 'M8[us]')

    def test_to_numpy(self):
        f = to_numpy
        self.assertRaises(format_error, f, '')
        self.assertTupleEqual(f('ub'), ('u1',))
        self.assertTupleEqual(f('ub,uw,ui,b,w,i,f,d,s[1],s[10],t'), ('u1', 'u2', 'u4', 'i1', 'i2', 'i4', 'f4', 'f8', 'S1', 'S10', 'M8[us]'))
        self.assertTupleEqual(f('2ub,3s[1],4t'), ('u1', 'u1', 'S1', 'S1', 'S1', 'M8[us]', 'M8[us]', 'M8[us]', 'M8[us]'))
        self.assertTupleEqual(f('2d,3d'), ('f8', 'f8', 'f8', 'f8', 'f8'))
        self.assertTupleEqual(f('11d'), ('f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8'))
        self.assertRaises(format_error, f, 'invalid,2invalid')

    def test_from_numpy(self):
        f = from_numpy
        self.assertRaises(TypeError, f, '')
        self.assertEqual(f('u1'), 'ub')
        self.assertEqual(f('u1', 'u2', 'u4', 'i1', 'i2', 'i4', 'f4', 'f8', 'S1', 'S10', 'datetime64[us]'), 'ub,uw,ui,b,w,i,f,d,s[1],s[10],t')
        self.assertEqual(f('u1', 'u1', 'S1', 'S1', 'S1', 'datetime64[us]', 'datetime64[us]', 'datetime64[us]', 'datetime64[us]'), '2ub,3s[1],4t')
        self.assertEqual(f('f8', 'f8', 'f8', 'f8', 'f8'), '5d')
        self.assertEqual(f('f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8'), '11d')
        self.assertRaises(TypeError, f, 'invalid,2invalid')
        self.assertEqual(f(np.float64, np.uint32), 'd,ui')
        self.assertEqual(f('2f8,(2,3)u1,u4'), '2d,6ub,ui')

if __name__ == '__main__':
    unittest.main()
