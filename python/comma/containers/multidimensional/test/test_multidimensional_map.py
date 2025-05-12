import numpy, pytest, sys
from comma.containers import multidimensional

def test_multidimensional_map_size():
    assert multidimensional.Map( resolution=numpy.array([1, 1, 1]) ).size() == 0
    assert multidimensional.Map( resolution=numpy.array([1, 1, 1]) ).count() == 0
    assert multidimensional.Map( resolution=numpy.array([1, 1, 1]), values=[[0.5, 0.5, 0.5], [0.5, 0.5, 0.5], [0.5, 0.5, 0.5]] ).size() == 1
    assert multidimensional.Map( resolution=numpy.array([1, 1, 1]), values=[[0.5, 0.5, 0.5], [0.5, 0.5, 0.5], [0.5, 0.5, 0.5]] ).count() == 3
    assert multidimensional.Map( resolution=numpy.array([10, 10, 10], dtype=numpy.float32), values=[[1, 2, 3], [4, 5, 6], [7, 8, 9]] ).size() == 1
    assert multidimensional.Map( resolution=numpy.array([10, 10, 10], dtype=numpy.float32), values=[[1, 2, 3], [4, 5, 6], [7, 8, 9]] ).count() == 3

def test_multidimensional_map_at():
    assert multidimensional.Map( resolution=numpy.array([1, 1, 1]) ).at( [0, 0, 0] ) is None
    m = multidimensional.Map( resolution=numpy.array([2, 2, 2]), values=[[0.5, 0.5, 0.5], [1, 1, 1], [3, 3, 3]] )
    assert m.size() == 2
    assert m.count() == 3
    # assert m.at( [-1, -1, -1] ) is None
    # assert m.at( [0, 0, 0] ).all( [0, 1] )
    # assert m.at( [1, 1, 1] ).all( [0, 1] )
    # assert m.at( [2, 2, 2] ) == [2]
    # assert m.at( [5, 5, 5] ) is None