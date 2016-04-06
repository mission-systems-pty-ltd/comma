import unittest
import comma

class test( unittest.TestCase ):
  def test_expand( self ):
    f = comma.csv.format.expand
    self.assertEqual( f( '' ), '' )
    self.assertEqual( f( 'ub' ), 'ub' )
    self.assertEqual( f( 'ub,uw,ui,b,w,i,f,d,s[1],s[10],t' ), 'ub,uw,ui,b,w,i,f,d,s[1],s[10],t' )
    self.assertEqual( f( '2ub,3s[1],4t' ), 'ub,ub,s[1],s[1],s[1],t,t,t,t' )
    self.assertEqual( f( '2d,3d' ), 'd,d,d,d,d' )
    self.assertEqual( f( '11d' ), 'd,d,d,d,d,d,d,d,d,d,d' )
    self.assertEqual( f( 'invalid,2invalid' ), 'invalid,2invalid' )

  def test_time_format( self ):
    self.assertEqual( comma.csv.format.dictionary()['t'], 'datetime64[us]' )

  def test_to_numpy( self ):
    f = comma.csv.format.to_numpy
    self.assertRaises( Exception, f, '' )
    self.assertTupleEqual( f( 'ub' ), ('u1',) )
    self.assertTupleEqual( f( 'ub,uw,ui,b,w,i,f,d,s[1],s[10],t' ), ( 'u1', 'u2', 'u4', 'i1', 'i2', 'i4', 'f4', 'f8', 'S1', 'S10', 'datetime64[us]' ) )
    self.assertTupleEqual( f( '2ub,3s[1],4t' ), ( 'u1', 'u1', 'S1', 'S1', 'S1', 'datetime64[us]', 'datetime64[us]', 'datetime64[us]', 'datetime64[us]' ) )
    self.assertTupleEqual( f( '2d,3d' ), ( 'f8', 'f8', 'f8', 'f8', 'f8' ) )
    self.assertTupleEqual( f( '11d' ), ( 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8', 'f8' ) )
    self.assertRaises( Exception, f, 'invalid,2invalid' )

  def test_bool( self ):
    self.assertEqual( comma.csv.format.to_numpy( 'bool' ), ('b1',) )
    self.assertEqual( comma.csv.format.to_numpy( 'd,bool' ), ( 'f8', 'b1' ) )

if __name__ == '__main__':
    unittest.main()
