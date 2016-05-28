import unittest
import numpy as np
import comma


class test_merge_arrays(unittest.TestCase):
    def test_mismatched_size(self):
        dtype_a = np.dtype([('name', 'u4')])
        dtype_b = np.dtype([('name', 'f8')])
        a = np.array([(1,), (2,)], dtype=dtype_a)
        b = np.array([(4,), (5,), (6,)], dtype=dtype_b)
        self.assertRaises(ValueError, comma.csv.common.merge_arrays, a, b)

    def test_merge_empty(self):
        dtype_a = np.dtype([('name', 'u4')])
        dtype_b = np.dtype([('name', 'f8')])
        a = np.array([], dtype=dtype_a)
        b = np.array([], dtype=dtype_b)
        expected = np.array([], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.common.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_both_single_column(self):
        dtype_a = np.dtype([('name', 'u4')])
        dtype_b = np.dtype([('name', 'f8')])
        a = np.array([(1,), (2,), (3,)], dtype=dtype_a)
        b = np.array([(4,), (5,), (6,)], dtype=dtype_b)
        expected = np.array([((1,), (4,)), ((2,), (5,)), ((3,), (6,))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.common.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_first_single_column(self):
        dtype_a = [('name', 'u4')]
        dtype_b = [('name_1', 'f8'), ('name_2', 'S1')]
        a = np.array([(1,), (2,), (3,)], dtype=dtype_a)
        b = np.array([(4, 'a'), (5, 'b'), (6, 'c')], dtype=dtype_b)
        expected = np.array([((1,), (4, 'a')), ((2,), (5, 'b')), ((3,), (6, 'c'))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.common.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_second_single_column(self):
        dtype_a = [('name', 'u4'), ('name_2', 'S1')]
        dtype_b = [('name_1', 'f8')]
        a = np.array([(1, 'a'), (2, 'b'), (3, 'c')], dtype=dtype_a)
        b = np.array([(4,), (5,), (6,)], dtype=dtype_b)
        expected = np.array([((1, 'a'), (4,)), ((2, 'b'), (5,)), ((3, 'c'), (6,))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.common.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_both_multicolumn(self):
        dtype_a = [('name', 'u4'), ('name_2', 'S1')]
        dtype_b = [('name_1', 'f8'), ('name_2', 'S1')]
        a = np.array([(1, 'a'), (2, 'b'), (3, 'c')], dtype=dtype_a)
        b = np.array([(4, 'x'), (5, 'y'), (6, 'z')], dtype=dtype_b)
        expected = np.array([((1, 'a'), (4, 'x')), ((2, 'b'), (5, 'y')), ((3, 'c'), (6, 'z'))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.common.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)


class test_unrolled_types_of_flat_dtype(unittest.TestCase):
    def test_array_1d(self):
        dtype = np.dtype([('name', '3u4')])
        expected = ('u4', 'u4', 'u4')
        self.assertTupleEqual(comma.csv.common.unrolled_types_of_flat_dtype(dtype), expected)

    def test_array_1d_long(self):
        dtype = np.dtype([('name', '12u4')])
        expected = ('u4',) * 12
        self.assertTupleEqual(comma.csv.common.unrolled_types_of_flat_dtype(dtype), expected)

    def test_array_2d(self):
        dtype = np.dtype([('name', '(2,3)u4')])
        expected = ('u4', 'u4', 'u4', 'u4', 'u4', 'u4')
        self.assertTupleEqual(comma.csv.common.unrolled_types_of_flat_dtype(dtype), expected)

    def test_array_2d_with_other_fields(self):
        dtype = np.dtype([('name1', 'S2'), ('name2', '(2,3)u4'), ('name3', 'f8')])
        expected = ('S2', 'u4', 'u4', 'u4', 'u4', 'u4', 'u4', 'f8')
        self.assertTupleEqual(comma.csv.common.unrolled_types_of_flat_dtype(dtype), expected)

    def test_no_array(self):
        dtype = np.dtype([('name1', 'S2'), ('name2', 'f8')])
        expected = ('S2', 'f8')
        self.assertTupleEqual(comma.csv.common.unrolled_types_of_flat_dtype(dtype), expected)

    def test_time(self):
        dtype = np.dtype([('name1', 'datetime64[us]'), ('name2', 'timedelta64[us]')])
        expected = ('M8[us]', 'm8[us]')
        self.assertTupleEqual(comma.csv.common.unrolled_types_of_flat_dtype(dtype), expected)

    def test_time_array(self):
        dtype = np.dtype([('name', '2datetime64[us]')])
        expected = ('M8[us]', 'M8[us]')
        self.assertTupleEqual(comma.csv.common.unrolled_types_of_flat_dtype(dtype), expected)


class test_types_of_flat_dtype(unittest.TestCase):
    def test_array_2d_with_other_fields(self):
        dtype = np.dtype([('name1', 'S2'), ('name2', '(2,3)u4'), ('name3', 'f8')])
        expected = ('S2', '(2,3)u4', 'f8')
        self.assertTupleEqual(comma.csv.common.types_of_flat_dtype(dtype), expected)

    def test_no_array(self):
        dtype = np.dtype([('name1', 'S2'), ('name2', 'f8')])
        expected = ('S2', 'f8')
        self.assertTupleEqual(comma.csv.common.types_of_flat_dtype(dtype), expected)


class test_structured_dtype(unittest.TestCase):
    def test_multiple_types(self):
        expected = np.dtype('S2,u4,f8')
        self.assertEqual(comma.csv.common.structured_dtype('S2,u4,f8'), expected)

    def test_single_type_string(self):
        expected = np.dtype([('f0', 'S2')])
        self.assertEqual(comma.csv.common.structured_dtype('S2'), expected)

    def test_single_type_time(self):
        expected = np.dtype([('f0', 'datetime64[us]')])
        self.assertEqual(comma.csv.common.structured_dtype('datetime64[us]'), expected)


class test_format_from_types(unittest.TestCase):
    def test_strings(self):
        expected = 'f8,u4,M8[us]'
        self.assertEqual(comma.csv.common.format_from_types(('f8', 'u4', 'datetime64[us]')), expected)

    def test_types(self):
        expected = 'f8,u4'
        self.assertEqual(comma.csv.common.format_from_types((np.float64, np.uint32)), expected)

    def test_arrays(self):
        expected = '(2,3)u4,3f8'
        self.assertEqual(comma.csv.common.format_from_types(('(2,3)u4', '3f8')), expected)

if __name__ == '__main__':
    unittest.main()
