import unittest
import comma.cpp_bindings

class test(unittest.TestCase):
    def test_size(self):
        self.assertEqual(comma.cpp_bindings.csv.format('d,2ub,s[5]').size(), 15)


if __name__ == '__main__':
    unittest.main()

