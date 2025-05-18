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
    assert multidimensional.Map( resolution=numpy.array([1, 1, 1]) ).at([0, 0, 0]) is None
    m = multidimensional.Map( resolution=numpy.array([2, 2, 2], dtype=float), values=[[0, 0, 0], [0.5, 0.5, 0.5], [1, 1, 1], [2, 2, 2], [3, 3, 3]] )
    assert m.size() == 2
    assert m.count() == 5
    assert m.at( [-1, -1, -1] ) is None
    assert ( m.at( [0, 0, 0] ) == [0, 1, 2] ).all()
    assert ( m.at( [2, 2, 2] ) == [3, 4] ).all()
    assert m.at( [5, 5, 5] ) is None
    # todo
    # - single point method? or pass multiple points and decide on shape?
    # - return list of sizes and list of indices
    # ? profile performance 
    # print( m.at( [5, 5, 5], radius=3 ), file=sys.stderr )

def test_multidimensional_map_at():
    values=[[0, 0, 0], [0.5, 0.5, 0.5], [1, 1, 1], [2, 2, 2], [3, 3, 3]]
    m = multidimensional.Map( resolution=numpy.array([2, 2, 2], dtype=float), values=values )
    assert values[ m.nearest( [0, 0, 0] ) ] == [0, 0, 0]
    assert values[ m.nearest( [0.4, 0.4, 0.4] ) ] == [0.5, 0.5, 0.5]
    assert values[ m.nearest( [1.4, 1.4, 1.4] ) ] == [1, 1, 1]
    assert values[ m.nearest( [5, 5, 5] ) ] == [3, 3, 3]
    assert m.nearest( [7, 7, 7] ) is None
