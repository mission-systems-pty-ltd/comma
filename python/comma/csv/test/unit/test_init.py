import unittest
import comma
import numpy as np
import sys

data_in = 'data/in'
data_out = 'data/out'


class test_merge_arrays(unittest.TestCase):
    def test_mismatched_size(self):
        dtype_a = np.dtype([('name', 'u4')])
        dtype_b = np.dtype([('name', 'f8')])
        a = np.array([(1,), (2,)], dtype=dtype_a)
        b = np.array([(4,), (5,), (6,)], dtype=dtype_b)
        self.assertRaises(Exception, comma.csv.merge_arrays, a, b)

    def test_merge_empty(self):
        dtype_a = np.dtype([('name', 'u4')])
        dtype_b = np.dtype([('name', 'f8')])
        a = np.array([], dtype=dtype_a)
        b = np.array([], dtype=dtype_b)
        expected = np.array([], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_both_single_column(self):
        dtype_a = np.dtype([('name', 'u4')])
        dtype_b = np.dtype([('name', 'f8')])
        a = np.array([(1,), (2,), (3,)], dtype=dtype_a)
        b = np.array([(4,), (5,), (6,)], dtype=dtype_b)
        expected = np.array([((1,), (4,)), ((2,), (5,)), ((3,), (6,))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_first_single_column(self):
        dtype_a = [('name', 'u4')]
        dtype_b = [('name_1', 'f8'), ('name_2', 'S1')]
        a = np.array([(1,), (2,), (3,)], dtype=dtype_a)
        b = np.array([(4, 'a'), (5, 'b'), (6, 'c')], dtype=dtype_b)
        expected = np.array([((1,), (4, 'a')), ((2,), (5, 'b')), ((3,), (6, 'c'))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_second_single_column(self):
        dtype_a = [('name', 'u4'), ('name_2', 'S1')]
        dtype_b = [('name_1', 'f8')]
        a = np.array([(1, 'a'), (2, 'b'), (3, 'c')], dtype=dtype_a)
        b = np.array([(4,), (5,), (6,)], dtype=dtype_b)
        expected = np.array([((1, 'a'), (4,)), ((2, 'b'), (5,)), ((3, 'c'), (6,))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)

    def test_both_multicolumn(self):
        dtype_a = [('name', 'u4'), ('name_2', 'S1')]
        dtype_b = [('name_1', 'f8'), ('name_2', 'S1')]
        a = np.array([(1, 'a'), (2, 'b'), (3, 'c')], dtype=dtype_a)
        b = np.array([(4, 'x'), (5, 'y'), (6, 'z')], dtype=dtype_b)
        expected = np.array([((1, 'a'), (4, 'x')), ((2, 'b'), (5, 'y')), ((3, 'c'), (6, 'z'))], dtype=[('first', dtype_a), ('second', dtype_b)])
        c = comma.csv.merge_arrays(a, b)
        np.testing.assert_equal(c, expected)


class test_unrolled_types_of_flat_dtype(unittest.TestCase):
    def test_array_1d(self):
        dtype = np.dtype([('name', '3u4')])
        expected = ('u4', 'u4', 'u4')
        self.assertTupleEqual(comma.csv.unrolled_types_of_flat_dtype(dtype), expected)

    def test_array_1d_long(self):
        dtype = np.dtype([('name', '12u4')])
        expected = ('u4',) * 12
        self.assertTupleEqual(comma.csv.unrolled_types_of_flat_dtype(dtype), expected)

    def test_array_2d(self):
        dtype = np.dtype([('name', '(2,3)u4')])
        expected = ('u4', 'u4', 'u4', 'u4', 'u4', 'u4')
        self.assertTupleEqual(comma.csv.unrolled_types_of_flat_dtype(dtype), expected)

    def test_array_2d_with_other_fields(self):
        dtype = np.dtype([('name1', 'S2'), ('name2', '(2,3)u4'), ('name3', 'f8')])
        expected = ('S2', 'u4', 'u4', 'u4', 'u4', 'u4', 'u4', 'f8')
        self.assertTupleEqual(comma.csv.unrolled_types_of_flat_dtype(dtype), expected)

    def test_no_array(self):
        dtype = np.dtype([('name1', 'S2'), ('name2', 'f8')])
        expected = ('S2', 'f8')
        self.assertTupleEqual(comma.csv.unrolled_types_of_flat_dtype(dtype), expected)

    def test_time(self):
        dtype = np.dtype([('name1', 'datetime64[us]'), ('name2', 'timedelta64[us]')])
        expected = ('M8[us]', 'm8[us]')
        self.assertTupleEqual(comma.csv.unrolled_types_of_flat_dtype(dtype), expected)

    def test_time_array(self):
        dtype = np.dtype([('name', '2datetime64[us]')])
        expected = ('M8[us]', 'M8[us]')
        self.assertTupleEqual(comma.csv.unrolled_types_of_flat_dtype(dtype), expected)


class test_structured_dtype(unittest.TestCase):
    def test_multiple_types(self):
        expected = np.dtype('S2,u4,f8')
        self.assertEqual(comma.csv.structured_dtype('S2,u4,f8'), expected)

    def test_single_type_string(self):
        expected = np.dtype([('f0', 'S2')])
        self.assertEqual(comma.csv.structured_dtype('S2'), expected)

    def test_single_type_time(self):
        expected = np.dtype([('f0', 'datetime64[us]')])
        self.assertEqual(comma.csv.structured_dtype('datetime64[us]'), expected)


class test_format_from_types(unittest.TestCase):
    def test_strings(self):
        expected = 'f8,u4,datetime64[us]'
        self.assertEqual(comma.csv.format_from_types(('f8', 'u4', 'datetime64[us]')), expected)

    def test_types(self):
        expected = 'f8,u4'
        self.assertEqual(comma.csv.format_from_types((np.float64, np.uint32)), expected)

    def test_arrays(self):
        expected = '(2,3)u4,3f8'
        self.assertEqual(comma.csv.format_from_types(('(2,3)u4', '3f8')), expected)


class test_struct(unittest.TestCase):
    def test_single_field(self):
        s = comma.csv.struct('name', 'u4')
        self.assertTupleEqual(s.fields, ('name',))
        self.assertTupleEqual(s.types, ('u4',))
        self.assertEqual(s.format, 'u4')
        self.assertSetEqual(s.ambiguous_leaves, set())
        self.assertDictEqual(s.xpath_of_leaf, dict(name='name'))
        self.assertDictEqual(s.shorthand, dict())
        self.assertDictEqual(s.type_of_field, dict(name='u4'))
        self.assertEqual(s.dtype, s.flat_dtype)
        self.assertEqual(s.flat_dtype, np.dtype([('name', 'u4')]))
        self.assertEqual(s.unrolled_flat_dtype, np.dtype([('f0', 'u4')]))

    def test_multiple_fields(self):
        s = comma.csv.struct('id,x', 'u4', 'f8')
        self.assertTupleEqual(s.fields, ('id', 'x'))
        self.assertTupleEqual(s.types, ('u4', 'f8'))
        self.assertEqual(s.format, 'u4,f8')
        self.assertSetEqual(s.ambiguous_leaves, set())
        self.assertDictEqual(s.xpath_of_leaf, dict(id='id', x='x'))
        self.assertDictEqual(s.shorthand, dict())
        self.assertDictEqual(s.type_of_field, dict(id='u4', x='f8'))
        self.assertEqual(s.dtype, s.flat_dtype)
        self.assertEqual(s.flat_dtype, np.dtype([('id', 'u4'), ('x', 'f8')]))
        self.assertEqual(s.unrolled_flat_dtype, np.dtype([('f0', 'u4'), ('f1', 'f8')]))

    def test_array_1d(self):
        s = comma.csv.struct('id,point', 'u4', '3f8')
        self.assertTupleEqual(s.fields, ('id', 'point'))
        self.assertTupleEqual(s.types, ('u4', '3f8'))
        self.assertEqual(s.format, 'u4,3f8')
        self.assertSetEqual(s.ambiguous_leaves, set())
        self.assertDictEqual(s.xpath_of_leaf, dict(id='id', point='point'))
        self.assertDictEqual(s.shorthand, dict())
        self.assertDictEqual(s.type_of_field, dict(id='u4', point='3f8'))
        self.assertEqual(s.dtype, s.flat_dtype)
        self.assertEqual(s.flat_dtype, np.dtype([('id', 'u4'), ('point', '3f8')]))
        self.assertEqual(s.unrolled_flat_dtype, np.dtype([('f0', 'u4'), ('f1', 'f8'), ('f2', 'f8'), ('f3', 'f8')]))

    def test_array_2d(self):
        s = comma.csv.struct('id,data', 'u4', '(2,3)f8')
        self.assertTupleEqual(s.fields, ('id', 'data'))
        self.assertTupleEqual(s.types, ('u4', '(2,3)f8'))
        self.assertEqual(s.format, 'u4,(2,3)f8')
        self.assertSetEqual(s.ambiguous_leaves, set())
        self.assertDictEqual(s.xpath_of_leaf, dict(id='id', data='data'))
        self.assertDictEqual(s.shorthand, dict())
        self.assertDictEqual(s.type_of_field, dict(id='u4', data='(2,3)f8'))
        self.assertEqual(s.dtype, s.flat_dtype)
        self.assertEqual(s.flat_dtype, np.dtype([('id', 'u4'), ('data', '(2,3)f8')]))
        self.assertEqual(s.unrolled_flat_dtype, np.dtype([('f0', 'u4'), ('f1', 'f8'), ('f2', 'f8'), ('f3', 'f8'), ('f4', 'f8'), ('f5', 'f8'), ('f6', 'f8')]))

    def test_xpath(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        s = comma.csv.struct('id,event', 'u4', event_t)
        self.assertTupleEqual(s.fields, ('id', 'event/t', 'event/point/x', 'event/point/y', 'event/point/z'))
        self.assertTupleEqual(s.types, ('u4', 'datetime64[us]', 'f8', 'f8', 'f8'))
        self.assertEqual(s.format, 'u4,datetime64[us],f8,f8,f8')
        self.assertSetEqual(s.ambiguous_leaves, set())
        self.assertDictEqual(s.xpath_of_leaf, dict(id='id', t='event/t', x='event/point/x', y='event/point/y', z='event/point/z'))
        self.assertDictEqual(s.shorthand, { 'event': ('event/t', 'event/point/x', 'event/point/y', 'event/point/z'), 'event/point': ('event/point/x', 'event/point/y', 'event/point/z') })
        self.assertDictEqual(s.type_of_field, { 'id': 'u4', 'event/t': 'datetime64[us]', 'event/point/x': 'f8', 'event/point/y': 'f8', 'event/point/z': 'f8' })
        self.assertEqual(s.dtype, np.dtype([('id', 'u4'), ('event', [('t', 'datetime64[us]'), ('point', [('x', 'f8'), ('y', 'f8'), ('z', 'f8')])])]))
        self.assertEqual(s.flat_dtype, np.dtype([('id', 'u4'), ('event/t', 'datetime64[us]'), ('event/point/x', 'f8'), ('event/point/y', 'f8'), ('event/point/z', 'f8')]))
        self.assertEqual(s.unrolled_flat_dtype, np.dtype([('f0', 'u4'), ('f1', 'M8[us]'), ('f2', 'f8'), ('f3', 'f8'), ('f4', 'f8')]))

    def test_missing_fields(self):
        s = comma.csv.struct(',x,y', 'u4', 'f8', 'f4', 'S2')
        self.assertEqual(len(s.fields), 4)
        self.assertEqual(s.fields[1], 'x')
        self.assertEqual(s.fields[2], 'y')
        self.assertTupleEqual(s.types, ('u4', 'f8', 'f4', 'S2'))
        self.assertEqual(s.format, 'u4,f8,f4,S2')
        self.assertEqual(s.type_of_field['x'], 'f8')
        self.assertEqual(s.type_of_field['y'], 'f4')
        self.assertEqual(len(s.flat_dtype.descr), 4)
        self.assertEqual(s.dtype, s.flat_dtype)
        self.assertEqual(s.unrolled_flat_dtype, np.dtype([('f0', 'u4'), ('f1', 'f8'), ('f2', 'f4'), ('f3', 'S2')]))

    def test_ambiguous_leaves(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('id,point', 'datetime64[us]', point_t)
        s = comma.csv.struct('id,x,event', 'u4', 'S4', event_t)
        self.assertDictEqual(s.xpath_of_leaf, dict(y='event/point/y', z='event/point/z'))
        self.assertSetEqual(s.ambiguous_leaves, set(['id', 'x']))

    def test_too_many_fields(self):
        self.assertRaises(Exception, comma.csv.struct, 'x,y', 'u4')

    def test_invalid_field_names(self):
        self.assertRaises(Exception, comma.csv.struct, 'data/x', 'u4')

    def test_strings_vs_types(self):
        a = comma.csv.struct('id,x', 'u4', 'f8')
        b = comma.csv.struct('id,x', np.uint32, np.float64)
        self.assertEqual(a.fields, b.fields)
        self.assertEqual(a.fields, b.fields)
        self.assertTupleEqual(a.types, ('u4', 'f8'))
        self.assertTupleEqual(b.types, (np.uint32, np.float64))
        self.assertEqual(a.format, b.format)
        self.assertEqual(a.ambiguous_leaves, b.ambiguous_leaves)
        self.assertEqual(a.xpath_of_leaf, b.xpath_of_leaf)
        self.assertEqual(a.shorthand, b.shorthand)
        self.assertDictEqual(a.type_of_field, dict(id='u4', x='f8'))
        self.assertDictEqual(b.type_of_field, dict(id=np.uint32, x=np.float64))
        self.assertEqual(a.flat_dtype, b.flat_dtype)
        self.assertEqual(a.unrolled_flat_dtype, b.unrolled_flat_dtype)

    def test_call_default(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('id,point', 'datetime64[us]', point_t)
        event = event_t()
        self.assertEqual(event.dtype, event_t.dtype)
        self.assertEqual(event.size, 1)

    def test_call_size(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('id,point', 'datetime64[us]', point_t)
        event = event_t(12)
        self.assertEqual(event.dtype, event_t.dtype)
        self.assertEqual(event.size, 12)

    def test_to_tuple(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('id,point', 'u4', point_t)
        event = event_t()
        event['id'] = 123
        event['point']['x'] = 1.1
        event['point']['y'] = 1.2
        event['point']['z'] = 1.3
        self.assertTupleEqual(event_t.to_tuple(event), (123, 1.1, 1.2, 1.3))

    def test_to_tuple_array(self):
        event_t = comma.csv.struct('id,point', 'u4', '3f8')
        event = event_t()
        event['id'] = 123
        event['point'] = (1.1, 1.2, 1.3)
        self.assertTupleEqual(event_t.to_tuple(event), (123, 1.1, 1.2, 1.3))

    def test_to_tuple_time(self):
        comma.csv.time.zone('UTC')
        event_t = comma.csv.struct('t,x,y,z', 'datetime64[us]', 'f8', 'f8', 'f8')
        event = event_t()
        event['t'] = comma.csv.time.to_numpy('20101011T010203')
        event['x'] = 1.1
        event['y'] = 1.2
        event['z'] = 1.3
        from datetime import datetime
        self.assertTupleEqual(event_t.to_tuple(event), (datetime(2010, 10, 11, 1, 2, 3), 1.1, 1.2, 1.3))

    def test_to_tuple_throw_type(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        data_t = comma.csv.struct('id', 'u4')
        self.assertRaises(comma.csv.struct_error, point_t.to_tuple, data_t())

    def test_to_tuple_throw_size(self):
        event_t = comma.csv.struct('t,x,y,z', 'datetime64[us]', 'f8', 'f8', 'f8')
        event = event_t(2)
        self.assertRaises(comma.csv.struct_error, event_t.to_tuple, event)


class test_stream(unittest.TestCase):
    def test_use_defaults(self):
        s = comma.csv.stream(comma.csv.struct('x', 'f8'))
        self.assertEqual(s.delimiter, ',')
        self.assertFalse(s.flush)
        self.assertEqual(s.precision, 12)
        self.assertEqual(s.source, sys.stdin)
        self.assertEqual(s.target, sys.stdout)
        self.assertIsNone(s.tied)
        self.assertTrue(s.full_xpath)

    def test_override_defaults(self):
        source = open(data_in, 'r')
        target = open(data_out, 'w')
        t = comma.csv.stream(comma.csv.struct('id', 'S4'), delimiter=';')
        s = comma.csv.stream(comma.csv.struct('x', 'f8'), delimiter=';', flush=True, precision=4, source=source, target=target, tied=t, full_xpath=False)
        self.assertEqual(s.delimiter, ';')
        self.assertTrue(s.flush)
        self.assertEqual(s.precision, 4)
        self.assertEqual(s.source, source)
        self.assertEqual(s.target, target)
        self.assertEqual(s.tied, t)
        self.assertFalse(s.full_xpath)

    def test_fields_from_struct(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t)
        self.assertTupleEqual(s.fields, ('id', 'event/t', 'event/point/x', 'event/point/y', 'event/point/z'))

    def test_fields_simple(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t, fields='id,event/t,event/point/x,event/point/y,event/point/z')
        self.assertTupleEqual(s.fields, ('id', 'event/t', 'event/point/x', 'event/point/y', 'event/point/z'))

    def test_fields_mixedup(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t, fields='event/point/z,id,event/point/x,event/t,event/point/y')
        self.assertTupleEqual(s.fields, ('event/point/z', 'id', 'event/point/x', 'event/t', 'event/point/y'))

    def test_fields_missing(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t, fields='event/point/z,event/point/y')
        self.assertTupleEqual(s.fields, ('event/point/z', 'event/point/y'))

    def test_fields_blanks(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t, fields=',,event/point/z,,event/point/y,')
        self.assertTupleEqual(s.fields, ('', '', 'event/point/z', '', 'event/point/y', ''))

    def test_fields_unknown_fields(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t, fields='u1,u2,event/point/z,,event/point/y,u3')
        self.assertTupleEqual(s.fields, ('u1', 'u2', 'event/point/z', '', 'event/point/y', 'u3'))

    def test_fields_leaves(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t, fields=',z,id,,t,y', full_xpath=False)
        self.assertTupleEqual(s.fields, ('', 'event/point/z', 'id', '', 'event/t', 'event/point/y'))

    def test_fields_shorthand(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        s = comma.csv.stream(record_t, fields=',event/point,,,id')
        self.assertTupleEqual(s.fields, ('', 'event/point/x', 'event/point/y', 'event/point/z', '', '', 'id'))

    def test_fields_invalid_fields(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('t,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,event', 'u4', event_t)
        self.assertRaises(Exception, comma.csv.stream, record_t, fields='event/point', full_xpath=False)

    def test_fields_ambiguous_leaves(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('id,point', 'datetime64[us]', point_t)
        record_t = comma.csv.struct('id,x,event', 'u4', 'S4', event_t)
        self.assertRaises(Exception, comma.csv.stream, record_t, fields='x', full_xpath=False)

    def test_fields_duplicates(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        self.assertRaises(Exception, comma.csv.stream, point_t, fields='x,y,z,x')

    def test_tied_is_not_stream(self):
        t = comma.csv.struct('x', 'f8')
        self.assertRaises(Exception, comma.csv.stream, t, tied='not_a_stream')

    def test_mismatched_tied(self):
        t_binary = comma.csv.stream(comma.csv.struct('i', 'u1'), binary=True)
        t_ascii = comma.csv.stream(comma.csv.struct('i', 'u1'))
        s = comma.csv.struct('x', 'f8')
        self.assertRaises(Exception, comma.csv.stream, s, tied=t_binary)
        self.assertRaises(Exception, comma.csv.stream, s, binary=True, tied=t_ascii)

    def test_binary(self):
        point_t = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        t1 = comma.csv.stream(point_t, fields=',x')
        self.assertFalse(t1.binary)
        t2 = comma.csv.stream(point_t, fields=',x', format='u1,f8', binary=False)
        self.assertFalse(t2.binary)
        t3 = comma.csv.stream(point_t, fields='x', binary=False)
        self.assertFalse(t3.binary)

        t4 = comma.csv.stream(point_t, fields=',x', format='u1,f8')
        self.assertTrue(t4.binary)
        t5 = comma.csv.stream(point_t, fields='x', binary=True)
        self.assertTrue(t5.binary)
        t6 = comma.csv.stream(point_t, fields=',x', format='u1,f8', binary=True)
        self.assertTrue(t6.binary)

    def test_size(self):
        self.assertEqual(comma.csv.stream.buffer_size_in_bytes, 65536)
        s = comma.csv.struct('x,id', 'f8', 'u4')
        tied = comma.csv.stream(comma.csv.struct('i', 'u2'))
        t1 = comma.csv.stream(s)
        self.assertEqual(t1.size, comma.csv.stream.buffer_size_in_bytes / 12)
        t2 = comma.csv.stream(s, flush=True)
        self.assertEqual(t2.size, 1)
        t3 = comma.csv.stream(s, tied=tied)
        self.assertEqual(t3.size, comma.csv.stream.buffer_size_in_bytes / 2)

    def test_ascii_simple_single_field(self):
        s = comma.csv.struct('x', 'f8')
        t = comma.csv.stream(s)
        self.assertEqual(t.input_dtype, np.dtype([('x', 'f8')]))
        self.assertDictEqual(t.ascii_converters, {})
        self.assertTupleEqual(t.usecols, (0,))
        self.assertEqual(t.filling_values, '')
        self.assertTupleEqual(t.missing_fields, ())
        self.assertEqual(t.complete_fields, t.fields)
        self.assertEqual(t.complete_dtype, t.input_dtype)
        self.assertIsNone(t.missing_dtype)
        self.assertIsNone(t.data_extraction_dtype)

    def test_ascii_simple_multiple_fields(self):
        s = comma.csv.struct('x,y,z', 'f8', 'f8', 'f8')
        t = comma.csv.stream(s)
        self.assertEqual(t.input_dtype, np.dtype([('x', 'f8'), ('y', 'f8'), ('z', 'f8')]))
        self.assertDictEqual(t.ascii_converters, {})
        self.assertTupleEqual(t.usecols, (0, 1, 2))
        self.assertTupleEqual(t.filling_values, ('', '', ''))
        self.assertTupleEqual(t.missing_fields, ())
        self.assertEqual(t.complete_fields, t.fields)
        self.assertEqual(t.complete_dtype, t.input_dtype)
        self.assertIsNone(t.missing_dtype)
        self.assertIsNone(t.data_extraction_dtype)

    def test_ascii_general_fields(self):
        s = comma.csv.struct('t,x,y,z', 'datetime64[us]', 'f8', 'f8', 'f8')
        t = comma.csv.stream(s, fields='n1,n2,t,,y,')
        # f0,f1,... are standard numpy field names, which are used when field names are not specified
        self.assertEqual(t.input_dtype, np.dtype([('f0', 'S'), ('f1', 'S'), ('f2', 'M8[us]'), ('f3', 'S'), ('f4', 'f8'), ('f5', 'S')]))
        self.assertDictEqual(t.ascii_converters, {2: comma.csv.time.to_numpy })
        self.assertTupleEqual(t.usecols, (0, 1, 2, 3, 4, 5))
        self.assertTupleEqual(t.filling_values, ('', '', '', '', '', ''))
        self.assertTupleEqual(t.missing_fields, ('x', 'z'))
        self.assertTupleEqual(t.complete_fields, ('n1', 'n2', 't', '', 'y', '', 'x', 'z'))
        self.assertEqual(t.missing_dtype, np.dtype([('f6', 'f8'), ('f7', 'f8')]))
        self.assertEqual(t.complete_dtype, np.dtype([('f0', 'S'), ('f1', 'S'), ('f2', 'M8[us]'), ('f3', 'S'), ('f4', 'f8'), ('f5', 'S'), ('f6', 'f8'), ('f7', 'f8')]))
        self.assertEqual(t.data_extraction_dtype, np.dtype({ 'names': ['f2', 'f6', 'f4', 'f7'], 'formats': ['M8[us]', 'f8', 'f8', 'f8'], 'offsets': [0, 16, 8, 24] }))

    def test_ascii_structured(self):
        point_t = comma.csv.struct('t,x,y,z', 'datetime64[us]', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('dt,point', 'timedelta64[us]', point_t)
        record_t = comma.csv.struct('id,name,event', 'u4', 'S36', event_t)
        t = comma.csv.stream(record_t, fields=',name,,,id,event/point/x,event/point/t,')
        self.assertEqual(t.input_dtype, np.dtype([('f0', 'S'), ('f1', 'S36'), ('f2', 'S'), ('f3', 'S'), ('f4', 'u4'), ('f5', 'f8'), ('f6', 'M8[us]'), ('f7', 'S')]))
        self.assertDictEqual(t.ascii_converters, {6: comma.csv.time.to_numpy })
        self.assertTupleEqual(t.usecols, (0, 1, 2, 3, 4, 5, 6, 7))
        self.assertTupleEqual(t.filling_values, ('', '', '', '', '', '', '', ''))
        self.assertTupleEqual(t.missing_fields, ('event/dt', 'event/point/y', 'event/point/z'))
        self.assertTupleEqual(t.complete_fields, ('', 'name', '', '', 'id', 'event/point/x', 'event/point/t', '', 'event/dt', 'event/point/y', 'event/point/z'))
        self.assertEqual(t.missing_dtype, np.dtype([('f8', 'm8[us]'), ('f9', 'f8'), ('f10', 'f8')]))
        self.assertEqual(t.complete_dtype, np.dtype([('f0', 'S'), ('f1', 'S36'), ('f2', 'S'), ('f3', 'S'), ('f4', 'u4'), ('f5', 'f8'), ('f6', 'M8[us]'), ('f7', 'S'), ('f8', 'm8[us]'), ('f9', 'f8'), ('f10', 'f8')]))
        self.assertEqual(t.data_extraction_dtype, np.dtype({'names': ['f4', 'f1', 'f8', 'f6', 'f5', 'f9', 'f10'], 'formats': ['u4', 'S36', 'm8[us]', 'M8[us]', 'f8', 'f8', 'f8'], 'offsets': [36, 0, 56, 48, 40, 64, 72] }))

    def test_binary_structured(self):
        point_t = comma.csv.struct('t,x,y,z', 'datetime64[us]', 'f8', 'f8', 'f8')
        event_t = comma.csv.struct('dt,point', 'timedelta64[us]', point_t)
        record_t = comma.csv.struct('id,name,event', 'u4', 'S36', event_t)
        t = comma.csv.stream(record_t, fields=',name,,,id,event/point/x,event/point/t,', format='i1,S36,i2,i2,u4,f8,datetime64[us],S10')
        self.assertEqual(t.input_dtype, np.dtype([('f0', 'i1'), ('f1', 'S36'), ('f2', 'i2'), ('f3', 'i2'), ('f4', 'u4'), ('f5', 'f8'), ('f6', 'M8[us]'), ('f7', 'S10')]))
        self.assertRaises(AttributeError, getattr, t, 'ascii_converters')
        self.assertRaises(AttributeError, getattr, t, 'usecols')
        self.assertRaises(AttributeError, getattr, t, 'filling_values')
        self.assertTupleEqual(t.missing_fields, ('event/dt', 'event/point/y', 'event/point/z'))
        self.assertTupleEqual(t.complete_fields, ('', 'name', '', '', 'id', 'event/point/x', 'event/point/t', '', 'event/dt', 'event/point/y', 'event/point/z'))
        self.assertEqual(t.missing_dtype, np.dtype([('f8', 'm8[us]'), ('f9', 'f8'), ('f10', 'f8')]))
        self.assertEqual(t.complete_dtype, np.dtype([('f0', 'i1'), ('f1', 'S36'), ('f2', 'i2'), ('f3', 'i2'), ('f4', 'u4'), ('f5', 'f8'), ('f6', 'M8[us]'), ('f7', 'S10'), ('f8', 'm8[us]'), ('f9', 'f8'), ('f10', 'f8')]))
        self.assertEqual(t.data_extraction_dtype, np.dtype({'names': ['f4', 'f1', 'f8', 'f6', 'f5', 'f9', 'f10'], 'formats': ['u4', 'S36', 'm8[us]', 'M8[us]', 'f8', 'f8', 'f8'], 'offsets': [41, 1, 71, 53, 45, 79, 87] }))

    def test_numpy_scalar_to_string(self):
        stream = comma.csv.stream(comma.csv.struct('i', 'S1'))
        stream_with_precision = comma.csv.stream(comma.csv.struct('i', 'S1'), precision=3)
        def f(value): return stream.numpy_scalar_to_string(np.float64(value))
        self.assertEqual(f(1.6789), '1.6789')
        self.assertEqual(f(0.12345678901234567890), '0.123456789012')
        self.assertEqual(f(-1.2345), '-1.2345')
        self.assertEqual(f(0.0), '0')
        self.assertEqual(f(10), '10')
        self.assertEqual(f(-12345678), '-12345678')
        def p(value): return stream_with_precision.numpy_scalar_to_string(np.float64(value))
        self.assertEqual(p(1.6789), '1.68')
        self.assertEqual(p(-1.2345), '-1.23')
        self.assertEqual(p(0.0), '0')
        self.assertEqual(p(10), '10')
        self.assertEqual(p(-1000), '-1e+03')
        def i(value): return stream.numpy_scalar_to_string(np.int64(value))
        self.assertEqual(i(0), '0')
        self.assertEqual(i(-123456789), '-123456789')
        def s(value): return stream.numpy_scalar_to_string(np.str_(value))
        self.assertEqual(s('test'), 'test')
        def t(value): return stream.numpy_scalar_to_string(np.datetime64(value, 'us'))
        self.assertEqual(t('2015-01-02 01:02:03.123456'), '20150102T010203.123456')
        self.assertEqual(t('2015-01-02 01:02:03'), '20150102T010203')
        def c(value): return stream.numpy_scalar_to_string(np.complex128(value))
        self.assertRaises(Exception, c, 1 + 2j)


if __name__ == '__main__':
    unittest.main()
