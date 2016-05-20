import unittest
from comma.csv.time import to_numpy, from_numpy, is_undefined, undefined_time, zone, ascii_converters
import numpy as np


class test_time(unittest.TestCase):
    def test_to_numpy(self):
        f = to_numpy
        np.testing.assert_equal(f('20150102T122345.012345'), np.datetime64('2015-01-02 12:23:45.012345', 'us'))
        np.testing.assert_equal(f('19690102T122345.012345'), np.datetime64('1969-01-02 12:23:45.012345', 'us'))
        np.testing.assert_equal(f('20150102T122345'), np.datetime64('2015-01-02 12:23:45.000000', 'us'))
        self.assertTrue(is_undefined(f('')))
        self.assertTrue(is_undefined(f('not-a-date-time')))
        self.assertEqual(f('20150102T122345.012345').itemsize, 8)
        invalid = '20150102'
        self.assertRaises(Exception, f, invalid)

    def test_from_numpy(self):
        f = from_numpy
        np.testing.assert_equal(f(np.datetime64('2015-01-02 12:23:45.012345', 'us')), '20150102T122345.012345')
        np.testing.assert_equal(f(np.datetime64('1969-01-02 12:23:45.012345', 'us')), '19690102T122345.012345')
        np.testing.assert_equal(f(np.datetime64('2015-01-02 12:23:45.000000', 'us')), '20150102T122345')
        self.assertEqual(f(undefined_time()), 'not-a-date-time')
        self.assertRaises(Exception, f, 'invalid')

    def test_ascii_converters(self):
        f = ascii_converters
        self.assertDictEqual(f(('u1', 'u1', 'datetime64[us]', 'u1', 'M8[us]', 'u1')), { 2: to_numpy, 4: to_numpy })

    def test_zone(self):
        f = to_numpy
        # note that the time zones used below do not have daylight saving time
        zone('Asia/Jakarta')
        self.assertEqual(f('20150101T000000'), np.datetime64('2015-01-01T00:00:00'))
        zone('America/Caracas')
        self.assertEqual(f('20150101T000000'), np.datetime64('2015-01-01T00:00:00'))
        zone('UTC')
        self.assertEqual(f('20150101T000000'), np.datetime64('2015-01-01T00:00:00'))


if __name__ == '__main__':
    unittest.main()
