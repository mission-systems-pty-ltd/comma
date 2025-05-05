import numpy, pytest
from comma.containers import multidimensional

def test_multidimensional_map_size():
    assert multidimensional.Map( numpy.float32, 3, resolution=[1, 1, 1] ).size() == 0
    assert multidimensional.Map( numpy.float32, 3, resolution=[1, 1, 1] ).count() == 0
    # todo: debug: assert multidimensional.Map( numpy.float32, 3, resolution=[0.5, 0.5, 0.5], values=[[0, 0, 0], [0, 0, 0], [1, 1, 1]] ).size() == 2
    assert multidimensional.Map( numpy.float32, 3, resolution=[0.5, 0.5, 0.5], values=[[0, 0, 0], [0, 0, 0], [1, 1, 1]] ).count() == 3
