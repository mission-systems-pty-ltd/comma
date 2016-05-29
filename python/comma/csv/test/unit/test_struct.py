import unittest
import numpy as np
import comma


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
        self.assertTupleEqual(s.types, ('u4', 'M8[us]', 'f8', 'f8', 'f8'))
        self.assertEqual(s.format, 'u4,M8[us],f8,f8,f8')
        self.assertSetEqual(s.ambiguous_leaves, set())
        self.assertDictEqual(s.xpath_of_leaf, dict(id='id', t='event/t', x='event/point/x', y='event/point/y', z='event/point/z'))
        self.assertDictEqual(s.shorthand, { 'event': ('event/t', 'event/point/x', 'event/point/y', 'event/point/z'), 'event/point': ('event/point/x', 'event/point/y', 'event/point/z') })
        self.assertDictEqual(s.type_of_field, { 'id': 'u4', 'event/t': 'M8[us]', 'event/point/x': 'f8', 'event/point/y': 'f8', 'event/point/z': 'f8' })
        self.assertEqual(s.dtype, np.dtype([('id', 'u4'), ('event', [('t', 'M8[us]'), ('point', [('x', 'f8'), ('y', 'f8'), ('z', 'f8')])])]))
        self.assertEqual(s.flat_dtype, np.dtype([('id', 'u4'), ('event/t', 'M8[us]'), ('event/point/x', 'f8'), ('event/point/y', 'f8'), ('event/point/z', 'f8')]))
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
        self.assertRaises(ValueError, comma.csv.struct, 'x,y', 'u4')

    def test_invalid_field_names(self):
        self.assertRaises(ValueError, comma.csv.struct, 'data/x', 'u4')

    def test_strings_vs_types(self):
        a = comma.csv.struct('id,x', 'u4', 'f8')
        b = comma.csv.struct('id,x', np.uint32, np.float64)
        self.assertEqual(a.fields, b.fields)
        self.assertEqual(a.fields, b.fields)
        self.assertTupleEqual(a.types, ('u4', 'f8'))
        self.assertTupleEqual(b.types, ('u4', 'f8'))
        self.assertEqual(a.format, b.format)
        self.assertEqual(a.ambiguous_leaves, b.ambiguous_leaves)
        self.assertEqual(a.xpath_of_leaf, b.xpath_of_leaf)
        self.assertEqual(a.shorthand, b.shorthand)
        self.assertDictEqual(a.type_of_field, dict(id='u4', x='f8'))
        self.assertDictEqual(b.type_of_field, dict(id='u4', x='f8'))
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
        self.assertRaises(TypeError, point_t.to_tuple, data_t())

    def test_to_tuple_throw_size(self):
        event_t = comma.csv.struct('t,x,y,z', 'datetime64[us]', 'f8', 'f8', 'f8')
        event = event_t(2)
        self.assertRaises(ValueError, event_t.to_tuple, event)

if __name__ == '__main__':
    unittest.main()
